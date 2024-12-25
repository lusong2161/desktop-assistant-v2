import 'package:http/http.dart' as http;
import 'dart:convert';
import 'command_processor.dart';

class AIService {
  final String baseUrl;
  final CommandProcessor _commandProcessor;
  final http.Client _client;

  AIService({
    required this.baseUrl,
    http.Client? client,
  }) : _commandProcessor = CommandProcessor(),
       _client = client ?? http.Client();

  Future<Map<String, dynamic>> processMessage(String message) async {
    try {
      // 首先通过命令处理器处理输入
      final command = await _commandProcessor.processInput(message);
      
      if (command == null) {
        return {
          'type': 'error',
          'content': '命令格式错误',
        };
      }

      switch (command.type) {
        case CommandType.chat:
          return _processChatMessage(command);
        case CommandType.system:
          return _processSystemCommand(command);
        case CommandType.file:
          return _processFileCommand(command);
        case CommandType.document:
          return _processDocumentCommand(command);
        default:
          throw Exception('未知的命令类型');
      }
    } catch (e) {
      return {
        'type': 'error',
        'content': '处理失败: $e',
      };
    }
  }

  Future<Map<String, dynamic>> _processChatMessage(Command command) async {
    final response = await _client.post(
      Uri.parse('$baseUrl/api/chat'),
      headers: {'Content-Type': 'application/json'},
      body: json.encode({
        'message': command.parameters['message'],
        'type': 'chat',
      }),
    );

    if (response.statusCode == 200) {
      return {
        'type': 'chat',
        'content': json.decode(response.body)['response'],
      };
    } else {
      throw Exception('智能对话服务响应错误: ${response.statusCode}');
    }
  }

  Future<Map<String, dynamic>> _processSystemCommand(Command command) async {
    final response = await _client.post(
      Uri.parse('$baseUrl/api/system'),
      headers: {'Content-Type': 'application/json'},
      body: json.encode({
        'command': command.parameters['command'],
        'type': 'system',
      }),
    );

    if (response.statusCode == 200) {
      return {
        'type': 'system',
        'content': json.decode(response.body)['response'],
      };
    } else {
      throw Exception('系统命令执行失败: ${response.statusCode}');
    }
  }

  Future<Map<String, dynamic>> _processFileCommand(Command command) async {
    final response = await _client.post(
      Uri.parse('$baseUrl/api/file'),
      headers: {'Content-Type': 'application/json'},
      body: json.encode({
        'command': command.parameters['command'],
        'type': 'file',
      }),
    );

    if (response.statusCode == 200) {
      return {
        'type': 'file',
        'content': json.decode(response.body)['response'],
      };
    } else {
      throw Exception('文件命令执行失败: ${response.statusCode}');
    }
  }

  Future<Map<String, dynamic>> _processDocumentCommand(Command command) async {
    final response = await _client.post(
      Uri.parse('$baseUrl/api/document'),
      headers: {'Content-Type': 'application/json'},
      body: json.encode({
        'command': command.parameters['command'],
        'type': 'document',
      }),
    );

    if (response.statusCode == 200) {
      return {
        'type': 'document',
        'content': json.decode(response.body)['response'],
      };
    } else {
      throw Exception('文档命令执行失败: ${response.statusCode}');
    }
  }
}
