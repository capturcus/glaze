import 'package:flutter/material.dart';

class ActionPage extends StatefulWidget {
  final String label;
  ActionPage(this.label);
  @override
  _ActionPageState createState() => _ActionPageState(label);
}

class _ActionPageState extends State<ActionPage> {
  final String label;
  _ActionPageState(this.label);
  @override
  Widget build(BuildContext context) {
    return Container();
  }
}
