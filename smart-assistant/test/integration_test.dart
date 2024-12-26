import 'package:flutter_test/flutter_test.dart';
import 'package:smart_assistant/services/ai_service.dart';
import 'package:smart_assistant/services/file_service.dart';
import 'package:smart_assistant/services/user_service.dart';

void main() {
  group('集成测试', () {
    late AIService aiService;
    late FileService fileService;
    late UserService userService;

    setUp(() {
      aiService = AIService(baseUrl: 'http://test.api');
      fileService = FileService();
      userService = UserService();
    });

    test('文件服务和用户服务集成测试', () async {
      // 测试文件共享功能
      final testFile = await fileService.createTestFile();
      expect(testFile, isNotNull);

      // 测试用户权限
      final user = await userService.createTestUser();
      expect(user, isNotNull);

      // 测试文件共享权限
      final recipients = ['user1@example.com'];
      await fileService.shareFile(testFile, recipients);
      // 文件共享成功不会抛出异常
    });

    test('AI服务和用户服务集成测试', () async {
      // 测试AI对话
      final response = await aiService.processMessage('测试消息');
      expect(response, isNotNull);

      // 测试用户消息历史
      final user = await userService.createTestUser();
      final history = await aiService.getUserMessageHistory(user);
      expect(history, isNotNull);
    });
  });
}
