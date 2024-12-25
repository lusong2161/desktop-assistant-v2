using System;
using System.IO;
using System.Text.Json;

namespace SmartAssistant.UI.Services
{
    public interface IConfigurationService
    {
        string ApiBaseUrl { get; }
        string WebSocketUrl { get; }
        string JwtSecret { get; }
        void LoadConfiguration();
        void SaveConfiguration();
        string EncryptData(string data);
        string DecryptData(string encryptedData);
    }

    public class ConfigurationService : IConfigurationService
    {
        private const string CONFIG_FILE = "config.json";
        private ConfigurationData _config;
        private readonly byte[] _encryptionKey;

        public string ApiBaseUrl => _config?.ApiBaseUrl ?? "https://localhost/api";
        public string WebSocketUrl => _config?.WebSocketUrl ?? "wss://localhost/ws";
        public string JwtSecret => _config?.JwtSecret ?? "your-jwt-secret-key";

        public ConfigurationService()
        {
            string keyBase64 = Environment.GetEnvironmentVariable("SMARTASSISTANT_KEY");
            if (string.IsNullOrEmpty(keyBase64))
            {
                _encryptionKey = new byte[32];
                using (var rng = new System.Security.Cryptography.RNGCryptoServiceProvider())
                {
                    rng.GetBytes(_encryptionKey);
                }
                Environment.SetEnvironmentVariable("SMARTASSISTANT_KEY", 
                    Convert.ToBase64String(_encryptionKey), 
                    EnvironmentVariableTarget.User);
            }
            else
            {
                _encryptionKey = Convert.FromBase64String(keyBase64);
            }

        public ConfigurationService()
        {
            LoadConfiguration();
        }

        public void LoadConfiguration()
        {
            try
            {
                if (File.Exists(CONFIG_FILE))
                {
                    var jsonString = File.ReadAllText(CONFIG_FILE);
                    _config = JsonSerializer.Deserialize<ConfigurationData>(jsonString);
                }
                else
                {
                    _config = new ConfigurationData
                    {
                        ApiBaseUrl = "https://localhost/api",
                        WebSocketUrl = "wss://localhost/ws"
                    };
                    SaveConfiguration();
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"配置加载失败: {ex.Message}");
                _config = new ConfigurationData
                {
                    ApiBaseUrl = "https://localhost/api",
                    WebSocketUrl = "wss://localhost/ws"
                };
            }
        }

        public void SaveConfiguration()
        {
            try
            {
                var jsonString = JsonSerializer.Serialize(_config, new JsonSerializerOptions
                {
                    WriteIndented = true
                });
                File.WriteAllText(CONFIG_FILE, jsonString);
            }
            catch (Exception ex)
            {
                Console.WriteLine($"配置保存失败: {ex.Message}");
            }
        }

        public string EncryptData(string data)
        {
            try
            {
                byte[] iv = new byte[16];
                using (var rng = new System.Security.Cryptography.RNGCryptoServiceProvider())
                {
                    rng.GetBytes(iv);
                }

                using (var aes = System.Security.Cryptography.Aes.Create())
                {
                    aes.Key = _encryptionKey;
                    aes.IV = iv;

                    using (var encryptor = aes.CreateEncryptor())
                    using (var msEncrypt = new System.IO.MemoryStream())
                    {
                        msEncrypt.Write(iv, 0, iv.Length);

                        using (var csEncrypt = new System.Security.Cryptography.CryptoStream(
                            msEncrypt, encryptor, System.Security.Cryptography.CryptoStreamMode.Write))
                        using (var swEncrypt = new System.IO.StreamWriter(csEncrypt))
                        {
                            swEncrypt.Write(data);
                        }

                        return Convert.ToBase64String(msEncrypt.ToArray());
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"加密失败: {ex.Message}");
                throw;
            }
        }

        public string DecryptData(string encryptedData)
        {
            try
            {
                byte[] fullCipher = Convert.FromBase64String(encryptedData);
                byte[] iv = new byte[16];
                byte[] cipher = new byte[fullCipher.Length - 16];

                Buffer.BlockCopy(fullCipher, 0, iv, 0, 16);
                Buffer.BlockCopy(fullCipher, 16, cipher, 0, fullCipher.Length - 16);

                using (var aes = System.Security.Cryptography.Aes.Create())
                {
                    aes.Key = _encryptionKey;
                    aes.IV = iv;

                    using (var decryptor = aes.CreateDecryptor())
                    using (var msDecrypt = new System.IO.MemoryStream(cipher))
                    using (var csDecrypt = new System.Security.Cryptography.CryptoStream(
                        msDecrypt, decryptor, System.Security.Cryptography.CryptoStreamMode.Read))
                    using (var srDecrypt = new System.IO.StreamReader(csDecrypt))
                    {
                        return srDecrypt.ReadToEnd();
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"解密失败: {ex.Message}");
                throw;
            }
        }

        private class ConfigurationData
        {
            public string ApiBaseUrl { get; set; }
            public string WebSocketUrl { get; set; }
            public string JwtSecret { get; set; }
        }
    }
}
