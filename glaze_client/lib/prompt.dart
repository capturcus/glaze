import 'package:flutter/material.dart';
import 'package:flutter/services.dart';

enum PromptType {
  text,
  choice,
  textResponse,
  numberResponse,
}

PromptType textToPromptType(String text) {
  if (text == "text") {
    return PromptType.text;
  } else if (text == "choice") {
    return PromptType.choice;
  } else if (text == "text_response") {
    return PromptType.textResponse;
  } else if (text == "number_response") {
    return PromptType.numberResponse;
  }
  throw Exception("unknown prompt type: " + text.toString());
}

class PromptPage extends StatefulWidget {
  final String prompt;
  final PromptType promptType;
  final data;
  PromptPage(this.prompt, this.promptType, this.data);
  @override
  _PromptPageState createState() => _PromptPageState(prompt, promptType, data);
}

class _PromptPageState extends State<PromptPage> {
  final String prompt;
  final PromptType promptType;
  final data;
  int selectedChoice = -1;
  _PromptPageState(this.prompt, this.promptType, this.data);

  Widget _buildText() {
    return ElevatedButton(
        onPressed: () {
          Navigator.pop(context);
        },
        child: Text("OK"));
  }

  Widget _buildChoice() {
    List<dynamic> choices = data;
    return Column(children: [
      ListView.builder(
          shrinkWrap: true,
          itemCount: choices.length * 2,
          itemBuilder: (context, i) {
            if (i.isOdd) return Divider();

            final index = i ~/ 2;
            return ListTile(
              selectedTileColor: Colors.blue.shade100,
              selected: index == selectedChoice,
              contentPadding: EdgeInsets.all(16.0),
              title: Text(choices[index], style: TextStyle(fontSize: 18.0)),
              onTap: () {
                setState(() {
                  selectedChoice = index;
                });
              },
            );
          }),
      ElevatedButton(
          onPressed: () {
            Navigator.pop(context, choices[selectedChoice]);
          },
          child: Text("OK")),
    ]);
  }

  Widget _buildTextResponse() {
    final controller = TextEditingController();
    return Column(children: [
      TextField(
        controller: controller,
        decoration: new InputDecoration(labelText: "Enter your text"),
      ),
      ElevatedButton(
          onPressed: () {
            Navigator.pop(context, controller.text);
          },
          child: Text("OK"))
    ]);
  }

  Widget _buildNumberResponse() {
    final controller = TextEditingController();
    return Column(children: [
      TextField(
        controller: controller,
        decoration: new InputDecoration(labelText: "Enter your number"),
        keyboardType: TextInputType.number,
        inputFormatters: <TextInputFormatter>[
          FilteringTextInputFormatter.digitsOnly
        ], // Only numbers can be entered
      ),
      ElevatedButton(
          onPressed: () {
            Navigator.pop(context, int.parse(controller.text));
          },
          child: Text("OK"))
    ]);
  }

  @override
  Widget build(BuildContext context) {
    Widget interaction;
    switch (promptType) {
      case PromptType.text:
        interaction = _buildText();
        break;
      case PromptType.choice:
        interaction = _buildChoice();
        break;
      case PromptType.textResponse:
        interaction = _buildTextResponse();
        break;
      case PromptType.numberResponse:
        interaction = _buildNumberResponse();
        break;
    }
    return Scaffold(
        appBar: AppBar(
          title: Text("alert"),
        ),
        body: Column(
          children: [
            Text(
              this.prompt,
              style: TextStyle(fontSize: 20),
            ),
            Divider(thickness: 3),
            interaction
          ],
        ));
  }
}
