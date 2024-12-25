using System;
using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Windows.Input;

namespace SmartAssistant.UI.ViewModels
{
    public class MainViewModel : INotifyPropertyChanged
    {
        private bool _isTopMost = true;
        private double _opacity = 0.9;
        
        public bool IsTopMost
        {
            get => _isTopMost;
            set
            {
                _isTopMost = value;
                OnPropertyChanged();
            }
        }
        
        public double Opacity
        {
            get => _opacity;
            set
            {
                _opacity = value;
                OnPropertyChanged();
            }
        }

        public event PropertyChangedEventHandler PropertyChanged;

        protected virtual void OnPropertyChanged([CallerMemberName] string propertyName = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        }
    }
}
