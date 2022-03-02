// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.embedding.engine.loader;

import static java.util.Arrays.asList;

import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.os.Build;
import androidx.annotation.NonNull;
import androidx.annotation.WorkerThread;
import io.flutter.BuildConfig;
import io.flutter.Log;
import java.io.*;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.FutureTask;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.RejectedExecutionHandler;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

/** A class to initialize the native code. */
class ResourceExtractor {
  private static final String TAG = "ResourceExtractor";
  private static final String TIMESTAMP_PREFIX = "res_timestamp-";
  private static final String[] SUPPORTED_ABIS = getSupportedAbis();

  @NonNull private final String mDataDirPath;
  @NonNull private final String mPackageName;
  @NonNull private final PackageManager mPackageManager;
  @NonNull private final AssetManager mAssetManager;
  @NonNull private final HashSet<String> mResources;
  private FutureTask mExtractTask;

  /**
   * Thread pool settings are the same as default values from the deprecated android.os.AsyncTask
   * API, which was previously used (see {@link android.os.AsyncTask}).
   */
  private static final int CORE_POOL_SIZE = 1;

  private static final int MAXIMUM_POOL_SIZE = 20;
  private static final int BACKUP_POOL_SIZE = 5;
  private static final int KEEP_ALIVE_SECONDS = 3;

  private static final ThreadFactory mThreadFactory =
      new ThreadFactory() {
        private final AtomicInteger mCount = new AtomicInteger(1);

        public Thread newThread(Runnable r) {
          return new Thread(r, TAG + " #" + mCount.getAndIncrement());
        }
      };

  // Used only for rejected executions.
  private static ThreadPoolExecutor mBackupExecutor;
  private static LinkedBlockingQueue<Runnable> mBackupExecutorQueue;

  private static final RejectedExecutionHandler mRunOnSerialPolicy =
      new RejectedExecutionHandler() {
        public void rejectedExecution(Runnable r, ThreadPoolExecutor e) {
          Log.w(TAG, "Exceeded ThreadPoolExecutor pool size");
          synchronized (this) {
            if (mBackupExecutor == null) {
              mBackupExecutorQueue = new LinkedBlockingQueue<Runnable>();
              mBackupExecutor =
                  new ThreadPoolExecutor(
                      BACKUP_POOL_SIZE,
                      BACKUP_POOL_SIZE,
                      KEEP_ALIVE_SECONDS,
                      TimeUnit.SECONDS,
                      mBackupExecutorQueue,
                      mThreadFactory);

              mBackupExecutor.allowCoreThreadTimeOut(true);
            }
          }
          mBackupExecutor.execute(r);
        }
      };

  ResourceExtractor(
      @NonNull String dataDirPath,
      @NonNull String packageName,
      @NonNull PackageManager packageManager,
      @NonNull AssetManager assetManager) {
    mDataDirPath = dataDirPath;
    mPackageName = packageName;
    mPackageManager = packageManager;
    mAssetManager = assetManager;
    mResources = new HashSet<>();
  }

  @SuppressWarnings("deprecation")
  static long getVersionCode(@NonNull PackageInfo packageInfo) {
    // Linter needs P (28) hardcoded or else it will fail these lines.
    if (Build.VERSION.SDK_INT >= 28) {
      return packageInfo.getLongVersionCode();
    } else {
      return packageInfo.versionCode;
    }
  }

  ResourceExtractor addResource(@NonNull String resource) {
    mResources.add(resource);
    return this;
  }

  ResourceExtractor addResources(@NonNull Collection<String> resources) {
    mResources.addAll(resources);
    return this;
  }

  ResourceExtractor start() {
    if (BuildConfig.DEBUG && mExtractTask != null) {
      Log.e(
          TAG, "Attempted to start resource extraction while another extraction was in progress.");
    }

    mExtractTask =
        new FutureTask(
            new Runnable() {
              @Override
              public void run() {
                final File dataDir = new File(mDataDirPath);

                final String timestamp = checkTimestamp(dataDir, mPackageManager, mPackageName);
                if (timestamp == null) {
                  return;
                }

                deleteFiles(mDataDirPath, mResources);

                if (!extractAPK(dataDir)) {
                  return;
                }

                if (timestamp != null) {
                  try {
                    new File(dataDir, timestamp).createNewFile();
                  } catch (IOException e) {
                    Log.w(TAG, "Failed to write resource timestamp");
                  }
                }

                return;
              }
            },
            null);

    ThreadPoolExecutor executor =
        new ThreadPoolExecutor(
            CORE_POOL_SIZE,
            MAXIMUM_POOL_SIZE,
            KEEP_ALIVE_SECONDS,
            TimeUnit.SECONDS,
            new SynchronousQueue<Runnable>(),
            mThreadFactory);

    executor.setRejectedExecutionHandler(mRunOnSerialPolicy);
    executor.execute(mExtractTask);

    return this;
  }

