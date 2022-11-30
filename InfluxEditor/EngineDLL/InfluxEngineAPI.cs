using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Numerics;

using InfluxEditor.EngineAPIStructs;
namespace InfluxEditor.EngineAPIStructs
{
    [StructLayout(LayoutKind.Sequential)]
    class TransformComponent
    {
        public float test;
    }
}

namespace InfluxEditor.EngineDLL
{
    class InfluxEngineAPI
    {
        private const string _dllName = "InfluxEngineDLL.dll";

        [DllImport(_dllName)] private static extern int CreateTransformComponent_DLL();

        public static void CreateTransformComponent(TransformComponent transformComponent)
        {
            int a = CreateTransformComponent_DLL();
            

        }
    }
}
