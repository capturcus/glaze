import 'package:flutter/material.dart';
import 'package:flutter_treeview/flutter_treeview.dart';
import 'package:glaze_client/connect.dart';
import 'package:flutter/scheduler.dart';
import 'package:web_socket_channel/web_socket_channel.dart';
import 'package:web_socket_channel/status.dart' as status;

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

  _websocketListen(message) {
    debugPrint("ws message: " + message);
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
    webSocketChannel.sink.add('{"name":"'+result[1]+'"}');
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
    List<Node> nodes = [
      Node(
        label: 'Documents',
        key: 'docs',
        expanded: true,
        icon: Icons.folder,
        children: [
          Node(label: 'Job Search', key: 'd3', icon: Icons.input, children: [
            Node(
              label: 'Resume.docx',
              key: 'pd1',
              icon: Icons.insert_drive_file,
            ),
            Node(
                label: 'Cover Letter.docx',
                key: 'pd2',
                icon: Icons.insert_drive_file),
          ]),
          Node(
            label: 'Inspection.docx',
            key: 'd1',
          ),
          Node(label: 'Invoice.docx', key: 'd2', icon: Icons.insert_drive_file),
        ],
      ),
      Node(
          label: 'MeetingReport.xls',
          key: 'mrxls',
          icon: Icons.insert_drive_file),
      Node(
          label: 'MeetingReport.pdf',
          key: 'mrpdf',
          icon: Icons.insert_drive_file),
      Node(label: 'Demo.zip', key: 'demo', icon: Icons.archive),
    ];
    _treeViewController = TreeViewController(children: nodes);
    return Scaffold(
      appBar: AppBar(
        title: Text("Glaze game"),
      ),
      body: TreeView(
          controller: _treeViewController,
          allowParentSelect: false,
          supportParentDoubleTap: false,
          onExpansionChanged: (key, expanded) => _expandNode(key, expanded),
          onNodeTap: (key) {
            setState(() {
              _treeViewController =
                  _treeViewController.copyWith(selectedKey: key);
            });
          },
          theme: _treeViewTheme),
    );
  }
}
