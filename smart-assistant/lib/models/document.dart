class Document {
  final String id;
  final String title;
  final String content;
  final String ownerId;
  final List<String> sharedWith;
  final DateTime lastModified;
  final DocumentType type;

  Document({
    required this.id,
    required this.title,
    required this.content,
    required this.ownerId,
    required this.sharedWith,
    required this.lastModified,
    required this.type,
  });
}

enum DocumentType {
  text,
  spreadsheet,
  presentation,
}
