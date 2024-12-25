using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Windows.Input;

namespace SmartAssistant.UI.ViewModels
{
    public class DocumentViewModel : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public ObservableCollection<DocumentItem> Documents { get; }
        public ICommand OpenDocumentCommand { get; }
        public ICommand ShareDocumentCommand { get; }
        public ICommand ViewHistoryCommand { get; }

        public DocumentViewModel()
        {
            Documents = new ObservableCollection<DocumentItem>();
            OpenDocumentCommand = new RelayCommand(OpenDocument);
            ShareDocumentCommand = new RelayCommand(ShareDocument);
            ViewHistoryCommand = new RelayCommand(ViewHistory);
        }

        private void OpenDocument()
        {
            // TODO: 实现打开文档逻辑
        }

        private void ShareDocument()
        {
            // TODO: 实现共享文档逻辑
        }

        private void ViewHistory()
        {
            // TODO: 实现查看历史记录逻辑
        }
    }

    public class DocumentItem : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        private string _name;
        public string Name
        {
            get => _name;
            set
            {
                _name = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(Name)));
            }
        }

        private DateTime _lastModified;
        public DateTime LastModified
        {
            get => _lastModified;
            set
            {
                _lastModified = value;
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(LastModified)));
            }
        }

        public ICommand EditCommand { get; }
        public ICommand PreviewCommand { get; }

        public DocumentItem()
        {
            EditCommand = new RelayCommand(Edit);
            PreviewCommand = new RelayCommand(Preview);
        }

        private void Edit()
        {
            // TODO: 实现编辑文档逻辑
        }

        private void Preview()
        {
            // TODO: 实现预览文档逻辑
        }
    }
}
