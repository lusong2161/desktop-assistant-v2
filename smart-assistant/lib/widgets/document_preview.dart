import 'package:flutter/material.dart';
import 'package:flutter_markdown/flutter_markdown.dart';
import '../models/document.dart';

class DocumentPreview extends StatelessWidget {
  final Document document;
  final bool isEditing;
  final Function(String)? onContentChanged;

  const DocumentPreview({
    Key? key,
    required this.document,
    this.isEditing = false,
    this.onContentChanged,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    if (isEditing) {
      return TextField(
        maxLines: null,
        decoration: const InputDecoration(
          border: InputBorder.none,
          contentPadding: EdgeInsets.all(16.0),
        ),
        controller: TextEditingController(text: document.content),
        onChanged: onContentChanged,
      );
    }

    return Markdown(
      data: document.content,
      selectable: true,
    );
  }
}
