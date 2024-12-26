import 'package:file_picker/file_picker.dart';
import 'dart:io';

class FileService {
  Future<String?> pickFile() async {
    try {
      FilePickerResult? result = await FilePicker.platform.pickFiles();
      return result?.files.single.path;
    } catch (e) {
      throw Exception('文件选择失败: $e');
    }
  }

  Future<void> shareFile(String filePath, List<String> recipients) async {
    try {
      // 实现文件共享逻辑
      print('文件共享: $filePath 与 $recipients');
    } catch (e) {
      throw Exception('文件共享失败: $e');
    }
  }

  // 创建测试文件
  Future<String> createTestFile() async {
    try {
      final tempDir = Directory.systemTemp;
      final testFile = File('${tempDir.path}/test_file.txt');
      await testFile.writeAsString('测试文件内容');
      return testFile.path;
    } catch (e) {
      throw Exception('创建测试文件失败: $e');
    }
  }
}
