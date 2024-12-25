using System;
using System.Collections.ObjectModel;
using System.Windows.Input;
using System.Windows;
using SmartAssistant.Core;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows.Threading;

namespace SmartAssistant.UI.ViewModels
{
    public class ChatViewModel : INotifyPropertyChanged
    {
        private readonly NetworkService _networkService;
        private readonly FileTransferManager _fileTransferManager;
        private string _searchText;
        private Friend _selectedFriend;
        private string _messageText;
        private ObservableCollection<ChatMessage> _messages;
        private ObservableCollection<Friend> _friends;

        public ChatViewModel(NetworkService networkService, FileTransferManager fileTransferManager)
        {
            _networkService = networkService;
            _fileTransferManager = fileTransferManager;
            Messages = new ObservableCollection<ChatMessage>();
            Friends = new ObservableCollection<Friend>();

            // Commands
            SendMessageCommand = new RelayCommand(SendMessage, CanSendMessage);
            SendFileCommand = new RelayCommand(SendFile, CanSendFile);
            AddFriendCommand = new RelayCommand(AddFriend);
            VideoCallCommand = new RelayCommand(InitiateVideoCall, CanInitiateVideoCall);

            // Register network callbacks
            _networkService.RegisterCallback(new NetworkCallbackHandler(this));

            // Load initial data
            LoadFriends();
        }

        public string SearchText
        {
            get => _searchText;
            set
            {
                if (_searchText != value)
                {
                    _searchText = value;
                    OnPropertyChanged();
                    FilterFriends();
                }
            }
        }

        public Friend SelectedFriend
        {
            get => _selectedFriend;
            set
            {
                if (_selectedFriend != value)
                {
                    _selectedFriend = value;
                    OnPropertyChanged();
                    LoadMessages();
                }
            }
        }

        public string MessageText
        {
            get => _messageText;
            set
            {
                if (_messageText != value)
                {
                    _messageText = value;
                    OnPropertyChanged();
                    (SendMessageCommand as RelayCommand)?.RaiseCanExecuteChanged();
                }
            }
        }

        public ObservableCollection<ChatMessage> Messages
        {
            get => _messages;
            set
            {
                if (_messages != value)
                {
                    _messages = value;
                    OnPropertyChanged();
                }
            }
        }

        public ObservableCollection<Friend> Friends
        {
            get => _friends;
            set
            {
                if (_friends != value)
                {
                    _friends = value;
                    OnPropertyChanged();
                }
            }
        }

        public ICommand SendMessageCommand { get; }
        public ICommand SendFileCommand { get; }
        public ICommand AddFriendCommand { get; }
        public ICommand VideoCallCommand { get; }

        private async void SendMessage()
        {
            if (string.IsNullOrWhiteSpace(MessageText) || SelectedFriend == null)
                return;

            var message = new Message
            {
                Content = MessageText,
                SenderId = "current_user_id", // Replace with actual user ID
                ReceiverId = SelectedFriend.Id,
                Type = MessageType.Text,
                Timestamp = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds()
            };

            var result = await Task.Run(() => _networkService.SendMessage(message));
            if (result.IsSuccess)
            {
                Messages.Add(new ChatMessage
                {
                    Content = MessageText,
                    IsFromMe = true,
                    Time = DateTime.Now
                });
                MessageText = string.Empty;
            }
            else
            {
                MessageBox.Show($"Failed to send message: {result.GetError().GetMessage()}");
            }
        }

        private bool CanSendMessage()
        {
            return !string.IsNullOrWhiteSpace(MessageText) && SelectedFriend != null;
        }

        private async void SendFile()
        {
            var dialog = new Microsoft.Win32.OpenFileDialog();
            if (dialog.ShowDialog() == true)
            {
                var result = await Task.Run(() => 
                    _fileTransferManager.InitiateTransfer(dialog.FileName, SelectedFriend.Id));

                if (result.IsSuccess)
                {
                    var transferId = result.GetValue();
                    _fileTransferManager.RegisterProgressCallback(transferId, 
                        (current, total) => UpdateTransferProgress(transferId, current, total));
                }
                else
                {
                    MessageBox.Show($"Failed to initiate file transfer: {result.GetError().GetMessage()}");
                }
            }
        }

