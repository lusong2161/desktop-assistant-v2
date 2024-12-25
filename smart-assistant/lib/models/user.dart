class User {
  final String id;
  final String name;
  final String? avatar;
  final bool isOnline;

  User({
    required this.id,
    required this.name,
    this.avatar,
    this.isOnline = false,
  });
}
