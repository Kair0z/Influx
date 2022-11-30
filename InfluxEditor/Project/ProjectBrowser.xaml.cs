using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace InfluxEditor.Project
{
    /// <summary>
    /// Interaction logic for ProjectBrowser.xaml
    /// </summary>
    public partial class ProjectBrowser : Window
    {
        public ProjectBrowser()
        {
            InitializeComponent();
        }

        private void OnToggleButtonClick(object sender, RoutedEventArgs e)
        {
            if (sender == _openProjectButton)
            {
                if (_createProjectButton.IsChecked == true)
                {
                    _createProjectButton.IsChecked = false;
                    _browserContent.Margin = new Thickness(0);
                }

                _openProjectButton.IsChecked = true;
            }
            else if (sender == _createProjectButton)
            {
                if (_openProjectButton.IsChecked == true)
                {
                    _openProjectButton.IsChecked = false;
                    _browserContent.Margin = new Thickness(-800, 0, 0, 0);
                }

                _createProjectButton.IsChecked = true;
            }
        }
    }
}
