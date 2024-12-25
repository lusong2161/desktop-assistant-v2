import 'package:flutter_test/flutter_test.dart';
import 'package:smart_assistant/services/command_processor.dart';

void main() {
  late CommandProcessor processor;

  setUp(() {
    processor = CommandProcessor();
  });

  group('命令处理器测试', () {
    test('处理系统命令', () async {
      final command = await processor.processInput('/system status');
      expect(command?.type, equals(CommandType.system));
      expect(command?.parameters['command'], equals('status'));
    });

    test('处理文件命令', () async {
      final command = await processor.processInput('/file share document.txt');
      expect(command?.type, equals(CommandType.file));
      expect(command?.parameters['command'], equals('share document.txt'));
    });

    test('处理文档命令', () async {
      final command = await processor.processInput('/doc edit report.md');
      expect(command?.type, equals(CommandType.document));
      expect(command?.parameters['command'], equals('edit report.md'));
    });

    test('处理普通聊天消息', () async {
      final command = await processor.processInput('你好，智能助手');
      expect(command?.type, equals(CommandType.chat));
      expect(command?.parameters['message'], equals('你好，智能助手'));
    });
  });
}
