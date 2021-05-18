import 'package:flutter/material.dart';
import 'package:flutter_treeview/flutter_treeview.dart';

void main() {
  runApp(MyApp());
}

class MyApp extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Flutter Demo',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: MyHomePage(title: 'Flutter Demo Home Page'),
    );
  }
}

class MyHomePage extends StatefulWidget {
  MyHomePage({Key key, this.title}) : super(key: key);

  final String title;

  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  int _counter = 0;
  TreeViewController _treeViewController;
  bool docsOpen = true;

  _expandNode(String key, bool expanded) {
    String msg = '${expanded ? "Expanded" : "Collapsed"}: $key';
    debugPrint(msg);
    Node node = _treeViewController.getNode(key);
    if (node != null) {
      List<Node> updated;
        updated = _treeViewController.updateNode(
            key, node.copyWith(expanded: expanded));
      setState(() {
        _treeViewController = _treeViewController.copyWith(children: updated);
      });
    }
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
        title: Text(widget.title),
      ),
      body: TreeView(
          controller: _treeViewController,
          allowParentSelect: false,
          supportParentDoubleTap: false,
          onExpansionChanged: (key, expanded) =>
                        _expandNode(key, expanded),
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
