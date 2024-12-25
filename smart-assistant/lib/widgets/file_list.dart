import 'package:flutter/material.dart';

class FileList extends StatelessWidget {
  final List<String> files;
  final Function(String) onFileSelected;

  const FileList({
    Key? key,
    required this.files,
    required this.onFileSelected,
  }) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return ListView.builder(
      itemCount: files.length,
      itemBuilder: (context, index) {
        final file = files[index];
        return ListTile(
          leading: const Icon(Icons.file_present),
          title: Text(file),
          onTap: () => onFileSelected(file),
        );
      },
    );
  }
}
