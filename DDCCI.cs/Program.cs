using System;
using System.Reflection;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace DDCCI.cs
{
    using DWORD = UInt32;
    using HANDLE = IntPtr;
    using HMONITOR = IntPtr;

    class Program
    {
        [DllImport("Dxva2.dll", CharSet = CharSet.Ansi, EntryPoint = "GetNumberOfPhysicalMonitorsFromHMONITOR")]
        private static extern bool GetNumberOfPhysicalMonitorsFromHMONITOR(HMONITOR hMonitor, ref DWORD pdwNumberOfPhysicalMonitors);

        [DllImport("Dxva2.dll", CharSet = CharSet.Ansi, EntryPoint = "GetPhysicalMonitorsFromHMONITOR")]
        private static extern bool GetPhysicalMonitorsFromHMONITOR(HMONITOR hMonitor, DWORD dwPhysicalMonitorArraySize, ref PHYSICAL_MONITOR pPhysicalMonitorArray);

        [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Unicode)]
        struct PHYSICAL_MONITOR
        {
            public HANDLE hPhysicalMonitor;
            [MarshalAs(UnmanagedType.ByValTStr, SizeConst = 256)] public string szPhysicalMonitorDescription;
        }

        readonly static List<HMONITOR> rcMONITOR = new List<HMONITOR>();
        readonly static List<DWORD> rcNumberOfPhysicalMonitors = new List<DWORD>();
        readonly static List<PHYSICAL_MONITOR[]> rcArrayOfPhysicalMonitors = new List<PHYSICAL_MONITOR[]>();

        static void Main(string[] args)
        {
            FieldInfo hmonitorField = typeof(Screen).GetField("hmonitor", BindingFlags.NonPublic | BindingFlags.GetField | BindingFlags.Instance);
            foreach (Screen screen in Screen.AllScreens)
                rcMONITOR.Add((HMONITOR)hmonitorField.GetValue(screen));

            foreach (HMONITOR hMonitor in rcMONITOR)
            {
                DWORD _rcNumberOfPhysicalMonitors = 0;
                GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, ref _rcNumberOfPhysicalMonitors);
                rcNumberOfPhysicalMonitors.Add(_rcNumberOfPhysicalMonitors);
            }

            for (int index = 0; index < rcMONITOR.Count; index++)
            {
                PHYSICAL_MONITOR[] _rcArrayOfPhysicalMonitors = new PHYSICAL_MONITOR[rcNumberOfPhysicalMonitors[index]];
                GetPhysicalMonitorsFromHMONITOR(rcMONITOR[index], rcNumberOfPhysicalMonitors[index], ref _rcArrayOfPhysicalMonitors[0]);
                rcArrayOfPhysicalMonitors.Add(_rcArrayOfPhysicalMonitors);
            }



        }
    }
}
