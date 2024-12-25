import 'package:flutter/material.dart';
import '../models/document.dart';
import '../services/document_service.dart';
import '../widgets/document_preview.dart';

class DocumentScreen extends StatefulWidget {
  const DocumentScreen({Key? key}) : super(key: key);

  @override
  _DocumentScreenState createState() => _DocumentScreenState();
}

class _DocumentScreenState extends State<DocumentScreen> {
  final DocumentService _documentService = DocumentService();
  Document? _currentDocument;
  bool _isEditing = false;

  Future<void> _createNewDocument() async {
    final document = await _documentService.createDocument(
      '新建文档',
      '# 新建文档\n\n在此处开始编辑...',
      'current_user',
    );
    setState(() {
      _currentDocument = document;
      _isEditing = true;
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('文档管理'),
        actions: [
          if (_currentDocument != null)
            IconButton(
              icon: Icon(_isEditing ? Icons.save : Icons.edit),
              onPressed: () {
                setState(() {
                  _isEditing = !_isEditing;
                });
              },
            ),
        ],
      ),
      body: _currentDocument == null
          ? Center(
              child: ElevatedButton(
                onPressed: _createNewDocument,
                child: const Text('创建新文档'),
              ),
            )
          : DocumentPreview(
              document: _currentDocument!,
              isEditing: _isEditing,
              onContentChanged: (content) {
                setState(() {
                  _currentDocument = Document(
                    id: _currentDocument!.id,
                    title: _currentDocument!.title,
                    content: content,
                    ownerId: _currentDocument!.ownerId,
                    sharedWith: _currentDocument!.sharedWith,
                    lastModified: DateTime.now(),
                    type: _currentDocument!.type,
                  );
                });
              },
            ),
    );
  }
}
