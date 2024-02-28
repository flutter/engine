package dev.flutter.scenarios.externaltextures;

import android.media.MediaCodec;
import android.media.MediaExtractor;
import android.media.MediaFormat;
import android.os.Build;
import android.util.Log;
import android.view.Surface;
import androidx.annotation.RequiresApi;
import androidx.core.util.Supplier;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Objects;
import java.util.concurrent.CountDownLatch;

/** Decodes a sample video into the attached Surface. */
@RequiresApi(Build.VERSION_CODES.LOLLIPOP)
public class MediaSurfaceRenderer implements SurfaceRenderer {
  private final Supplier<MediaExtractor> extractorSupplier;
  private final int rotation;
  private CountDownLatch onFirstFrame;

  private Surface surface;
  private MediaExtractor extractor;
  private MediaFormat format;
  private Thread decodeThread;

  public MediaSurfaceRenderer(Supplier<MediaExtractor> extractorSupplier, int rotation) {
    this.extractorSupplier = extractorSupplier;
    this.rotation = rotation;
  }

  @Override
  public void attach(Surface surface, CountDownLatch onFirstFrame) {
    this.surface = surface;
    this.onFirstFrame = onFirstFrame;

    extractor = extractorSupplier.get();
    format = extractor.getTrackFormat(0);

    // NOTE: MediaFormat.KEY_ROTATION was not available until 23+, but the key is still handled on
    // API 21+.
    format.setInteger("rotation-degrees", rotation);

    decodeThread = new Thread(this::decodeThreadMain);
    decodeThread.start();
  }

  private void decodeThreadMain() {
    try {
      MediaCodec codec =
          MediaCodec.createDecoderByType(
              Objects.requireNonNull(format.getString(MediaFormat.KEY_MIME)));
      codec.configure(format, surface, null, 0);
      codec.start();

      // Track 0 is always the video track, since the sample video doesn't contain audio.
      extractor.selectTrack(0);

      MediaCodec.BufferInfo bufferInfo = new MediaCodec.BufferInfo();
      boolean seenEOS = false;
      long startTimeNs = System.nanoTime();
      int frameCount = 0;

      while (true) {
        // Move samples (video frames) from the extractor into the decoder, as long as we haven't
        // consumed all samples.
        if (!seenEOS) {
          int inputBufferIndex = codec.dequeueInputBuffer(-1);
          ByteBuffer inputBuffer = codec.getInputBuffer(inputBufferIndex);
          assert inputBuffer != null;
          int sampleSize = extractor.readSampleData(inputBuffer, 0);
          if (sampleSize >= 0) {
            long presentationTimeUs = extractor.getSampleTime();
            codec.queueInputBuffer(inputBufferIndex, 0, sampleSize, presentationTimeUs, 0);
            extractor.advance();
          } else {
            codec.queueInputBuffer(inputBufferIndex, 0, 0, 0, MediaCodec.BUFFER_FLAG_END_OF_STREAM);
            seenEOS = true;
          }
        }

        // Then consume decoded video frames from the decoder. These frames are automatically
        // pushed to the attached Surface, so this schedules them for present.
        int outputBufferIndex = codec.dequeueOutputBuffer(bufferInfo, 10000);
        boolean lastBuffer = (bufferInfo.flags & MediaCodec.BUFFER_FLAG_END_OF_STREAM) != 0;
        if (outputBufferIndex >= 0) {
          if (bufferInfo.size > 0) {
            if (onFirstFrame != null) {
              onFirstFrame.countDown();
              onFirstFrame = null;
            }
            Log.v("Scenarios", "Presenting frame " + frameCount);
            frameCount++;

            codec.releaseOutputBuffer(
                outputBufferIndex, startTimeNs + (bufferInfo.presentationTimeUs * 1000));
          }
        }

        // Exit the loop if there are no more frames available.
        if (lastBuffer) {
          break;
        }
      }

      codec.stop();
      codec.release();
      extractor.release();
    } catch (IOException e) {
      e.printStackTrace();
      throw new RuntimeException(e);
    }
  }

  @Override
  public void repaint() {}

  @Override
  public void destroy() {
    try {
      decodeThread.join();
    } catch (InterruptedException e) {
      e.printStackTrace();
      throw new RuntimeException(e);
    }
  }
}
