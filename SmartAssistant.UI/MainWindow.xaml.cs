using System;
using System.Windows;
using System.Windows.Input;
using SmartAssistant.Core;

namespace SmartAssistant.UI
{
    public partial class MainWindow : Window
    {
        private bool _isProcessingCommand;

        public MainWindow()
        {
            InitializeComponent();
            
            // Register global hotkey (Alt+Space)
            CoreAPI.RegisterGlobalHotkey(1, (int)ModifierKeys.Alt, 0x20);
        }

        private void TitleBar_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (e.LeftButton == MouseButtonState.Pressed)
            {
                DragMove();
            }
        }

        private void MinimizeButton_Click(object sender, RoutedEventArgs e)
        {
            WindowState = WindowState.Minimized;
        }

        private void CloseButton_Click(object sender, RoutedEventArgs e)
        {
            CoreAPI.UnregisterGlobalHotkey(1);
            Application.Current.Shutdown();
        }

        private async void CommandInput_KeyDown(object sender, KeyEventArgs e)
        {
            if (e.Key == Key.Enter && !_isProcessingCommand)
            {
                _isProcessingCommand = true;
                try
                {
                    string command = CommandInput.Text;
                    string response = string.Empty;
                    
                    if (CoreAPI.ProcessCommand(command, out response))
                    {
                        ResponseText.Text = response;
                    }
                    else
                    {
                        ResponseText.Text = "无法处理命令，请重试。";
                    }
                    
                    CommandInput.Clear();
                }
                finally
                {
                    _isProcessingCommand = false;
                }
            }
        }

        protected override void OnSourceInitialized(EventArgs e)
        {
            base.OnSourceInitialized(e);
            
            // Set window position to bottom right
            var workArea = SystemParameters.WorkArea;
            Left = workArea.Right - Width - 20;
            Top = workArea.Bottom - Height - 20;
        }
    }
}
