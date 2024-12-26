import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:smart_assistant/main.dart';

void main() {
  testWidgets('智能助手应用基本UI测试', (WidgetTester tester) async {
    await tester.pumpWidget(const MyApp());

    // 验证基本UI元素存在
    expect(find.byType(AppBar), findsOneWidget);
    expect(find.byType(TextField), findsOneWidget);
    expect(find.byType(IconButton), findsOneWidget);
    
    // 验证按钮存在
    expect(find.byType(ElevatedButton), findsNWidgets(3));
    
    // 验证列表视图存在
    expect(find.byType(ListView), findsAtLeastNWidgets(1));
  });
}