        private void UpdateTransferProgress(string transferId, ulong current, ulong total)
        {
            Application.Current.Dispatcher.Invoke(() =>
            {
                var progress = (double)current / total;
                // Update UI with transfer progress
                // You might want to add a progress bar to the message list
            });
        }

        private bool CanSendFile()
        {
            return SelectedFriend != null;
        }

        private void AddFriend()
        {
            // Show add friend dialog
            var dialog = new AddFriendDialog();
            if (dialog.ShowDialog() == true)
            {
                var userId = dialog.UserId;
                var result = _networkService.AddFriend(userId);
                if (!result.IsSuccess)
                {
                    MessageBox.Show($"Failed to add friend: {result.GetError().GetMessage()}");
                }
            }
        }

        private void InitiateVideoCall()
        {
            // Video call functionality to be implemented
            MessageBox.Show("Video call feature coming soon!");
        }

        private bool CanInitiateVideoCall()
        {
            return SelectedFriend != null && SelectedFriend.IsOnline;
        }

        private void LoadFriends()
        {
            var result = _networkService.GetFriendList();
            if (result.IsSuccess)
            {
                var friends = result.GetValue();
                foreach (var friend in friends)
                {
                    Friends.Add(new Friend
                    {
                        Id = friend.UserId,
                        Name = friend.UserId, // You might want to get actual names from a user service
                        IsOnline = friend.IsOnline,
                        Status = friend.StatusMessage
                    });
                }
            }
        }

        private void FilterFriends()
        {
            if (string.IsNullOrWhiteSpace(SearchText))
            {
                LoadFriends();
                return;
            }

            var filtered = Friends.Where(f => 
                f.Name.Contains(SearchText, StringComparison.OrdinalIgnoreCase)).ToList();
            Friends.Clear();
            foreach (var friend in filtered)
            {
                Friends.Add(friend);
            }
        }

        private void LoadMessages()
        {
            Messages.Clear();
            if (SelectedFriend == null) return;

            // Load messages from local storage or server
            // This is a placeholder for actual message loading
        }

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }

        private class NetworkCallbackHandler : INetworkCallback
        {
            private readonly ChatViewModel _viewModel;

            public NetworkCallbackHandler(ChatViewModel viewModel)
            {
                _viewModel = viewModel;
            }

            public void OnMessageReceived(Message message)
            {
                Application.Current.Dispatcher.Invoke(() =>
                {
                    if (_viewModel.SelectedFriend?.Id == message.SenderId)
                    {
                        _viewModel.Messages.Add(new ChatMessage
                        {
                            Content = message.Content,
                            IsFromMe = false,
                            Time = DateTime.Now
                        });
                    }
                });
            }

            public void OnStatusChanged(UserStatus status)
            {
                Application.Current.Dispatcher.Invoke(() =>
                {
                    var friend = _viewModel.Friends.FirstOrDefault(f => f.Id == status.UserId);
                    if (friend != null)
                    {
                        friend.IsOnline = status.IsOnline;
                        friend.Status = status.StatusMessage;
                    }
                });
            }

            public void OnConnectionStatusChanged(ConnectionStatus status)
            {
                // Handle connection status changes
            }

            public void OnTransferProgress(string transferId, ulong bytesTransferred, ulong totalBytes)
            {
                _viewModel.UpdateTransferProgress(transferId, bytesTransferred, totalBytes);
            }
        }
    }

    public class RelayCommand : ICommand
    {
        private readonly Action _execute;
        private readonly Func<bool> _canExecute;

        public RelayCommand(Action execute, Func<bool> canExecute = null)
        {
            _execute = execute ?? throw new ArgumentNullException(nameof(execute));
            _canExecute = canExecute;
        }

        public event EventHandler CanExecuteChanged;

        public bool CanExecute(object parameter)
        {
            return _canExecute?.Invoke() ?? true;
        }

        public void Execute(object parameter)
        {
            _execute();
        }

        public void RaiseCanExecuteChanged()
        {
            CanExecuteChanged?.Invoke(this, EventArgs.Empty);
        }
    }
}
