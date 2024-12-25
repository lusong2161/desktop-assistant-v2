import 'package:flutter/material.dart';
import '../services/file_service.dart';
import '../widgets/file_list.dart';

class FileTransferScreen extends StatefulWidget {
  const FileTransferScreen({Key? key}) : super(key: key);

  @override
  _FileTransferScreenState createState() => _FileTransferScreenState();
}

class _FileTransferScreenState extends State<FileTransferScreen> {
  final FileService _fileService = FileService();
  final List<String> _sharedFiles = [];

  Future<void> _pickAndShareFile() async {
    final filePath = await _fileService.pickFile();
    if (filePath != null) {
      await _fileService.shareFile(filePath, ['user1', 'user2']);
      setState(() {
        _sharedFiles.add(filePath);
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('文件传输'),
      ),
      body: Column(
        children: [
          Expanded(
            child: FileList(
              files: _sharedFiles,
              onFileSelected: (file) {
                // 处理文件选择
              },
            ),
          ),
          Padding(
            padding: const EdgeInsets.all(16.0),
            child: ElevatedButton(
              onPressed: _pickAndShareFile,
              child: const Text('选择并共享文件'),
            ),
          ),
        ],
      ),
    );
  }
}
