using System;
using System.Windows;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using SmartAssistant.Core;

namespace SmartAssistant.UI.Views
{
    public partial class FileTransferDialog : Window, INotifyPropertyChanged
    {
        private readonly string _transferId;
        private readonly NetworkService _networkService;
        private double _progress;
        private string _fileName;
        private string _progressText;
        private string _transferSpeed;
        private bool _isPaused;

        public FileTransferDialog(string transferId, string fileName, NetworkService networkService)
        {
            InitializeComponent();
            DataContext = this;

            _transferId = transferId;
            _fileName = fileName;
            _networkService = networkService;
            _isPaused = false;

            PauseCommand = new RelayCommand(PauseTransfer, () => !IsPaused);
            ResumeCommand = new RelayCommand(ResumeTransfer, () => IsPaused);
            CancelCommand = new RelayCommand(CancelTransfer);
        }

        public string FileName
        {
            get => _fileName;
            set
            {
                if (_fileName != value)
                {
                    _fileName = value;
                    OnPropertyChanged();
                }
            }
        }

        public double Progress
        {
            get => _progress;
            set
            {
                if (_progress != value)
                {
                    _progress = value;
                    OnPropertyChanged();
                    UpdateProgressText();
                }
            }
        }

        public string ProgressText
        {
            get => _progressText;
            set
            {
                if (_progressText != value)
                {
                    _progressText = value;
                    OnPropertyChanged();
                }
            }
        }

        public string TransferSpeed
        {
            get => _transferSpeed;
            set
            {
                if (_transferSpeed != value)
                {
                    _transferSpeed = value;
                    OnPropertyChanged();
                }
            }
        }

        public bool IsPaused
        {
            get => _isPaused;
            set
            {
                if (_isPaused != value)
                {
                    _isPaused = value;
                    OnPropertyChanged();
                    OnPropertyChanged(nameof(PauseButtonVisibility));
                    OnPropertyChanged(nameof(ResumeButtonVisibility));
                    (PauseCommand as RelayCommand)?.RaiseCanExecuteChanged();
                    (ResumeCommand as RelayCommand)?.RaiseCanExecuteChanged();
                }
            }
        }

        public Visibility PauseButtonVisibility => 
            IsPaused ? Visibility.Collapsed : Visibility.Visible;

        public Visibility ResumeButtonVisibility => 
            IsPaused ? Visibility.Visible : Visibility.Collapsed;

        public RelayCommand PauseCommand { get; }
        public RelayCommand ResumeCommand { get; }
        public RelayCommand CancelCommand { get; }

        private async void PauseTransfer()
        {
            var result = await Task.Run(() => _networkService.PauseFileTransfer(_transferId));
            if (result.IsSuccess)
            {
                IsPaused = true;
            }
            else
            {
                MessageBox.Show($"Failed to pause transfer: {result.GetError().GetMessage()}");
            }
        }

        private async void ResumeTransfer()
        {
            var result = await Task.Run(() => _networkService.ResumeFileTransfer(_transferId));
            if (result.IsSuccess)
            {
                IsPaused = false;
            }
            else
            {
                MessageBox.Show($"Failed to resume transfer: {result.GetError().GetMessage()}");
            }
        }

        private async void CancelTransfer()
        {
            if (MessageBox.Show("Are you sure you want to cancel this transfer?",
                              "Cancel Transfer",
                              MessageBoxButton.YesNo) == MessageBoxResult.Yes)
            {
                var result = await Task.Run(() => _networkService.CancelFileTransfer(_transferId));
                if (result.IsSuccess)
                {
                    Close();
                }
                else
                {
                    MessageBox.Show($"Failed to cancel transfer: {result.GetError().GetMessage()}");
                }
            }
        }

        private void UpdateProgressText()
        {
            ProgressText = $"{Progress:F1}%";
        }

        public void UpdateTransferSpeed(double bytesPerSecond)
        {
            string[] sizes = { "B/s", "KB/s", "MB/s", "GB/s" };
            int order = 0;
            double speed = bytesPerSecond;

            while (speed >= 1024 && order < sizes.Length - 1)
            {
                order++;
                speed = speed / 1024;
            }

            TransferSpeed = $"{speed:0.##} {sizes[order]}";
        }

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
