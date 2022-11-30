using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace InfluxEditor.Project
{
    class CreateProject : ViewModelBase
    {
        private string _name = "NewProject";
        public string Name { get => _name; set { if (_name != value) { _name = value; OnPropertyChanged(nameof(Name)); } } }
    }
}
