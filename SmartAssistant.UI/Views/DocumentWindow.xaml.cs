using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using SmartAssistant.Core;
using System.Collections.ObjectModel;
using System.Windows.Threading;

namespace SmartAssistant.UI.Views
{
    public partial class DocumentWindow : Window, INotifyPropertyChanged, IDocumentCallback
    {
        private readonly DocumentService _documentService;
        private readonly string _documentId;
        private string _documentTitle;
        private bool _isReadOnly;
        private string _statusText;
        private FlowDocument _documentContent;
        private DispatcherTimer _autoSaveTimer;
        private bool _isDirty;

        public DocumentWindow(DocumentService documentService, string documentId)
        {
            InitializeComponent();
            DataContext = this;

            _documentService = documentService;
            _documentId = documentId;
            OnlineUsers = new ObservableCollection<OnlineUser>();

            InitializeCommands();
            InitializeAutoSave();
            LoadDocument();

            _documentService.RegisterCallback(this);
        }

        public string DocumentTitle
        {
            get => _documentTitle;
            set
            {
                if (_documentTitle != value)
                {
                    _documentTitle = value;
                    OnPropertyChanged();
                }
            }
        }

        public bool IsReadOnly
        {
            get => _isReadOnly;
            set
            {
                if (_isReadOnly != value)
                {
                    _isReadOnly = value;
                    OnPropertyChanged();
                }
            }
        }

        public string StatusText
        {
            get => _statusText;
            set
            {
                if (_statusText != value)
                {
                    _statusText = value;
                    OnPropertyChanged();
                }
            }
        }

        public FlowDocument DocumentContent
        {
            get => _documentContent;
            set
            {
                if (_documentContent != value)
                {
                    _documentContent = value;
                    OnPropertyChanged();
                }
            }
        }

        public ObservableCollection<OnlineUser> OnlineUsers { get; }

        public RelayCommand SaveCommand { get; private set; }
        public RelayCommand ShareCommand { get; private set; }
        public RelayCommand VersionHistoryCommand { get; private set; }
        public RelayCommand BoldCommand { get; private set; }
        public RelayCommand ItalicCommand { get; private set; }
        public RelayCommand UnderlineCommand { get; private set; }

        private void InitializeCommands()
        {
            SaveCommand = new RelayCommand(SaveDocument);
            ShareCommand = new RelayCommand(ShowShareDialog);
            VersionHistoryCommand = new RelayCommand(ShowVersionHistory);
            BoldCommand = new RelayCommand(ApplyBold);
            ItalicCommand = new RelayCommand(ApplyItalic);
            UnderlineCommand = new RelayCommand(ApplyUnderline);
        }

        private void InitializeAutoSave()
        {
            _autoSaveTimer = new DispatcherTimer
            {
                Interval = TimeSpan.FromMinutes(1)
            };
            _autoSaveTimer.Tick += AutoSave_Tick;
            _autoSaveTimer.Start();
        }

        private async void LoadDocument()
        {
            try
            {
                var openResult = await Task.Run(() => _documentService.OpenDocument(_documentId));
                if (!openResult.IsSuccess)
                {
                    MessageBox.Show($"Failed to open document: {openResult.GetError().GetMessage()}");
                    Close();
                    return;
                }

                var metadataResult = await Task.Run(() => _documentService.GetMetadata(_documentId));
                if (!metadataResult.IsSuccess)
                {
                    MessageBox.Show($"Failed to get document metadata: {metadataResult.GetError().GetMessage()}");
                    Close();
                    return;
                }

                var metadata = metadataResult.GetValue();
                DocumentTitle = metadata.title;
                IsReadOnly = !metadata.userPermissions.ContainsKey(CurrentUser.Id) || 
                           metadata.userPermissions[CurrentUser.Id] < DocumentPermission.Write;

                var contentResult = await Task.Run(() => _documentService.GetContent(_documentId));
                if (!contentResult.IsSuccess)
                {
                    MessageBox.Show($"Failed to get document content: {contentResult.GetError().GetMessage()}");
                    Close();
                    return;
                }

                DocumentContent = ConvertToFlowDocument(contentResult.GetValue());
                StatusText = "Document loaded successfully";
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error loading document: {ex.Message}");
                Close();
            }
        }

