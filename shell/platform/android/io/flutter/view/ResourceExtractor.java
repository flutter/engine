// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.view;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.os.AsyncTask;
import android.os.Build;
import android.util.Log;
import io.flutter.util.PathUtils;
import org.json.JSONObject;

import java.io.*;
import java.util.Collection;
import java.util.HashSet;
import java.util.concurrent.CancellationException;
import java.util.concurrent.ExecutionException;
import java.util.zip.GZIPInputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

/**
 * A class to initialize the native code.
 **/
class ResourceExtractor {
    private static final String TAG = "ResourceExtractor";
    private static final String TIMESTAMP_PREFIX = "res_timestamp-";

    @SuppressWarnings("deprecation")
    static long getVersionCode(PackageInfo packageInfo) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P) {
            return packageInfo.getLongVersionCode();
        } else {
            return packageInfo.versionCode;
        }
    }

    private class ExtractTask extends AsyncTask<Void, Void, Void> {
        ExtractTask() { }

        @Override
        protected Void doInBackground(Void... unused) {
            final File dataDir = new File(PathUtils.getDataDirectory(mContext));

            ResourceUpdater resourceUpdater = FlutterMain.getResourceUpdater();
            if (resourceUpdater != null) {
                // Protect patch file from being overwritten by downloader while
                // it's being extracted since downloading happens asynchronously.
                resourceUpdater.getInstallationLock().lock();
            }

            try {
                if (resourceUpdater != null) {
                    File updateFile = resourceUpdater.getDownloadedPatch();
                    File activeFile = resourceUpdater.getInstalledPatch();

                    if (updateFile.exists()) {
                        JSONObject manifest = resourceUpdater.readManifest(updateFile);
                        if (resourceUpdater.validateManifest(manifest)) {
                            // Graduate patch file as active for asset manager.
                            if (activeFile.exists() && !activeFile.delete()) {
                                Log.w(TAG, "Could not delete file " + activeFile);
                                return null;
                            }
                            if (!updateFile.renameTo(activeFile)) {
                                Log.w(TAG, "Could not create file " + activeFile);
                                return null;
                            }
                        }
                    }
                }

                final String timestamp = checkTimestamp(dataDir);
                if (timestamp == null) {
                    return null;
                }

                deleteFiles();

                if (!extractUpdate(dataDir)) {
                    return null;
                }

                if (!extractAPK(dataDir)) {
                    return null;
                }

                if (timestamp != null) {
                    try {
                        new File(dataDir, timestamp).createNewFile();
                    } catch (IOException e) {
                        Log.w(TAG, "Failed to write resource timestamp");
                    }
                }

                return null;

            } finally {
              if (resourceUpdater != null) {
                  resourceUpdater.getInstallationLock().unlock();
              }
          }
        }
    }

    private final Context mContext;
    private final HashSet<String> mResources;
    private ExtractTask mExtractTask;

    ResourceExtractor(Context context) {
        mContext = context;
        mResources = new HashSet<>();
    }

    ResourceExtractor addResource(String resource) {
        mResources.add(resource);
        return this;
    }

    ResourceExtractor addResources(Collection<String> resources) {
        mResources.addAll(resources);
        return this;
    }

    ResourceExtractor start() {
        assert mExtractTask == null;
        mExtractTask = new ExtractTask();
        mExtractTask.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        return this;
    }

    void waitForCompletion() {
        if (mExtractTask == null) {
            return;
        }

        try {
            mExtractTask.get();
        } catch (CancellationException | ExecutionException | InterruptedException e) {
            deleteFiles();
        }
    }

    private String[] getExistingTimestamps(File dataDir) {
        return dataDir.list(new FilenameFilter() {
            @Override
            public boolean accept(File dir, String name) {
                return name.startsWith(TIMESTAMP_PREFIX);
            }
        });
    }

    private void deleteFiles() {
        final File dataDir = new File(PathUtils.getDataDirectory(mContext));
        for (String resource : mResources) {
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

    /// Returns true if successfully unpacked APK resources,
    /// otherwise deletes all resources and returns false.
    private boolean extractAPK(File dataDir) {
        final AssetManager manager = mContext.getResources().getAssets();

        for (String asset : mResources) {
            try {
                final File output = new File(dataDir, asset);
                if (output.exists()) {
                    continue;
                }
                if (output.getParentFile() != null) {
                    output.getParentFile().mkdirs();
                }

                try (InputStream is = manager.open(asset);
                     OutputStream os = new FileOutputStream(output)) {
                    copy(is, os);
                }

                Log.i(TAG, "Extracted baseline resource " + asset);

            } catch (FileNotFoundException fnfe) {
                continue;

            } catch (IOException ioe) {
                Log.w(TAG, "Exception unpacking resources: " + ioe.getMessage());
                deleteFiles();
                return false;
            }
        }

        return true;
    }

    /// Returns true if successfully unpacked update resources or if there is no update,
    /// otherwise deletes all resources and returns false.
    private boolean extractUpdate(File dataDir) {
        final AssetManager manager = mContext.getResources().getAssets();

        ResourceUpdater resourceUpdater = FlutterMain.getResourceUpdater();
        if (resourceUpdater == null) {
            return true;
        }

        File updateFile = resourceUpdater.getInstalledPatch();
        if (!updateFile.exists()) {
            return true;
        }

        JSONObject manifest = resourceUpdater.readManifest(updateFile);
        if (!resourceUpdater.validateManifest(manifest)) {
            // Obsolete patch file, nothing to install.
            return true;
        }

        ZipFile zipFile;
        try {
            zipFile = new ZipFile(updateFile);

        } catch (IOException e) {
            Log.w(TAG, "Exception unpacking resources: " + e.getMessage());
            deleteFiles();
            return false;
        }

        for (String asset : mResources) {
            boolean useDiff = false;
            ZipEntry entry = zipFile.getEntry(asset);
            if (entry == null) {
                useDiff = true;
                entry = zipFile.getEntry(asset + ".bzdiff40");
                if (entry == null) {
                    continue;
                }
            }

            final File output = new File(dataDir, asset);
            if (output.exists()) {
                continue;
            }
            if (output.getParentFile() != null) {
                output.getParentFile().mkdirs();
            }

            try {
                if (useDiff) {
                    ByteArrayOutputStream diff = new ByteArrayOutputStream();
                    try (InputStream is = zipFile.getInputStream(entry)) {
                        copy(is, diff);
                    }

                    ByteArrayOutputStream orig = new ByteArrayOutputStream();
                    try (InputStream is = manager.open(asset)) {
                        copy(is, orig);
                    }

                    try (OutputStream os = new FileOutputStream(output)) {
                        os.write(bspatch(orig.toByteArray(), diff.toByteArray()));
                    }

                } else {
                    try (InputStream is = zipFile.getInputStream(entry);
                         OutputStream os = new FileOutputStream(output)) {
                        copy(is, os);
                    }
                }

                Log.i(TAG, "Extracted override resource " + asset);

            } catch (FileNotFoundException fnfe) {
                continue;

            } catch (IOException ioe) {
                Log.w(TAG, "Exception unpacking resources: " + ioe.getMessage());
                deleteFiles();
                return false;
            }
        }

        return true;
    }

    // Returns null if extracted resources are found and match the current APK version
    // and update version if any, otherwise returns the current APK and update version.
    private String checkTimestamp(File dataDir) {
        PackageManager packageManager = mContext.getPackageManager();
        PackageInfo packageInfo = null;

        try {
            packageInfo = packageManager.getPackageInfo(mContext.getPackageName(), 0);
        } catch (PackageManager.NameNotFoundException e) {
            return TIMESTAMP_PREFIX;
        }

        if (packageInfo == null) {
            return TIMESTAMP_PREFIX;
        }

        String expectedTimestamp =
                TIMESTAMP_PREFIX + getVersionCode(packageInfo) + "-" + packageInfo.lastUpdateTime;

        ResourceUpdater resourceUpdater = FlutterMain.getResourceUpdater();
        if (resourceUpdater != null) {
            File patchFile = resourceUpdater.getInstalledPatch();
            JSONObject manifest = resourceUpdater.readManifest(patchFile);
            if (resourceUpdater.validateManifest(manifest)) {
                String patchNumber = manifest.optString("patchNumber", null);
                if (patchNumber != null) {
                    expectedTimestamp += "-" + patchNumber + "-" + patchFile.lastModified();
                } else {
                    expectedTimestamp += "-" + patchFile.lastModified();
                }
            }
        }

        final String[] existingTimestamps = getExistingTimestamps(dataDir);

        if (existingTimestamps == null) {
            Log.i(TAG, "No extracted resources found");
            return expectedTimestamp;
        }

        if (existingTimestamps.length == 1) {
            Log.i(TAG, "Found extracted resources " + existingTimestamps[0]);
        }

        if (existingTimestamps.length != 1
                || !expectedTimestamp.equals(existingTimestamps[0])) {
            Log.i(TAG, "Resource version mismatch " + expectedTimestamp);
            return expectedTimestamp;
        }

        return null;
    }

    private static void copy(InputStream in, OutputStream out) throws IOException {
        byte[] buf = new byte[16 * 1024];
        for (int i; (i = in.read(buf)) >= 0; ) {
            out.write(buf, 0, i);
        }
    }

    private static void read(InputStream in, byte[] buf, int off, int len) throws IOException {
        for (int i, n = 0; n < len; n += i) {
            if ((i = in.read(buf, off+n, len-n)) < 0) {
                throw new IOException("Unexpected EOF");
            }
        }
    }

    // This is a Java port of algorithm from Flutter framework bsdiff.dart.
    // Note that this port uses 32-bit ints, which limits data size to 2GB.
    //
    // Copyright 2003-2005 Colin Percival. All rights reserved.
    // Use of this source code is governed by a BSD-style license that can be
    // found in the LICENSE file.
    // Redistribution and use in source and binary forms, with or without
    // modification, are permitted providing that the following conditions 
    // are met:
    // 1. Redistributions of source code must retain the above copyright
    //    notice, this list of conditions and the following disclaimer.
    // 2. Redistributions in binary form must reproduce the above copyright
    //    notice, this list of conditions and the following disclaimer in the
    //    documentation and/or other materials provided with the distribution.
    //
    // THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
    // IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    // WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    // ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
    // DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    // DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    // OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    // HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
    // STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    // IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    // POSSIBILITY OF SUCH DAMAGE.
    private static byte[] bspatch(byte[] olddata, byte[] diffdata) throws IOException {
        InputStream in = new ByteArrayInputStream(diffdata, 0, diffdata.length);
        DataInputStream header = new DataInputStream(in);

        byte[] magic = new byte[8];
        header.read(magic);
        if (!new String(magic).equals("BZDIFF40")) {
            throw new IOException("Invalid magic");
        }

        int ctrllen = (int) header.readLong();
        int datalen = (int) header.readLong();
        int newsize = (int) header.readLong();
        header.close();

        in = new ByteArrayInputStream(diffdata, 0, diffdata.length);
        in.skip(32);
        DataInputStream cpf = new DataInputStream(new GZIPInputStream(in));

        in = new ByteArrayInputStream(diffdata, 0, diffdata.length);
        in.skip(32 + ctrllen);
        InputStream dpf = new GZIPInputStream(in);

        in = new ByteArrayInputStream(diffdata, 0, diffdata.length);
        in.skip(32 + ctrllen + datalen);
        InputStream epf = new GZIPInputStream(in);

        byte[] newdata = new byte[newsize];

	int oldpos = 0;
        int newpos = 0;

        while (newpos < newsize) {
            int[] ctrl = new int[3];
            for (int i = 0; i <= 2; i++) {
                ctrl[i] = (int) cpf.readLong();
            }
            if (newpos + ctrl[0] > newsize) {
                throw new IOException("Invalid ctrl[0]");
            }

            read(dpf, newdata, newpos, ctrl[0]);

            for (int i = 0; i < ctrl[0]; i++) {
                if ((oldpos + i >= 0) && (oldpos + i < olddata.length)) {
                    newdata[newpos + i] += olddata[oldpos + i];
                }
            }

            newpos += ctrl[0];
            oldpos += ctrl[0];

            if (newpos + ctrl[1] > newsize) {
                throw new IOException("Invalid ctrl[0]");
            }

            read(epf, newdata, newpos, ctrl[1]);

            newpos += ctrl[1];
            oldpos += ctrl[2];
        }

        cpf.close();
        dpf.close();
        epf.close();

        return newdata;
    }
}
