using System.Windows;

namespace SmartAssistant.UI.Views
{
    public partial class DocumentWindow : Window
    {
        public DocumentWindow()
        {
            InitializeComponent();
            DataContext = new ViewModels.DocumentViewModel();
        }
    }
}
