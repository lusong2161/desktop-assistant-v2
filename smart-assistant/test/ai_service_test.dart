import 'package:flutter_test/flutter_test.dart';
import 'package:smart_assistant/services/ai_service.dart';
import 'package:smart_assistant/services/command_processor.dart';
import 'package:http/http.dart' as http;
import 'package:mockito/mockito.dart';
import 'package:mockito/annotations.dart';
import 'dart:convert';

@GenerateMocks([http.Client])
part 'ai_service_test.mocks.dart';

void main() {
  late AIService aiService;
  late MockClient mockClient;

  setUp(() {
    mockClient = MockClient();
    aiService = AIService(
      baseUrl: 'http://test.api',
      client: mockClient,
    );
  });

  group('AI服务测试', () {
    test('处理聊天消息', () async {
      when(mockClient.post(
        Uri.parse('http://test.api/api/chat'),
        headers: {'Content-Type': 'application/json'},
        body: json.encode({
          'message': '你好',
          'type': 'chat',
        }),
      )).thenAnswer((_) async => http.Response('{"response": "你好！"}', 200));

      final response = await aiService.processMessage('/chat 你好');
      expect(response['type'], equals('chat'));
      expect(response['content'], equals('你好！'));
    });

    test('处理错误响应', () async {
      when(mockClient.post(
        Uri.parse('http://test.api/api/chat'),
        headers: {'Content-Type': 'application/json'},
        body: json.encode({
          'message': '你好',
          'type': 'chat',
        }),
      )).thenAnswer((_) async => http.Response('服务器错误', 500));

      final response = await aiService.processMessage('/chat 你好');
      expect(response['type'], equals('error'));
      expect(response['content'], contains('错误'));
    });
  });
}
