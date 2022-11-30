// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:meta/meta.dart';

import '../../dom.dart';

import '../hot_restart_cache_handler.dart';
import 'custom_element_embedding_strategy.dart';
import 'full_page_embedding_strategy.dart';

/// Provides the API that the FlutterViewEmbedder uses to interact with the DOM.
///
/// The base class handles "global" stuff that is shared across implementations,
/// like handling hot-restart cleanup.
///
/// This class is specialized to handle different types of DOM embeddings:
///
/// * [FullPageEmbeddingStrategy] - The default behavior, where flutter takes
///   control of the whole web page. This is how Flutter Web used to operate.
/// * [CustomElementEmbeddingStrategy] - Flutter is rendered inside a custom host
///   element, provided by the web app programmer through the engine
///   initialization.
abstract class EmbeddingStrategy {
  EmbeddingStrategy() {
    // Prepare some global stuff...
    assert(() {
      _hotRestartCache = HotRestartCacheHandler();
      return true;
    }());
  }

  factory EmbeddingStrategy.create({DomElement? hostElement}) {
    if (hostElement != null) {
      return CustomElementEmbeddingStrategy(hostElement);
    } else {
      return FullPageEmbeddingStrategy();
    }
  }

  /// Keeps a list of elements to be cleaned up at hot-restart.
  HotRestartCacheHandler? _hotRestartCache;

  void initialize({
    required String defaultFont,
    Map<String, String>? embedderMetadata,
  });

  /// Attaches the glassPane element into the hostElement.
  void attachGlassPane(DomElement glassPaneElement);

  /// Attaches the resourceHost element into the hostElement.
  void attachResourcesHost(DomElement resourceHost, {DomElement? nextTo});

  /// A callback that runs when hot restart is triggered.
  ///
  /// This should "clean" up anything handled by the [EmbeddingStrategy] instance.
  @mustCallSuper
  void onHotRestart() {
    // Elements on the [_hotRestartCache] are cleaned up *after* hot-restart.
  }

  /// Registers a [DomElement] to be cleaned up after hot restart.
  @mustCallSuper
  void registerElementForCleanup(DomElement element) {
    _hotRestartCache?.registerElement(element);
  }
}
