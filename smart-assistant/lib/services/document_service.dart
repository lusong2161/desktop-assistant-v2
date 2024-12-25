import '../models/document.dart';

class DocumentService {
  Future<Document> createDocument(String title, String content, String ownerId) async {
    try {
      return Document(
        id: DateTime.now().millisecondsSinceEpoch.toString(),
        title: title,
        content: content,
        ownerId: ownerId,
        sharedWith: [],
        lastModified: DateTime.now(),
        type: DocumentType.text,
      );
    } catch (e) {
      throw Exception('文档创建失败: $e');
    }
  }

  Future<void> shareDocument(String documentId, List<String> users) async {
    try {
      // 实现文档共享逻辑
      print('文档共享: $documentId 与 $users');
    } catch (e) {
      throw Exception('文档共享失败: $e');
    }
  }
}
