import 'dart:async';

enum CommandType {
  system,    // 系统命令
  chat,      // 聊天命令
  file,      // 文件操作
  document,  // 文档操作
}

class Command {
  final String raw;
  final CommandType type;
  final Map<String, dynamic> parameters;

  Command({
    required this.raw,
    required this.type,
    required this.parameters,
  });
}

class CommandProcessor {
  static final RegExp _systemCommandPattern = RegExp(r'^/system\s+(.+)$');
  static final RegExp _fileCommandPattern = RegExp(r'^/file\s+(.+)$');
  static final RegExp _documentCommandPattern = RegExp(r'^/doc\s+(.+)$');

  Future<Command?> processInput(String input) async {
    try {
      if (input.startsWith('/')) {
        if (_systemCommandPattern.hasMatch(input)) {
          return _processSystemCommand(input);
        } else if (_fileCommandPattern.hasMatch(input)) {
          return _processFileCommand(input);
        } else if (_documentCommandPattern.hasMatch(input)) {
          return _processDocumentCommand(input);
        }
      }
      
      // 如果不是特殊命令，则作为聊天消息处理
      return Command(
        raw: input,
        type: CommandType.chat,
        parameters: {'message': input},
      );
    } catch (e) {
      print('命令处理错误: $e');
      return null;
    }
  }

  Command _processSystemCommand(String input) {
    final match = _systemCommandPattern.firstMatch(input);
    final command = match?.group(1) ?? '';
    return Command(
      raw: input,
      type: CommandType.system,
      parameters: {'command': command},
    );
  }

  Command _processFileCommand(String input) {
    final match = _fileCommandPattern.firstMatch(input);
    final command = match?.group(1) ?? '';
    return Command(
      raw: input,
      type: CommandType.file,
      parameters: {'command': command},
    );
  }

  Command _processDocumentCommand(String input) {
    final match = _documentCommandPattern.firstMatch(input);
    final command = match?.group(1) ?? '';
    return Command(
      raw: input,
      type: CommandType.document,
      parameters: {'command': command},
    );
  }
}
