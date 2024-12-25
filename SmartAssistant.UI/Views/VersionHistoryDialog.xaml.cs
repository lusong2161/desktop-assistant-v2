using System;
using System.Windows;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Collections.ObjectModel;
using SmartAssistant.Core;

namespace SmartAssistant.UI.Views
{
    public partial class VersionHistoryDialog : Window, INotifyPropertyChanged
    {
        private readonly DocumentService _documentService;
        private readonly string _documentId;
        private DocumentVersion _selectedVersion;

        public VersionHistoryDialog(DocumentService documentService, string documentId)
        {
            InitializeComponent();
            DataContext = this;

            _documentService = documentService;
            _documentId = documentId;
            Versions = new ObservableCollection<DocumentVersion>();

            PreviewCommand = new RelayCommand(PreviewVersion, () => SelectedVersion != null);
            RevertCommand = new RelayCommand(RevertToVersion, () => SelectedVersion != null);
            CloseCommand = new RelayCommand(Close);

            LoadVersionHistory();
        }

        public ObservableCollection<DocumentVersion> Versions { get; }

        public DocumentVersion SelectedVersion
        {
            get => _selectedVersion;
            set
            {
                if (_selectedVersion != value)
                {
                    _selectedVersion = value;
                    OnPropertyChanged();
                    (PreviewCommand as RelayCommand)?.RaiseCanExecuteChanged();
                    (RevertCommand as RelayCommand)?.RaiseCanExecuteChanged();
                }
            }
        }

        public RelayCommand PreviewCommand { get; }
        public RelayCommand RevertCommand { get; }
        public RelayCommand CloseCommand { get; }

        private async void LoadVersionHistory()
        {
            try
            {
                var result = await Task.Run(() => _documentService.GetVersionHistory(_documentId));
                if (result.IsSuccess)
                {
                    Versions.Clear();
                    foreach (var version in result.GetValue())
                    {
                        Versions.Add(version);
                    }
                }
                else
                {
                    MessageBox.Show($"Failed to load version history: {result.GetError().GetMessage()}");
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error loading version history: {ex.Message}");
            }
        }

        private void PreviewVersion()
        {
            if (SelectedVersion == null) return;

            try
            {
                var previewWindow = new DocumentPreviewWindow(_documentService, _documentId, 
                    SelectedVersion.versionId);
                previewWindow.Owner = this;
                previewWindow.ShowDialog();
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error previewing version: {ex.Message}");
            }
        }

        private async void RevertToVersion()
        {
            if (SelectedVersion == null) return;

            var result = MessageBox.Show(
                "Are you sure you want to revert to this version? " +
                "This will overwrite the current version.",
                "Confirm Revert",
                MessageBoxButton.YesNo,
                MessageBoxImage.Warning);

            if (result == MessageBoxResult.Yes)
            {
                try
                {
                    var revertResult = await Task.Run(() => 
                        _documentService.RevertToVersion(_documentId, SelectedVersion.versionId));
                    
                    if (revertResult.IsSuccess)
                    {
                        MessageBox.Show("Document reverted successfully.");
                        DialogResult = true;
                        Close();
                    }
                    else
                    {
                        MessageBox.Show($"Failed to revert document: {revertResult.GetError().GetMessage()}");
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show($"Error reverting document: {ex.Message}");
                }
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
