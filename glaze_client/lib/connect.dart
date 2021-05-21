import 'package:flutter/material.dart';
import 'dart:io' show Platform;

class ConnectPage extends StatefulWidget {
  ConnectPage(Key key) : super(key: key);

  @override
  _ConnectPageState createState() => _ConnectPageState();
}

class _ConnectPageState extends State<ConnectPage> {
  final _formKey = GlobalKey<FormState>();
  final _serverTextController = TextEditingController(
      text: Platform.isAndroid ? "10.0.2.2:8080" : "127.0.0.1:8080");
  final _nameTextController = TextEditingController(text: "siemka");

  @override
  Widget build(BuildContext context) {
    return Scaffold(
        appBar: AppBar(
          title: Text("Connect to a Glaze master"),
        ),
        body: Form(
          key: _formKey,
          child: Column(
            children: <Widget>[
              TextFormField(
                validator: (value) {
                  if (value == null || value.isEmpty) {
                    return 'Please enter the server address';
                  }
                  return null;
                },
                decoration: InputDecoration(labelText: 'server address'),
                controller: _serverTextController,
              ),
              TextFormField(
                validator: (value) {
                  if (value == null || value.isEmpty) {
                    return 'Please enter your name';
                  }
                  return null;
                },
                decoration: InputDecoration(labelText: 'your name'),
                controller: _nameTextController,
              ),
              ElevatedButton(
                onPressed: () {
                  // Validate returns true if the form is valid, or false otherwise.
                  if (_formKey.currentState!.validate()) {
                    Navigator.pop(context, [
                      _serverTextController.text,
                      _nameTextController.text,
                    ]);
                  }
                },
                child: Text('Connect'),
              ),
            ],
          ),
        ));
  }
}
