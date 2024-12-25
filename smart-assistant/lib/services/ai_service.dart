import 'package:http/http.dart' as http;
import 'dart:convert';

class AIService {
  final String baseUrl;

  AIService({required this.baseUrl});

  Future<String> processMessage(String message) async {
    try {
      final response = await http.post(
        Uri.parse('$baseUrl/api/chat'),
        headers: {'Content-Type': 'application/json'},
        body: json.encode({'message': message}),
      );

      if (response.statusCode == 200) {
        return response.body;
      } else {
        throw Exception('智能对话服务响应错误: ${response.statusCode}');
      }
    } catch (e) {
      throw Exception('智能对话服务连接失败: $e');
    }
  }
}
