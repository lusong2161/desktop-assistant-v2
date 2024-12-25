import 'package:flutter/material.dart';
import '../services/ai_service.dart';
import '../services/user_service.dart';
import '../models/message.dart';
import '../widgets/chat_message.dart';
import '../widgets/user_list.dart';

class ChatScreen extends StatefulWidget {
  const ChatScreen({Key? key}) : super(key: key);

  @override
  _ChatScreenState createState() => _ChatScreenState();
}

class _ChatScreenState extends State<ChatScreen> {
  final TextEditingController _messageController = TextEditingController();
  final AIService _aiService = AIService(baseUrl: 'http://localhost:8080');
  final UserService _userService = UserService();
  final List<Message> _messages = [];

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('智能对话'),
      ),
      body: Row(
        children: [
          UserList(
            onUserSelected: (user) async {
              await _userService.sendMessage(user.id, _messageController.text);
            },
          ),
          Expanded(
            child: Column(
              children: [
                Expanded(
                  child: ListView.builder(
                    itemCount: _messages.length,
                    itemBuilder: (context, index) {
                      return ChatMessageWidget(message: _messages[index]);
                    },
                  ),
                ),
                Padding(
                  padding: const EdgeInsets.all(8.0),
                  child: Row(
                    children: [
                      Expanded(
                        child: TextField(
                          controller: _messageController,
                          decoration: const InputDecoration(
                            hintText: '输入消息...',
                            border: OutlineInputBorder(),
                          ),
                        ),
                      ),
                      IconButton(
                        icon: const Icon(Icons.send),
                        onPressed: () async {
                          if (_messageController.text.isNotEmpty) {
                            final response = await _aiService.processMessage(
                              _messageController.text,
                            );
                            
                            if (response['type'] == 'error') {
                              ScaffoldMessenger.of(context).showSnackBar(
                                SnackBar(content: Text(response['content'])),
                              );
                              return;
                            }
                            setState(() {
                              _messages.add(Message(
                                id: DateTime.now().toString(),
                                content: _messageController.text,
                                senderId: 'user',
                                receiverId: 'ai',
                                timestamp: DateTime.now(),
                                type: MessageType.text,
                              ));
                              _messages.add(Message(
                                id: DateTime.now().toString(),
                                content: response['content'],
                                senderId: 'ai',
                                receiverId: 'user',
                                timestamp: DateTime.now(),
                                type: MessageType.text,
                              ));
                            });
                            _messageController.clear();
                          }
                        },
                      ),
                    ],
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
