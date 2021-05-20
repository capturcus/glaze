import 'dart:convert';

import 'package:flutter/material.dart';
import 'package:flutter_treeview/flutter_treeview.dart';
import 'package:glaze_client/connect.dart';
import 'package:flutter/scheduler.dart';
import 'package:web_socket_channel/web_socket_channel.dart';

class GamePage extends StatefulWidget {
  GamePage(Key key) : super(key: key);

  @override
  _GamePageState createState() => _GamePageState();
}

class _GamePageState extends State<GamePage> {
  late TreeViewController _treeViewController;
  bool docsOpen = true;
  late WebSocketChannel webSocketChannel;

  _GamePageState() {
    _treeViewController = TreeViewController(children: []);
    SchedulerBinding.instance!.addPostFrameCallback((_) {
      try {
        _connectToServer();
      } catch (e) {
        debugPrint("connection error");
        _connectToServer();
      }
    });
  }

  _expandNode(String key, bool expanded) {
    String msg = '${expanded ? "Expanded" : "Collapsed"}: $key';
    debugPrint(msg);
    Node node = _treeViewController.getNode(key)!;
    List<Node> updated;
    updated =
        _treeViewController.updateNode(key, node.copyWith(expanded: expanded));
    setState(() {
      _treeViewController = _treeViewController.copyWith(children: updated);
    });
  }

  int _lastKey = 0;

  _newKey() {
    _lastKey++;
    return "key" + _lastKey.toString();
  }

  List<Node> _jsonToNodeList(j) {
    print("_jsonToNodeList on: " + j.toString());
    List<Node> ret = [];
    if (j is Map<String, dynamic>) {
      Map<String, dynamic> map = j;
      for (var key in map.keys) {
        var val = map[key];
        if (val is String || val is int) {
          ret.add(Node(key: _newKey(), label: key + ": " + val.toString()));
        }
        if (val is Map<String, dynamic>) {
          ret.add(
              Node(key: _newKey(), label: key, children: _jsonToNodeList(val)));
        }
      }
      return ret;
    }
    throw Exception("unsupported json structure");
  }

  _websocketListen(message) {
    debugPrint("ws message: " + message);
    final j = jsonDecode(message);
    List<Node> jsonNodes = _jsonToNodeList(j);
    print("_jsonToNodeList finished");
    setState(() {
      print(_treeViewController.toString());
      _treeViewController = _treeViewController.copyWith(children: jsonNodes);
      print(_treeViewController.toString());
    });
  }

  _connectToServer() async {
    final result = await Navigator.push(
        context,
        MaterialPageRoute(
          builder: (context) => ConnectPage(Key('connect_page')),
        ));
    debugPrint(result[0] + " " + result[1]);
    final hostAndPort = result[0].toString().split(":");
    final uri = Uri(
        scheme: 'ws',
        host: hostAndPort[0],
        port: (hostAndPort.length == 2) ? int.parse(hostAndPort[1]) : 80);
    webSocketChannel = WebSocketChannel.connect(uri);
    webSocketChannel.stream.listen(_websocketListen);
    webSocketChannel.sink.add('{"name":"' + result[1] + '"}');
  }

  @override
  Widget build(BuildContext context) {
    TreeViewTheme _treeViewTheme = TreeViewTheme(
      expanderTheme: ExpanderThemeData(
        type: ExpanderType.caret,
        modifier: ExpanderModifier.none,
        position: ExpanderPosition.start,
        color: Colors.red.shade800,
        size: 20,
      ),
      labelStyle: TextStyle(
        fontSize: 16,
        letterSpacing: 0.3,
      ),
      parentLabelStyle: TextStyle(
        fontSize: 16,
        letterSpacing: 0.1,
        fontWeight: FontWeight.w800,
        // color: Colors.red.shade600,
      ),
      iconTheme: IconThemeData(
        size: 18,
        color: Colors.grey.shade800,
      ),
      colorScheme: ColorScheme.light(),
    );
    return Scaffold(
      appBar: AppBar(
        title: Text("Glaze game"),
      ),
      body: TreeView(
          controller: _treeViewController,
          allowParentSelect: true,
          supportParentDoubleTap: true,
          onExpansionChanged: (key, expanded) {
            print(_treeViewController.toString());
            return _expandNode(key, expanded);
          },
          onNodeDoubleTap: (key) {
            setState(() {
              _treeViewController =
                  _treeViewController.copyWith(selectedKey: key);
            });
          },
          theme: _treeViewTheme),
    );
  }
}
