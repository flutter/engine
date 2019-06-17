import 'dart:html';

import 'package:stream_channel/stream_channel.dart';
import 'package:test/test.dart';

// ignore: implementation_imports
import 'package:test_api/src/frontend/async_matcher.dart' show AsyncMatcher;

/// Attempts to match the current browser state with the screenshot [filename].
Matcher matchesGoldenFile(String filename) {
  return _GoldenFileMatcher(filename);
}

class _GoldenFileMatcher extends AsyncMatcher {
  _GoldenFileMatcher(this.key);

  final String key;

  static WebSocket _socket;
  static Future<void> _setup() async {
    if (_socket != null) {
      return;
    }
    final StreamChannel<dynamic> channel = spawnHybridUri('web_socket_server.dart');
    final int port = await channel.stream.first;
    _socket = WebSocket('ws://localhost:$port');
  }

  @override
  Description describe(Description description) => description;

  @override
  Future<String> matchAsync(dynamic item) async {
    await _setup();
    _socket.send(<String, Object>{
      'name': key,
    });
    final MessageEvent pong = await _socket.onMessage.first;
    return pong.data;
  }
}
