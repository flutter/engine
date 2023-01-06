// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
part of dart.ui;

/// A composable [SceneNode].
class SceneNode extends NativeFieldWrapperClass1 {
  @pragma('vm:entry-point')
  SceneNode._create() {
    _constructor();
  }

  String? _debugName;

  void setTransform(Float64List matrix4) {
    _setTransform(matrix4);
  }

  void setAnimationState(String animationName, bool playing, double weight, double timeScale) {
    _setAnimationState(animationName, playing, weight, timeScale);
  }

  void seekAnimation(String animationName, double time) {
    _seekAnimation(animationName, time);
  }

  /// Creates a scene node from the asset with key [assetKey].
  ///
  /// The asset must be a file produced as the output of the `scenec` importer.
  /// The constructed object should then be reused via the [shader]
  /// method to create [Shader] objects that can be used by [Shader.paint].
  static Future<SceneNode> fromAsset(String assetKey) {
    // The flutter tool converts all asset keys with spaces into URI
    // encoded paths (replacing ' ' with '%20', for example). We perform
    // the same encoding here so that users can load assets with the same
    // key they have written in the pubspec.
      final String encodedKey = Uri(path: Uri.encodeFull(assetKey)).path;
    {
      final SceneNode? scene_node = _ipsceneRegistry[encodedKey]?.target;
      if (scene_node != null) {
        return Future<SceneNode>.value(scene_node);
      }
    }

    final SceneNode scene_node = SceneNode._create();
    return _futurize((_Callback<void> callback) {
      final String error = scene_node._initFromAsset(assetKey, callback);
      if (error.isNotEmpty) {
        return error;
      }
      assert(() {
        scene_node._debugName = assetKey;
        return true;
      }());

      _ipsceneRegistry[encodedKey] = WeakReference<SceneNode>(scene_node);
    }).then((_) => scene_node);
  }

  // This is a cache of scene nodes that have been loaded by
  // SceneNode.fromAsset. It holds weak references to the SceneNodes
  // so that the case where an in-use program is requested again can be fast,
  // but programs that are no longer referenced are not retained because of the
  // cache.
  static final Map<String, WeakReference<SceneNode>> _ipsceneRegistry =
      <String, WeakReference<SceneNode>>{};

  static void _reinitializeScene(String assetKey) {
    // If a shader for the asset isn't already registered, then there's no
    // need to reinitialize it. The new shader will be loaded and initialized
    // the next time the program accesses it.
    final WeakReference<SceneNode>? nodeRef = _ipsceneRegistry == null
      ? null
      : _ipsceneRegistry[assetKey];
    if (nodeRef == null) {
      return;
    }

    final SceneNode? program = nodeRef.target;
    if (program == null) {
      return;
    }

    final String result = program._initFromAsset(assetKey, (_) => {});
    if (result.isNotEmpty) {
      throw result; // ignore: only_throw_errors
    }
  }

  @FfiNative<Void Function(Handle)>('SceneNode::Create')
  external void _constructor();

  @FfiNative<Handle Function(Pointer<Void>, Handle, Handle)>('SceneNode::initFromAsset')
  external String _initFromAsset(String assetKey, _Callback<void> completion_callback);

  @FfiNative<Void Function(Pointer<Void>, Handle)>('SceneNode::initFromTransform')
  external void _initFromTransform(Float64List matrix4);

  @FfiNative<Void Function(Pointer<Void>, Handle)>('SceneNode::AddChild')
  external void _addChild(SceneNode sceneNode);

  @FfiNative<Void Function(Pointer<Void>, Handle)>('SceneNode::SetTransform')
  external void _setTransform(Float64List matrix4);

  @FfiNative<Void Function(Pointer<Void>, Handle, Handle, Handle, Handle)>('SceneScene::SetAnimationState')
  external void _setAnimationState(String animationName, bool playing, double weight, double timeScale);

  @FfiNative<Void Function(Pointer<Void>, Handle, Handle)>('SceneNode::SeekAnimation')
  external void _seekAnimation(String animationName, double time);

  /// Returns a fresh instance of [SceneShader].
  SceneShader sceneShader() => SceneShader._(this, debugName: _debugName);
}

/// A [Shader] generated from a [SceneNode].
///
/// Instances of this class can be obtained from the
/// [SceneNode.sceneShader] method.
class SceneShader extends Shader {
  SceneShader._(SceneNode node, { String? debugName }) : _debugName = debugName, super._() {
    _constructor(node);
  }

  final String? _debugName;

  /// Releases the native resources held by the [SceneShader].
  ///
  /// After this method is called, calling methods on the shader, or attaching
  /// it to a [Paint] object will fail with an exception. Calling [dispose]
  /// twice will also result in an exception being thrown.
  @override
  void dispose() {
    super.dispose();
    _dispose();
  }

  @FfiNative<Handle Function(Pointer<Void>, Handle)>('SceneShader::Create')
  external Float32List _constructor(SceneNode node);

  @FfiNative<Void Function(Pointer<Void>)>('SceneShader::Dispose')
  external void _dispose();
}
