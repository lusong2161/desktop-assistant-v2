using System;
using System.Windows.Input;
using System.ComponentModel;

namespace SmartAssistant.UI.ViewModels
{
    public class MainViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        private string _statusMessage;
        public string StatusMessage
        {
            get => _statusMessage;
            set
            {
                _statusMessage = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(StatusMessage)));
            }
        }

        public ICommand OpenChatCommand { get; }
        public ICommand OpenFileTransferCommand { get; }
        public ICommand OpenDocumentCommand { get; }

        public MainViewModel()
        {
            OpenChatCommand = new RelayCommand(OpenChat);
            OpenFileTransferCommand = new RelayCommand(OpenFileTransfer);
            OpenDocumentCommand = new RelayCommand(OpenDocument);
            StatusMessage = "就绪";
        }

        private void OpenChat()
        {
            var chatWindow = new Views.ChatWindow();
            chatWindow.Show();
        }

        private void OpenFileTransfer()
        {
            var fileTransferDialog = new Views.FileTransferDialog();
            fileTransferDialog.ShowDialog();
        }

        private void OpenDocument()
        {
            var documentWindow = new Views.DocumentWindow();
            documentWindow.Show();
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

        public event EventHandler CanExecuteChanged
        {
            add => CommandManager.RequerySuggested += value;
            remove => CommandManager.RequerySuggested -= value;
        }

        public bool CanExecute(object parameter) => _canExecute?.Invoke() ?? true;
        public void Execute(object parameter) => _execute();
    }
}