  void waitForCompletion() {
    if (mExtractTask == null) {
      return;
    }

    try {
      mExtractTask.get();
    } catch (CancellationException | ExecutionException | InterruptedException e) {
      deleteFiles(mDataDirPath, mResources);
    }
  }

  private static String[] getExistingTimestamps(File dataDir) {
    return dataDir.list(
        new FilenameFilter() {
          @Override
          public boolean accept(File dir, String name) {
            return name.startsWith(TIMESTAMP_PREFIX);
          }
        });
  }

  private static void deleteFiles(@NonNull String dataDirPath, @NonNull HashSet<String> resources) {
    final File dataDir = new File(dataDirPath);
    for (String resource : resources) {
      final File file = new File(dataDir, resource);
      if (file.exists()) {
        file.delete();
      }
    }
    final String[] existingTimestamps = getExistingTimestamps(dataDir);
    if (existingTimestamps == null) {
      return;
    }
    for (String timestamp : existingTimestamps) {
      new File(dataDir, timestamp).delete();
    }
  }

  // Returns true if successfully unpacked APK resources,
  // otherwise deletes all resources and returns false.
  @WorkerThread
  private boolean extractAPK(@NonNull File dataDir) {
    for (String asset : mResources) {
      try {
        final String resource = "assets/" + asset;
        final File output = new File(dataDir, asset);
        if (output.exists()) {
          continue;
        }
        if (output.getParentFile() != null) {
          output.getParentFile().mkdirs();
        }

        try (InputStream is = mAssetManager.open(asset);
            OutputStream os = new FileOutputStream(output)) {
          copy(is, os);
        }
        if (BuildConfig.DEBUG) {
          Log.i(TAG, "Extracted baseline resource " + resource);
        }
      } catch (FileNotFoundException fnfe) {
        continue;

      } catch (IOException ioe) {
        Log.w(TAG, "Exception unpacking resources: " + ioe.getMessage());
        deleteFiles(mDataDirPath, mResources);
        return false;
      }
    }

    return true;
  }

  // Returns null if extracted resources are found and match the current APK version
  // and update version if any, otherwise returns the current APK and update version.
  private static String checkTimestamp(
      @NonNull File dataDir, @NonNull PackageManager packageManager, @NonNull String packageName) {
    PackageInfo packageInfo = null;

    try {
      packageInfo = packageManager.getPackageInfo(packageName, 0);
    } catch (PackageManager.NameNotFoundException e) {
      return TIMESTAMP_PREFIX;
    }

    if (packageInfo == null) {
      return TIMESTAMP_PREFIX;
    }

    String expectedTimestamp =
        TIMESTAMP_PREFIX + getVersionCode(packageInfo) + "-" + packageInfo.lastUpdateTime;

    final String[] existingTimestamps = getExistingTimestamps(dataDir);

    if (existingTimestamps == null) {
      if (BuildConfig.DEBUG) {
        Log.i(TAG, "No extracted resources found");
      }
      return expectedTimestamp;
    }

    if (existingTimestamps.length == 1) {
      if (BuildConfig.DEBUG) {
        Log.i(TAG, "Found extracted resources " + existingTimestamps[0]);
      }
    }

    if (existingTimestamps.length != 1 || !expectedTimestamp.equals(existingTimestamps[0])) {
      if (BuildConfig.DEBUG) {
        Log.i(TAG, "Resource version mismatch " + expectedTimestamp);
      }
      return expectedTimestamp;
    }

    return null;
  }

  private static void copy(@NonNull InputStream in, @NonNull OutputStream out) throws IOException {
    byte[] buf = new byte[16 * 1024];
    for (int i; (i = in.read(buf)) >= 0; ) {
      out.write(buf, 0, i);
    }
  }

  @SuppressWarnings("deprecation")
  private static String[] getSupportedAbis() {
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
      return Build.SUPPORTED_ABIS;
    } else {
      ArrayList<String> cpuAbis = new ArrayList<String>(asList(Build.CPU_ABI, Build.CPU_ABI2));
      cpuAbis.removeAll(asList(null, ""));
      return cpuAbis.toArray(new String[0]);
    }
  }
}
