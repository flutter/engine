import 'dart:convert';
import 'dart:io';
import 'dart:typed_data';

import 'package:shelf/shelf_io.dart' as io;
import 'package:shelf_web_socket/shelf_web_socket.dart';
import 'package:stream_channel/stream_channel.dart';
import 'package:webkit_inspection_protocol/webkit_inspection_protocol.dart';
import 'package:path/path.dart' as path;

String goldensDirectory;

Future<void> hybridMain(StreamChannel<dynamic> channel) async {
  int devtoolsPort;
  final HttpServer server =
      await io.serve(webSocketHandler((dynamic webSocket) {}), 'localhost', 0);
  final ChromeConnection chromeConnection =
      ChromeConnection('localhost', devtoolsPort);
  final ChromeTab chromeTab = await chromeConnection
      .getTab((ChromeTab chromeTab) => chromeTab.url.contains('localhost'));
  final WipConnection connection = await chromeTab.connect();

  server.listen((HttpRequest request) async {
    final String body = await request
      .transform(utf8.decoder)
      .join('');

    final Map data = json.decode(body);
    final String key = data['key'];
    final bool overwrite = data['overwrite'];
    final WipResponse response = await connection.sendCommand('Page.captureScreenshot');
    final Uint8List bytes = base64.decode(response.result['data']);
    final File file = File(path.join(goldensDirectory, key));
    if (overwrite) {
      file.writeAsBytesSync(bytes);
      await request.response.close();
    } else {
      final List<int> realBytes = file.readAsBytesSync();
      final int lengths = bytes.length;
      for (int i = 0; i < lengths; i++) {
        if (realBytes[i] != bytes[i]) {
          request.response.add(utf8.encode('FAIL'));
          await request.response.close();
          return;
        }
      }
      await request.response.close();
    }
  });

  // Send the port number of the WebSocket server to the browser test, so
  // it knows what to connect to.
  channel.sink.add(server.port);
}
