using System;
using System.Windows;
using System.Windows.Documents;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using SmartAssistant.Core;

namespace SmartAssistant.UI.Views
{
    public partial class DocumentPreviewWindow : Window, INotifyPropertyChanged
    {
        private readonly DocumentService _documentService;
        private readonly string _documentId;
        private readonly string _versionId;
        private DateTime _versionTimestamp;
        private string _versionUser;

        public DocumentPreviewWindow(DocumentService documentService, 
                                   string documentId, 
                                   string versionId)
        {
            InitializeComponent();
            DataContext = this;

            _documentService = documentService;
            _documentId = documentId;
            _versionId = versionId;

            CloseCommand = new RelayCommand(Close);

            LoadVersionContent();
        }

        public DateTime VersionTimestamp
        {
            get => _versionTimestamp;
            set
            {
                if (_versionTimestamp != value)
                {
                    _versionTimestamp = value;
                    OnPropertyChanged();
                }
            }
        }

        public string VersionUser
        {
            get => _versionUser;
            set
            {
                if (_versionUser != value)
                {
                    _versionUser = value;
                    OnPropertyChanged();
                }
            }
        }

        public RelayCommand CloseCommand { get; }

        private async void LoadVersionContent()
        {
            try
            {
                var historyResult = await Task.Run(() => 
                    _documentService.GetVersionHistory(_documentId));
                
                if (historyResult.IsSuccess)
                {
                    var versions = historyResult.GetValue();
                    var version = versions.Find(v => v.versionId == _versionId);
                    if (version != null)
                    {
                        VersionTimestamp = DateTimeOffset.FromUnixTimeMilliseconds(
                            version.timestamp).DateTime;
                        VersionUser = version.userId;
                        PreviewContent.Document = ConvertToFlowDocument(version.diff);
                    }
                }
                else
                {
                    MessageBox.Show($"Failed to load version: {historyResult.GetError().GetMessage()}");
                    Close();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error loading version: {ex.Message}");
                Close();
            }
        }

        private FlowDocument ConvertToFlowDocument(byte[] content)
        {
            // Implementation depends on document format
            // For now, just convert from plain text
            var text = System.Text.Encoding.UTF8.GetString(content);
            return new FlowDocument(new Paragraph(new Run(text)));
        }

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
