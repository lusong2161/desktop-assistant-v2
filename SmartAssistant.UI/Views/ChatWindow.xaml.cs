using System.Windows;

namespace SmartAssistant.UI.Views
{
    public partial class ChatWindow : Window
    {
        public ChatWindow()
        {
            InitializeComponent();
            DataContext = new ViewModels.ChatViewModel();
        }
    }
}
