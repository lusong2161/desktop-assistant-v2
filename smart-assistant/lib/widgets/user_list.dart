import 'package:flutter/material.dart';
import '../models/user.dart';
import '../services/user_service.dart';

class UserList extends StatefulWidget {
  final Function(User) onUserSelected;

  const UserList({
    Key? key,
    required this.onUserSelected,
  }) : super(key: key);

  @override
  _UserListState createState() => _UserListState();
}

class _UserListState extends State<UserList> {
  final UserService _userService = UserService();
  List<User> _users = [];

  @override
  void initState() {
    super.initState();
    _loadUsers();
  }

  Future<void> _loadUsers() async {
    final users = await _userService.getOnlineUsers();
    setState(() {
      _users = users;
    });
  }

  @override
  Widget build(BuildContext context) {
    return SizedBox(
      width: 200,
      child: Card(
        child: ListView.builder(
          itemCount: _users.length,
          itemBuilder: (context, index) {
            final user = _users[index];
            return ListTile(
              leading: CircleAvatar(
                child: Text(user.name[0]),
              ),
              title: Text(user.name),
              subtitle: Text(user.isOnline ? '在线' : '离线'),
              onTap: () => widget.onUserSelected(user),
            );
          },
        ),
      ),
    );
  }
}
