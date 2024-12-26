import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:smart_assistant/main.dart';

void main() {
  testWidgets('智能助手应用基本UI测试', (WidgetTester tester) async {
    await tester.pumpWidget(const MyApp());

    // 验证基本UI元素存在
    expect(find.byType(AppBar), findsOneWidget);
    expect(find.byType(FloatingActionButton), findsOneWidget);
    
    // 验证初始状态
    expect(find.byType(ListView), findsOneWidget);
  });
}
