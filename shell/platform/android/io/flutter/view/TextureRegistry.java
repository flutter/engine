// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package io.flutter.view;

import android.graphics.SurfaceTexture;
import android.opengl.EGLContext;

/**
 * Registry of backend textures used with a single {@link FlutterView} instance.
 * Entries may be embedded into the Flutter view using the
 * <a href="https://docs.flutter.io/flutter/widgets/Texture-class.html">Texture</a>
 * widget.
 */
public interface TextureRegistry {
   /**
    * Creates and registers a SurfaceTexture managed by the Flutter engine.
    *
    * @return A SurfaceTextureEntry.
    */
    SurfaceTextureEntry createSurfaceTexture();

    /**
     * A registry entry for a managed SurfaceTexture.
     */
    interface SurfaceTextureEntry {
        /**
         * @return The managed SurfaceTexture.
         */
        SurfaceTexture surfaceTexture();

        /**
         * @return The identity of this SurfaceTexture.
         */
        long id();

        /**
         * Deregisters and releases this SurfaceTexture.
         */
        void release();
    }
  
   /**
   * Informs the the Flutter Engine that the external texture has been updated, and to start a new rendering pipeline.
   */
    void onShareFrameAvaliable(int textureIndex);
  
   /**
   * The OpenGL context created in the Flutter Engine, which is safe to user for sharing
   * Param: Android SDK version code
   */
    EGLContext getShareContext(long sdkInt);
  
   /**
   * Create a ShareTextureEntry, and registers a External Texture in Flutter Engine
   * The paramater is a OpenGL Texture ID
   * Generally, this is used for users who process image frame with OpenGL(such as filter with GPUImage), and display the image frame using Flutter external texture
   * Unlike SurfaceTexture, this can directly send the processed textures from OpenGL to flutter rendering pipeline.
   * Avoid the performance consumption of data writing to SurfaceTexture
   * Requirement: The OpenGL Texture should created in the EGLContext which is created using getShareContext(sdkInt) as a ShareContext
   * @return A ShareTextureEntry.
   */
    ShareTextureEntry createShareTexture(long shareTextureID);
  
    /**
     * A registry entry for a managed ShareTexture.
     */
    interface ShareTextureEntry {
      
        /**
         * @return The identity of this ShareTexture.
         */
        long id();
      
        /**
         * Deregisters and releases this ShareTexture.
         */
        void release();
    }
}
