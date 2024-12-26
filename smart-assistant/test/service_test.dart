import 'package:flutter_test/flutter_test.dart';
import 'package:smart_assistant/services/file_service.dart';
import 'package:smart_assistant/services/user_service.dart';
import 'package:smart_assistant/services/document_service.dart';

void main() {
  group('服务测试', () {
    late FileService fileService;
    late UserService userService;
    late DocumentService documentService;

    setUp(() {
      fileService = FileService();
      userService = UserService();
      documentService = DocumentService();
    });

    test('文件服务测试', () async {
      final testFile = await fileService.createTestFile();
      expect(testFile, isNotNull);
    });

    test('用户服务测试', () async {
      final user = await userService.createTestUser();
      expect(user, isNotNull);
    });

    test('文档服务测试', () async {
      final doc = await documentService.createTestDocument();
      expect(doc, isNotNull);
    });
  });
}
