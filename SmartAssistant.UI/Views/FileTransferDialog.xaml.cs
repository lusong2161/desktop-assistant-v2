using System.Windows;

namespace SmartAssistant.UI.Views
{
    public partial class FileTransferDialog : Window
    {
        public FileTransferDialog()
        {
            InitializeComponent();
            DataContext = new ViewModels.FileTransferViewModel();
        }
    }
}
