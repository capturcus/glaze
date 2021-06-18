import 'package:flutter/material.dart';

class ActionPage extends StatefulWidget {
  final String label;
  final List<dynamic> actions;
  ActionPage(this.label, this.actions);
  @override
  _ActionPageState createState() => _ActionPageState(label, actions);
}

class _ActionPageState extends State<ActionPage> {
  final String label;
  final List<dynamic> actions;
  _ActionPageState(this.label, this.actions);
  @override
  Widget build(BuildContext context) {
    Widget actionPrompt;
    if (label == "") {
      // global action
      actionPrompt = Text(
        "choose a global action:",
        style: TextStyle(fontSize: 20),
      );
    } else {
      actionPrompt = Column(
        crossAxisAlignment: CrossAxisAlignment.start,
        children: [
          Text(
            "choose an action for:",
            style: TextStyle(fontSize: 20),
          ),
          Text(
            this.label,
            style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
          )
        ],
      );
    }
    return Scaffold(
        appBar: AppBar(
          title: Text(this.label),
        ),
        body: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Padding(
              padding: EdgeInsets.all(16),
              child: actionPrompt,
            ),
            Divider(
              thickness: 3,
            ),
            ListView.builder(
                shrinkWrap: true,
                itemCount: actions.length * 2,
                itemBuilder: (context, i) {
                  if (i.isOdd) return Divider();

                  final index = i ~/ 2;
                  return ListTile(
                    contentPadding: EdgeInsets.all(16.0),
                    title:
                        Text(actions[index], style: TextStyle(fontSize: 18.0)),
                    onTap: () {
                      Navigator.pop(context, actions[index]);
                    },
                  );
                })
          ],
        ));
  }
}
