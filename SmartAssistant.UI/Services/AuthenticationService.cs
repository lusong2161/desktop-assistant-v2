using System;
using System.Threading.Tasks;
using System.Security.Claims;
using System.IdentityModel.Tokens.Jwt;
using Microsoft.IdentityModel.Tokens;
using System.Text;

namespace SmartAssistant.UI.Services
{
    public interface IAuthenticationService
    {
        Task<bool> LoginAsync(string username, string password);
        Task<bool> LogoutAsync();
        bool IsAuthenticated { get; }
        string CurrentUser { get; }
        string AuthToken { get; }
    }

    public class AuthenticationService : IAuthenticationService
    {
        private readonly IApiService _apiService;
        private readonly IConfigurationService _config;
        private string _currentToken;
        private string _currentUser;

        public AuthenticationService(IApiService apiService, IConfigurationService config)
        {
            _apiService = apiService;
            _config = config;
        }

        public bool IsAuthenticated => !string.IsNullOrEmpty(_currentToken);
        public string CurrentUser => _currentUser;
        public string AuthToken => _currentToken;

        public async Task<bool> LoginAsync(string username, string password)
        {
            try
            {
                _apiService.OnAuthenticationFailed += HandleAuthenticationFailed;
                var success = await _apiService.AuthenticateAsync(username, password);
                if (success)
                {
                    _currentUser = username;
                    _currentToken = _apiService.AuthToken;
                    SaveAuthenticationState();
                    return true;
                }
                return false;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"登录失败: {ex.Message}");
                return false;
            }
        }

        private void HandleAuthenticationFailed(object sender, EventArgs e)
        {
            // Token refresh failed, user needs to re-authenticate
            _currentToken = null;
            _currentUser = null;
            OnAuthenticationExpired?.Invoke(this, EventArgs.Empty);
        }

        private void SaveAuthenticationState()
        {
            try
            {
                var state = new AuthState
                {
                    Username = _currentUser,
                    Token = _currentToken
                };
                
                var encrypted = _config.EncryptData(JsonSerializer.Serialize(state));
                File.WriteAllText(
                    Path.Combine(
                        Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData),
                        "SmartAssistant",
                        "auth.dat"
                    ),
                    encrypted
                );
            }
            catch (Exception ex)
            {
                Console.WriteLine($"保存认证状态失败: {ex.Message}");
            }
        }

        public event EventHandler OnAuthenticationExpired;

        private class AuthState
        {
            public string Username { get; set; }
            public string Token { get; set; }
        }

        public async Task<bool> LogoutAsync()
        {
            try
            {
                _currentToken = null;
                _currentUser = null;
                return true;
            }
            catch (Exception ex)
            {
                Console.WriteLine($"注销失败: {ex.Message}");
                return false;
            }
        }

        private bool ValidateToken(string token)
        {
            try
            {
                var tokenHandler = new JwtSecurityTokenHandler();
                var key = Encoding.ASCII.GetBytes(_config.JwtSecret);
                
                tokenHandler.ValidateToken(token, new TokenValidationParameters
                {
                    ValidateIssuerSigningKey = true,
                    IssuerSigningKey = new SymmetricSecurityKey(key),
                    ValidateIssuer = false,
                    ValidateAudience = false,
                    ClockSkew = TimeSpan.Zero
                }, out SecurityToken validatedToken);

                return true;
            }
            catch
            {
                return false;
            }
        }
    }
}
