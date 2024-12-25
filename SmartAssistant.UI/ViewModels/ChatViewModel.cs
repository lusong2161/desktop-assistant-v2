using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows.Input;

namespace SmartAssistant.UI.ViewModels
{
    public class ChatViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        private string _inputText;
        public string InputText
        {
            get => _inputText;
            set
            {
                _inputText = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(InputText)));
            }
        }

        public ObservableCollection<ChatMessage> Messages { get; }
        public ICommand SendCommand { get; }

        public ChatViewModel()
        {
            Messages = new ObservableCollection<ChatMessage>();
            SendCommand = new RelayCommand(SendMessage, () => !string.IsNullOrEmpty(InputText));
        }

        private void SendMessage()
        {
            if (string.IsNullOrEmpty(InputText)) return;
            
            Messages.Add(new ChatMessage { Text = InputText, IsFromUser = true });
            // TODO: 调用AI服务处理消息
            InputText = string.Empty;
        }
    }

    public class ChatMessage
    {
        public string Text { get; set; }
        public bool IsFromUser { get; set; }
    }
}
