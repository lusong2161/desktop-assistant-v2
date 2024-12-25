import 'package:flutter_test/flutter_test.dart';
import 'package:smart_assistant/services/ai_service.dart';
import 'package:smart_assistant/services/command_processor.dart';
import 'package:http/http.dart' as http;
import 'package:mockito/mockito.dart';
import 'package:mockito/annotations.dart';

class MockHttpClient extends Mock implements http.Client {}

void main() {
  late AIService aiService;
  late MockHttpClient mockClient;

  setUp(() {
    mockClient = MockHttpClient();
    aiService = AIService(
      baseUrl: 'http://test.api',
      client: mockClient,
    );
  });

  group('AI服务测试', () {
    test('处理聊天消息', () async {
      when(mockClient.post(
        Uri.parse('http://test.api/api/chat'),
        headers: anyNamed('headers'),
        body: anyNamed('body'),
      )).thenAnswer((_) async => http.Response('{"response": "你好！"}', 200));

      final response = await aiService.processMessage('你好');
      expect(response['type'], equals('chat'));
      expect(response['content'], equals('你好！'));
    });

    test('处理错误响应', () async {
      when(mockClient.post(
        Uri.parse('http://test.api/api/chat'),
        headers: anyNamed('headers'),
        body: anyNamed('body'),
      )).thenAnswer((_) async => http.Response('服务器错误', 500));

      final response = await aiService.processMessage('你好');
      expect(response['type'], equals('error'));
      expect(response['content'], contains('错误'));
    });
  });
}
