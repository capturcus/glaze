import 'dart:async';
import 'dart:convert';

import 'package:flutter_treeview/flutter_treeview.dart';
import 'package:flutter/material.dart';
import 'package:flutter/scheduler.dart';
import 'package:glaze_client/action.dart';
import 'package:glaze_client/connect.dart';
import 'package:glaze_client/prompt.dart';
import 'package:web_socket_channel/web_socket_channel.dart';

class GamePage extends StatefulWidget {
  @override
  _GamePageState createState() => _GamePageState();
}

final TreeViewTheme treeViewTheme = TreeViewTheme(
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

class _GamePageState extends State<GamePage> {
  late TreeViewController _treeViewController;
  bool docsOpen = true;
  late WebSocketChannel webSocketChannel;
  List<String> logMessages = [];
  ScrollController _scrollController = new ScrollController();
  Map<String, Completer<dynamic>> _rpcCompleters = {};

  _GamePageState() {
    _treeViewController = TreeViewController(children: []);
    SchedulerBinding.instance!.addPostFrameCallback((_) {
      _connectToServer();
    });
  }

  _expandNode(String key, bool expanded) {
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

  Node? _getChildByLabel(List<Node> nodes, String label) {
    for (var child in nodes) {
      if (child.label == label) {
        return child;
      }
    }
    return null;
  }

  _getPreviousExpandedStatus(List<Node> nodes, String label) {
    Node? child = _getChildByLabel(nodes, label);
    if (child == null) return false;
    return child.expanded;
  }

  List<Node> _jsonToNodeList(j, List<Node>? previousNodes) {
    List<Node> ret = [];
    if (j is Map<String, dynamic>) {
      Map<String, dynamic> map = j;
      for (var key in map.keys) {
        var val = map[key];
        if (val is String || val is double) {
          ret.add(Node(key: _newKey(), label: key + ": " + val.toString()));
        }
        if (val is Map<String, dynamic>) {
          var previousExpandedStatus = (previousNodes == null)
              ? false
              : _getPreviousExpandedStatus(previousNodes, key);
          Node? node = (previousNodes == null)
              ? null
              : _getChildByLabel(previousNodes, key);
          ret.add(Node(
              key: _newKey(),
              label: key,
              expanded: previousExpandedStatus,
              children:
                  _jsonToNodeList(val, (node == null) ? null : node.children)));
        }
      }
      return ret;
    }
    throw Exception("unsupported json structure");
  }

  _pushLog(String log) {
    setState(() {
      logMessages.add(log);
    });
    SchedulerBinding.instance!.addPostFrameCallback((_) {
      _scrollController.animateTo(
        _scrollController.position.maxScrollExtent,
        duration: const Duration(milliseconds: 300),
        curve: Curves.easeOut,
      );
    });
  }

  _handlePrompt(j) async {
    PromptType pt = textToPromptType(j["prompt_type"]);
    final promptResult = await Navigator.push(
        context,
        MaterialPageRoute(
          builder: (context) =>
              PromptPage(j["prompt_text"], pt, j["prompt_data"]),
        ));
    print("prompt result: " + promptResult.toString());
    Map<String, dynamic> ret = {
      "type": "prompt_result",
      "prompt_key": j["prompt_key"],
      "prompt_result": promptResult,
    };
    webSocketChannel.sink.add(jsonEncode(ret));
  }

  _websocketListen(message) {
    debugPrint("websocket message: " + message);
    final j = jsonDecode(message);
    if (j["type"] == "world_update") {
      List<Node> jsonNodes =
          _jsonToNodeList(j["world"], _treeViewController.children);
      setState(() {
        _treeViewController = _treeViewController.copyWith(children: jsonNodes);
      });
    } else if (j["type"] == "log_message") {
      _pushLog(j["log"]);
    } else if (j["type"] == "rpc_result") {
      _rpcCompleters[j["rpc_key"]]!.complete(j["data"]);
      _rpcCompleters.remove(j["rpc_key"]);
    } else if (j["type"] == "prompt") {
      _handlePrompt(j);
    }
  }

  _connectToServer() async {
    final result = await Navigator.push(
        context,
        MaterialPageRoute(
          builder: (context) => ConnectPage(),
        ));
    final hostAndPort = result[0].toString().split(":");
    final uri = Uri(
        scheme: 'ws',
        host: hostAndPort[0],
        port: (hostAndPort.length == 2) ? int.parse(hostAndPort[1]) : 80);
    webSocketChannel = WebSocketChannel.connect(uri);
    webSocketChannel.stream.listen(_websocketListen);
    webSocketChannel.sink.add('{"name":"' + result[1] + '"}');
  }

  List<String> _getPathAux(Node n, List<String> ret) {
    Node? p = _treeViewController.getParent(n.key);
    ret.add(n.label.split(":")[0]);
    if (p!.key == n.key) {
      return ret;
    }
    return _getPathAux(p, ret);
  }

  List<String> _getPath(Node n) {
    return List.from((_getPathAux(n, [])).reversed);
  }

  Future<dynamic> _performRpc(String functionName, data) {
    String rpcKey = Utilities.generateRandom();
    Map<String, dynamic> req = {
      "type": "rpc_call",
      "rpc_key": rpcKey,
      "function_name": functionName,
      "data": data,
    };
    Completer<dynamic> ret = Completer();
    _rpcCompleters[rpcKey] = ret;
    webSocketChannel.sink.add(jsonEncode(req));
    return ret.future;
  }

  _performAction(String key) async {
    Node n = _treeViewController.getNode(key)!;
    String path = _getPath(n).join(".");
    List<dynamic> actions = await _performRpc("actions_for_node", path);
    final result = await Navigator.push(
        context,
        MaterialPageRoute(
          builder: (context) => ActionPage(path, actions),
        ));
    Map<String, dynamic> ret = {
      "type": "action_taken",
      "target": path,
      "action": result,
    };
    webSocketChannel.sink.add(jsonEncode(ret));
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(
          title: Text("Glaze game"),
        ),
        body: Column(children: [
          Expanded(
            flex: 7,
            child: TreeView(
                shrinkWrap: true,
                controller: _treeViewController,
                allowParentSelect: true,
                supportParentDoubleTap: true,
                onExpansionChanged: (key, expanded) {
                  return _expandNode(key, expanded);
                },
                onNodeDoubleTap: _performAction,
                onNodeTap: (key) {
                  Node n = _treeViewController.getNode(key)!;
                  _expandNode(key, !n.expanded);
                },
                theme: treeViewTheme),
          ),
          const Divider(
            height: 20,
            thickness: 5,
            indent: 20,
            endIndent: 20,
          ),
          Expanded(
              flex: 3,
              child: ListView.builder(
                  padding: const EdgeInsets.all(8),
                  itemCount: logMessages.length,
                  controller: _scrollController,
                  itemBuilder: (BuildContext context, int index) {
                    return Container(
                      child: Text(
                        logMessages[index],
                        style: TextStyle(fontSize: 16),
                      ),
                    );
                  }))
        ]));
  }
}
