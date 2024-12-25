using System;
using System.Windows;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using SmartAssistant.Core;

namespace SmartAssistant.UI.Views
{
    public partial class FriendRequestDialog : Window, INotifyPropertyChanged
    {
        private readonly NetworkService _networkService;
        private readonly string _requestId;
        private readonly string _senderName;
        private readonly string _message;

        public FriendRequestDialog(NetworkService networkService, 
                                 string requestId, 
                                 string senderName, 
                                 string message = "")
        {
            InitializeComponent();
            DataContext = this;

            _networkService = networkService;
            _requestId = requestId;
            _senderName = senderName;
            _message = message;

            AcceptCommand = new RelayCommand(AcceptRequest);
            RejectCommand = new RelayCommand(RejectRequest);
        }

        public string SenderName => _senderName;
        public string Message => _message;

        public RelayCommand AcceptCommand { get; }
        public RelayCommand RejectCommand { get; }

        private async void AcceptRequest()
        {
            var result = await Task.Run(() => _networkService.AcceptFriendRequest(_requestId));
            if (result.IsSuccess)
            {
                DialogResult = true;
                Close();
            }
            else
            {
                MessageBox.Show($"Failed to accept friend request: {result.GetError().GetMessage()}");
            }
        }

        private async void RejectRequest()
        {
            var result = await Task.Run(() => _networkService.RejectFriendRequest(_requestId));
            if (result.IsSuccess)
            {
                DialogResult = false;
                Close();
            }
            else
            {
                MessageBox.Show($"Failed to reject friend request: {result.GetError().GetMessage()}");
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
