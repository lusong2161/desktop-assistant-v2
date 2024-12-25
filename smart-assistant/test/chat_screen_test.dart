import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:smart_assistant/screens/chat_screen.dart';
import 'package:smart_assistant/services/ai_service.dart';
import 'package:mockito/mockito.dart';
import 'package:mockito/annotations.dart';

class MockAIService extends Mock implements AIService {}

void main() {
  late AIService mockAIService;

  setUp(() {
    mockAIService = MockAIService();
  });

  testWidgets('聊天界面测试', (WidgetTester tester) async {
    await tester.pumpWidget(
      MaterialApp(
        home: ChatScreen(),
      ),
    );

    // 验证界面元素存在
    expect(find.byType(TextField), findsOneWidget);
    expect(find.byIcon(Icons.send), findsOneWidget);

    // 测试发送消息
    await tester.enterText(find.byType(TextField), '你好');
    await tester.tap(find.byIcon(Icons.send));
    await tester.pump();

    // 验证消息显示
    expect(find.text('你好'), findsOneWidget);
  });

  testWidgets('错误处理测试', (WidgetTester tester) async {
    when(mockAIService.processMessage(any))
        .thenAnswer((_) async => {'type': 'chat', 'content': '你好！'});

    await tester.pumpWidget(
      MaterialApp(
        home: ChatScreen(),
      ),
    );

    await tester.enterText(find.byType(TextField), '你好');
    await tester.tap(find.byIcon(Icons.send));
    await tester.pump();

    expect(find.text('你好'), findsOneWidget);
    expect(find.text('你好！'), findsOneWidget);
  });
}
