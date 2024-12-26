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
  bool _isEditing = false;
  List<String> _onlineUsers = ['用户A', '用户B', '用户C'];
  List<String> _sharedFiles = [];
  String _currentDocument = '';

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

  // 测试文档预览和协作
  Future<void> _testDocumentPreview() async {
    setState(() {
      _markdownContent = '''
# 协同办公文档
这是一个协同办公测试文档。

## 功能列表
1. 多人实时编辑
2. 文档版本控制
3. 在线协作
4. 即时预览
''';
      _isEditing = true;
    });
  }

  // 测试用户通信
  void _testUserCommunication(String user) {
    setState(() {
      _messages.add('向 $user 发送消息: ${_messageController.text}');
      _messageController.clear();
    });
  }

  // 测试文件共享
  Future<void> _shareFile() async {
    if (_selectedFilePath != null) {
      setState(() {
        _sharedFiles.add('共享文件: $_selectedFilePath');
        _messages.add('文件已共享: $_selectedFilePath');
      });
    }
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('智能助手功能测试'),
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
      ),
      body: Row(
        children: <Widget>[
          // 左侧用户列表
          SizedBox(
            width: 200,
            child: Card(
              child: ListView.builder(
                itemCount: _onlineUsers.length,
                itemBuilder: (context, index) {
                  return ListTile(
                    leading: const Icon(Icons.person),
                    title: Text(_onlineUsers[index]),
                    onTap: () => _testUserCommunication(_onlineUsers[index]),
                  );
                },
              ),
            ),
          ),
          // 主要内容区域
          Expanded(
            child: Column(
              crossAxisAlignment: CrossAxisAlignment.stretch,
              children: <Widget>[
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
                    children: <Widget>[
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
                    children: <Widget>[
                      ElevatedButton(
                        onPressed: _testFileTransfer,
                        child: const Text('选择文件'),
                      ),
                      ElevatedButton(
                        onPressed: _shareFile,
                        child: const Text('共享文件'),
                      ),
                      ElevatedButton(
                        onPressed: _testDocumentPreview,
                        child: const Text('协同文档'),
                      ),
                    ],
                  ),
                ),
                if (_sharedFiles.isNotEmpty)
                  Container(
                    height: 100,
                    padding: const EdgeInsets.all(8.0),
                    child: Card(
                      child: ListView.builder(
                        itemCount: _sharedFiles.length,
                        itemBuilder: (context, index) {
                          return ListTile(
                            leading: const Icon(Icons.file_present),
                            title: Text(_sharedFiles[index]),
                          );
                        },
                      ),
                    ),
                  ),
              ],
            ),
          ),
        ],
      ),
    );
  }
}
