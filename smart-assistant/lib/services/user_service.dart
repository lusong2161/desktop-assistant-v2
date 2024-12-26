import '../models/user.dart';

class UserService {
  Future<List<User>> getOnlineUsers() async {
    try {
      // 模拟获取在线用户列表
      return [
        User(id: '1', name: '用户A', isOnline: true),
        User(id: '2', name: '用户B', isOnline: true),
        User(id: '3', name: '用户C', isOnline: false),
      ];
    } catch (e) {
      throw Exception('获取用户列表失败: $e');
    }
  }


  Future<void> sendMessage(String userId, String message) async {
    try {
      // 实现发送消息逻辑
      print('发送消息给 $userId: $message');
    } catch (e) {
      throw Exception('发送消息失败: $e');
    }
  }

  // 创建测试用户
  Future<User> createTestUser() async {
    try {
      return User(
        id: 'test-user-${DateTime.now().millisecondsSinceEpoch}',
        name: '测试用户',
        isOnline: true,
      );
    } catch (e) {
      throw Exception('创建测试用户失败: $e');
    }
  }
}
