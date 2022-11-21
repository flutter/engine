// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/types.glsl>
#include <impeller/transform.glsl>

uniform VertInfo {
  mat4 mvp;
<<<<<<< HEAD
  f16vec4 color;
} vert_info;
=======
  vec4 color;
}
vert_info;
>>>>>>> ddf6a20b86578f147ee7da023f3f08ecb4256d07

in f16vec2 position;

out f16vec4 v_color;

void main() {
  gl_Position = vert_info.mvp * vec4(position, 0.0, 1.0);
  v_color = vert_info.color;
}
