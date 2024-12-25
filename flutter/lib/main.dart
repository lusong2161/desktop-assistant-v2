import 'package:flutter/material.dart';
import 'package:window_manager/window_manager.dart';
import 'package:file_picker/file_picker.dart';
import 'package:shared_preferences/shared_preferences.dart';
import 'package:http/http.dart' as http;
import 'package:path_provider/path_provider.dart';
import 'package:flutter_markdown/flutter_markdown.dart';
import 'dart:io';
import 'dart:convert';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await windowManager.ensureInitialized();
  WindowOptions windowOptions = WindowOptions(
    size: Size(800, 600),
    center: true,
    title: '智能助手测试程序',
    backgroundColor: Colors.transparent,
    skipTaskbar: false,
    titleBarStyle: TitleBarStyle.normal,
  );
  await windowManager.waitUntilReadyToShow(windowOptions, () async {
    await windowManager.show();
    await windowManager.focus();
  });
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: '智能助手测试程序',
      theme: ThemeData(
        primarySwatch: Colors.blue,
        useMaterial3: true,
      ),
      home: const MyHomePage(),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({Key? key}) : super(key: key);

  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  final TextEditingController _messageController = TextEditingController();
  final List<String> _messages = [];
  String? _selectedFilePath;
  String _markdownContent = '';

  // 测试智能对话系统
  Future<void> _testAIDialog() async {
    try {
      final response = await http.post(
        Uri.parse('http://localhost:8080/api/chat'),
        headers: {'Content-Type': 'application/json'},
        body: json.encode({'message': _messageController.text}),
      );
      
      if (response.statusCode == 200) {
        setState(() {
          _messages.add('发送: ${_messageController.text}');
          _messages.add('接收: ${response.body}');
          _messageController.clear();
        });
      }
    } catch (e) {
      print('AI对话测试: $e');
      setState(() {
        _messages.add('AI对话测试失败: $e');
      });
    }
  }

  // 测试文件传输
  Future<void> _testFileTransfer() async {
    try {
      FilePickerResult? result = await FilePicker.platform.pickFiles();
      if (result != null) {
        setState(() {
          _selectedFilePath = result.files.single.path;
          _messages.add('已选择文件: $_selectedFilePath');
        });
      }
    } catch (e) {
      print('文件传输测试: $e');
      setState(() {
        _messages.add('文件传输测试失败: $e');
      });
    }
  }

  // 测试文档预览
  Future<void> _testDocumentPreview() async {
    setState(() {
      _markdownContent = '''
# 测试文档
这是一个测试文档，用于验证文档预览功能。

## 功能列表
1. 文档编辑
2. 实时预览
3. 格式化支持
''';
    });
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('智能助手功能测试'),
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
      ),
      body: Column(
        children: [
          Expanded(
            child: ListView.builder(
              itemCount: _messages.length,
              itemBuilder: (context, index) {
                return ListTile(
                  title: Text(_messages[index]),
                );
              },
            ),
          ),
          if (_markdownContent.isNotEmpty)
            Expanded(
              child: Markdown(data: _markdownContent),
            ),
          Padding(
            padding: const EdgeInsets.all(8.0),
            child: Row(
              children: [
                Expanded(
                  child: TextField(
                    controller: _messageController,
                    decoration: const InputDecoration(
                      hintText: '输入测试消息...',
                      border: OutlineInputBorder(),
                    ),
                  ),
                ),
                IconButton(
                  icon: const Icon(Icons.send),
                  onPressed: _testAIDialog,
                ),
              ],
            ),
          ),
          Padding(
            padding: const EdgeInsets.all(8.0),
            child: Row(
              mainAxisAlignment: MainAxisAlignment.spaceEvenly,
              children: [
                ElevatedButton(
                  onPressed: _testFileTransfer,
                  child: const Text('测试文件传输'),
                ),
                ElevatedButton(
                  onPressed: _testDocumentPreview,
                  child: const Text('测试文档预览'),
                ),
              ],
            ),
          ),
        ],
      ),
    );
  }
}
