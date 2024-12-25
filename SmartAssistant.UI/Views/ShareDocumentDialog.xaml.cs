using System;
using System.Windows;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Collections.ObjectModel;
using SmartAssistant.Core;

namespace SmartAssistant.UI.Views
{
    public partial class ShareDocumentDialog : Window, INotifyPropertyChanged
    {
        private readonly DocumentService _documentService;
        private readonly string _documentId;
        private string _userId;
        private DocumentPermission _selectedPermission;

        public ShareDocumentDialog(DocumentService documentService, string documentId)
        {
            InitializeComponent();
            DataContext = this;

            _documentService = documentService;
            _documentId = documentId;
            SharedUsers = new ObservableCollection<SharedUser>();
            Permissions = new ObservableCollection<DocumentPermission>
            {
                DocumentPermission.Read,
                DocumentPermission.Write,
                DocumentPermission.Comment,
                DocumentPermission.Share
            };
            SelectedPermission = DocumentPermission.Read;

            ShareCommand = new RelayCommand(ShareWithUser, CanShare);
            CloseCommand = new RelayCommand(Close);

            LoadSharedUsers();
        }

        public string UserId
        {
            get => _userId;
            set
            {
                if (_userId != value)
                {
                    _userId = value;
                    OnPropertyChanged();
                    (ShareCommand as RelayCommand)?.RaiseCanExecuteChanged();
                }
            }
        }

        public DocumentPermission SelectedPermission
        {
            get => _selectedPermission;
            set
            {
                if (_selectedPermission != value)
                {
                    _selectedPermission = value;
                    OnPropertyChanged();
                }
            }
        }

        public ObservableCollection<DocumentPermission> Permissions { get; }
        public ObservableCollection<SharedUser> SharedUsers { get; }

        public RelayCommand ShareCommand { get; }
        public RelayCommand CloseCommand { get; }

        private async void LoadSharedUsers()
        {
            try
            {
                var result = await Task.Run(() => _documentService.GetMetadata(_documentId));
                if (result.IsSuccess)
                {
                    var metadata = result.GetValue();
                    SharedUsers.Clear();
                    foreach (var perm in metadata.userPermissions)
                    {
                        if (perm.Key != CurrentUser.Id)
                        {
                            SharedUsers.Add(new SharedUser
                            {
                                UserId = perm.Key,
                                Permission = perm.Value
                            });
                        }
                    }
                }
                else
                {
                    MessageBox.Show($"Failed to load shared users: {result.GetError().GetMessage()}");
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error loading shared users: {ex.Message}");
            }
        }

        private bool CanShare()
        {
            return !string.IsNullOrWhiteSpace(UserId);
        }

        private async void ShareWithUser()
        {
            try
            {
                var result = await Task.Run(() => 
                    _documentService.ShareDocument(_documentId, UserId, SelectedPermission));
                
                if (result.IsSuccess)
                {
                    SharedUsers.Add(new SharedUser
                    {
                        UserId = UserId,
                        Permission = SelectedPermission
                    });
                    UserId = string.Empty;
                    SelectedPermission = DocumentPermission.Read;
                }
                else
                {
                    MessageBox.Show($"Failed to share document: {result.GetError().GetMessage()}");
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show($"Error sharing document: {ex.Message}");
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }

    public class SharedUser
    {
        public string UserId { get; set; }
        public DocumentPermission Permission { get; set; }
    }
}
