import 'package:flutter/material.dart';
import 'package:glaze_client/action.dart';
import 'package:glaze_client/game.dart';

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
        home: GamePage());
  }
}
