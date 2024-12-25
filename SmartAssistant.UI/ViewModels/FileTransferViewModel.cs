using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows.Input;

namespace SmartAssistant.UI.ViewModels
{
    public class FileTransferViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public ObservableCollection<TransferTask> TransferTasks { get; }
        public ICommand SelectFileCommand { get; }
        public ICommand SelectRecipientCommand { get; }

        public FileTransferViewModel()
        {
            TransferTasks = new ObservableCollection<TransferTask>();
            SelectFileCommand = new RelayCommand(SelectFile);
            SelectRecipientCommand = new RelayCommand(SelectRecipient);
        }

        private void SelectFile()
        {
            // TODO: 实现文件选择逻辑
        }

        private void SelectRecipient()
        {
            // TODO: 实现接收者选择逻辑
        }
    }

    public class TransferTask : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        private string _fileName;
        public string FileName
        {
            get => _fileName;
            set
            {
                _fileName = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(FileName)));
            }
        }

        private double _progress;
        public double Progress
        {
            get => _progress;
            set
            {
                _progress = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(Progress)));
            }
        }

        private bool _isPaused;
        public bool IsPaused
        {
            get => _isPaused;
            set
            {
                _isPaused = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(IsPaused)));
            }
        }

        public ICommand PauseResumeCommand { get; }
        public ICommand CancelCommand { get; }

        public TransferTask()
        {
            PauseResumeCommand = new RelayCommand(() => IsPaused = !IsPaused);
            CancelCommand = new RelayCommand(Cancel);
        }

        private void Cancel()
        {
            // TODO: 实现取消传输逻辑
        }
    }
}