        private async void SaveDocument()
        {
            try
            {
                var content = ConvertFromFlowDocument(DocumentContent);
                var result = await Task.Run(() => _documentService.UpdateContent(_documentId, content));
                if (result.IsSuccess)
                {
                    _isDirty = false;
                    StatusText = "Document saved successfully";
                }
                else
                {
                    MessageBox.Show($"Failed to save document: {result.GetError().GetMessage()}");
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error saving document: {ex.Message}");
            }
        }

        private void ShowShareDialog()
        {
            var dialog = new ShareDocumentDialog(_documentService, _documentId);
            dialog.Owner = this;
            dialog.ShowDialog();
        }

        private void ShowVersionHistory()
        {
            var dialog = new VersionHistoryDialog(_documentService, _documentId);
            dialog.Owner = this;
            dialog.ShowDialog();
        }

        private void ApplyBold()
        {
            if (DocumentContent.Selection.IsEmpty) return;
            var selection = DocumentContent.Selection;
            TextRange range = new TextRange(selection.Start, selection.End);
            range.ApplyPropertyValue(TextElement.FontWeightProperty, 
                range.GetPropertyValue(TextElement.FontWeightProperty) == FontWeights.Bold
                    ? FontWeights.Normal
                    : FontWeights.Bold);
        }

        private void ApplyItalic()
        {
            if (DocumentContent.Selection.IsEmpty) return;
            var selection = DocumentContent.Selection;
            TextRange range = new TextRange(selection.Start, selection.End);
            range.ApplyPropertyValue(TextElement.FontStyleProperty,
                range.GetPropertyValue(TextElement.FontStyleProperty) == FontStyles.Italic
                    ? FontStyles.Normal
                    : FontStyles.Italic);
        }

        private void ApplyUnderline()
        {
            if (DocumentContent.Selection.IsEmpty) return;
            var selection = DocumentContent.Selection;
            TextRange range = new TextRange(selection.Start, selection.End);
            TextDecorationCollection decorations = 
                (TextDecorationCollection)range.GetPropertyValue(Inline.TextDecorationsProperty);
            range.ApplyPropertyValue(Inline.TextDecorationsProperty,
                decorations != null && decorations.Contains(TextDecorations.Underline[0])
                    ? null
                    : TextDecorations.Underline);
        }

        private void DocumentContent_TextChanged(object sender, TextChangedEventArgs e)
        {
            if (!_isDirty)
            {
                _isDirty = true;
                StatusText = "Document has unsaved changes";
            }
        }

        private void AutoSave_Tick(object sender, EventArgs e)
        {
            if (_isDirty)
            {
                SaveDocument();
            }
        }

        private FlowDocument ConvertToFlowDocument(byte[] content)
        {
            // Implementation depends on document format
            // For now, just convert from plain text
            var text = System.Text.Encoding.UTF8.GetString(content);
            return new FlowDocument(new Paragraph(new Run(text)));
        }

        private byte[] ConvertFromFlowDocument(FlowDocument document)
        {
            // Implementation depends on document format
            // For now, just convert to plain text
            var range = new TextRange(document.ContentStart, document.ContentEnd);
            var text = range.Text;
            return System.Text.Encoding.UTF8.GetBytes(text);
        }

        public void OnDocumentChanged(string documentId, byte[] diff)
        {
            if (documentId != _documentId) return;

            Dispatcher.Invoke(() =>
            {
                try
                {
                    // Apply diff to document content
                    // Implementation depends on diff format
                    DocumentContent = ConvertToFlowDocument(diff);
                    StatusText = "Document updated by another user";
                }
                catch (Exception ex)
                {
                    MessageBox.Show($"Error applying document changes: {ex.Message}");
                }
            });
        }

        public void OnPermissionChanged(string documentId, string userId, DocumentPermission permission)
        {
            if (documentId != _documentId) return;

            Dispatcher.Invoke(() =>
            {
                if (userId == CurrentUser.Id)
                {
                    IsReadOnly = permission < DocumentPermission.Write;
                    StatusText = $"Your permissions have been updated to: {permission}";
                }
            });
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            base.OnClosing(e);

            _autoSaveTimer.Stop();
            _documentService.UnregisterCallback(this);
            _documentService.CloseDocument(_documentId);

            if (_isDirty)
            {
                var result = MessageBox.Show("Do you want to save changes before closing?",
                    "Save Changes",
                    MessageBoxButton.YesNoCancel);

                switch (result)
                {
                    case MessageBoxResult.Yes:
                        SaveDocument();
                        break;
                    case MessageBoxResult.Cancel:
                        e.Cancel = true;
                        break;
                }
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class OnlineUser
    {
        public string Name { get; set; }
        public string Color { get; set; }
    }
}
