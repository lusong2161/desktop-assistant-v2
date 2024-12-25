using System;
using System.Net.Http;
using System.Net.WebSockets;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;
using System.Collections.Generic;

namespace SmartAssistant.UI.Services
{
    public interface IApiService
    {
        Task<string> SendChatMessageAsync(string message);
        Task<bool> InitiateFileTransferAsync(string filePath, string recipientId);
        Task<bool> ConnectWebSocketAsync();
        Task<bool> AuthenticateAsync(string username, string password);
    }

    public class ApiService : IApiService, IDisposable
    {
        private readonly HttpClient _httpClient;
        private ClientWebSocket _webSocket;
        private readonly string _baseUrl;
        private readonly string _wsUrl;
        private bool _isDisposed;

        private readonly IConfigurationService _config;
        private string _authToken;

        public string AuthToken => _authToken;

        public ApiService(IConfigurationService config)
        {
            _config = config;
            _httpClient = new HttpClient();
            _webSocket = new ClientWebSocket();
            _baseUrl = _config.ApiBaseUrl;
            _wsUrl = _config.WebSocketUrl;
        }

        public async Task<string> SendChatMessageAsync(string message)
        {
            try
            {
                var content = new StringContent(
                    JsonSerializer.Serialize(new { message }), 
                    Encoding.UTF8, 
                    "application/json"
                );
                
                var response = await _httpClient.PostAsync($"{_baseUrl}/chat", content);
                response.EnsureSuccessStatusCode();
                
                var responseContent = await response.Content.ReadAsStringAsync();
                return responseContent;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"发送消息时出错: {ex.Message}");
                throw;
            }
        }

        public async Task<bool> InitiateFileTransferAsync(string filePath, string recipientId)
        {
            try
            {
                using var form = new MultipartFormDataContent();
                var fileContent = new ByteArrayContent(await System.IO.File.ReadAllBytesAsync(filePath));
                form.Add(fileContent, "file", System.IO.Path.GetFileName(filePath));
                form.Add(new StringContent(recipientId), "recipientId");

                var response = await _httpClient.PostAsync($"{_baseUrl}/files/transfer", form);
                return response.IsSuccessStatusCode;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"文件传输初始化失败: {ex.Message}");
                return false;
            }
        }

        public async Task<bool> ConnectWebSocketAsync()
        {
            try
            {
                if (_webSocket.State != WebSocketState.Open)
                {
                    await _webSocket.ConnectAsync(new Uri(_wsUrl), default);
                    _ = ReceiveWebSocketMessagesAsync();  // 启动后台接收任务
                }
                return true;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"WebSocket连接失败: {ex.Message}");
                return false;
            }
        }

        private async Task ReceiveWebSocketMessagesAsync()
        {
            var buffer = new byte[4096];
            try
            {
                while (_webSocket.State == WebSocketState.Open)
                {
                    var result = await _webSocket.ReceiveAsync(
                        new ArraySegment<byte>(buffer), default);

                    if (result.MessageType == WebSocketMessageType.Close)
                    {
                        await _webSocket.CloseAsync(
                            WebSocketCloseStatus.NormalClosure, 
                            string.Empty, 
                            default);
                    }
                    else
                    {
                        var message = Encoding.UTF8.GetString(buffer, 0, result.Count);
                        // 处理接收到的消息
                        OnMessageReceived(message);
                    }
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"WebSocket接收消息时出错: {ex.Message}");
            }
        }

        public event EventHandler<string> MessageReceived;
        protected virtual void OnMessageReceived(string message)
        {
            MessageReceived?.Invoke(this, message);
        }

        public async Task<bool> AuthenticateAsync(string username, string password)
        {
            try
            {
                var content = new StringContent(
                    JsonSerializer.Serialize(new { username, password }),
                    Encoding.UTF8,
                    "application/json"
                );

                var response = await _httpClient.PostAsync($"{_baseUrl}/auth/login", content);
                if (response.IsSuccessStatusCode)
                {
                    var tokenResponse = await response.Content.ReadAsStringAsync();
                    var tokenData = JsonSerializer.Deserialize<TokenResponse>(tokenResponse);
                    _authToken = tokenData.Token;
                    _httpClient.DefaultRequestHeaders.Authorization = 
                        new System.Net.Http.Headers.AuthenticationHeaderValue("Bearer", _authToken);
                    
                    // Start token refresh timer
                    StartTokenRefreshTimer(tokenData.ExpiresIn);
                    return true;
                }
                return false;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"认证失败: {ex.Message}");
                return false;
            }
        }

        private System.Threading.Timer _refreshTimer;
        private void StartTokenRefreshTimer(int expiresIn)
        {
            _refreshTimer?.Dispose();
            
            // Refresh token 5 minutes before expiration
            var refreshTime = TimeSpan.FromSeconds(expiresIn - 300);
            _refreshTimer = new System.Threading.Timer(
                async _ => await RefreshTokenAsync(),
                null,
                refreshTime,
                Timeout.InfiniteTimeSpan
            );
        }

        private async Task RefreshTokenAsync()
        {
            try
            {
                var response = await _httpClient.PostAsync($"{_baseUrl}/auth/refresh", null);
                if (response.IsSuccessStatusCode)
                {
                    var tokenResponse = await response.Content.ReadAsStringAsync();
                    var tokenData = JsonSerializer.Deserialize<TokenResponse>(tokenResponse);
                    _authToken = tokenData.Token;
                    _httpClient.DefaultRequestHeaders.Authorization = 
                        new System.Net.Http.Headers.AuthenticationHeaderValue("Bearer", _authToken);
                    
                    StartTokenRefreshTimer(tokenData.ExpiresIn);
                }
            }
            catch (Exception ex)
            {
                Console.WriteLine($"令牌刷新失败: {ex.Message}");
                // Notify authentication service about refresh failure
                OnAuthenticationFailed?.Invoke(this, EventArgs.Empty);
            }
        }

        public event EventHandler OnAuthenticationFailed;

        private class TokenResponse
        {
            public string Token { get; set; }
            public int ExpiresIn { get; set; }
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!_isDisposed)
            {
                if (disposing)
                {
                    _httpClient.Dispose();
                    _webSocket.Dispose();
                }
                _isDisposed = true;
            }
        }
    }
}
