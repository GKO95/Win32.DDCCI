using System;
using System.Reflection;
using System.Collections.Generic;
using System.Windows.Forms;

namespace DDCCI.Framework.cs
{
    using DWORD = UInt32;
    using HANDLE = IntPtr;
    using HMONITOR = IntPtr;

    //==============================================================
    // ENUMERATION FOR COMMAND OPTION SELECTION
    //==============================================================
    enum cmdInterface : byte
    {
        EXIT,
	    GET,
	    SET,
	    MAX_COUNT,
    }

    class Program
    {
        readonly static List<HMONITOR> rcHMONITOR = new List<HMONITOR>();
        readonly static List<DWORD> rcNumberOfPhysicalMonitors = new List<DWORD>();
        readonly static List<Dxva2.PHYSICAL_MONITOR[]> rcArrayOfPhysicalMonitors = new List<Dxva2.PHYSICAL_MONITOR[]>();

        static void Main(string[] args)
        {
            //==============================================================
            // GET PHYSICAL MONITORS TO ACQUIRE DISPLAY DATA CHANNEL (DDC)
            //==============================================================
            const string msgCaption = "DDCCI.Framework.cs";

            /*
                UNLIKE C++ PROJECT, Screen class from System.Windows.Forms has a private "hmonitor" field 
                that contains the HMONITOR exactly what this project needs. However, since the field is not
                accessible publicly, the code below get the field value indirectly from the instance.
            */
            FieldInfo hmonitorField = typeof(Screen).GetField("hmonitor", BindingFlags.NonPublic | BindingFlags.GetField | BindingFlags.Instance);
            foreach (Screen screen in Screen.AllScreens)
                rcHMONITOR.Add((HMONITOR)hmonitorField.GetValue(screen));

            /*
                FIRST, GET the number of physical monitors associated to the HMONITOR. The number of the associated physical monitor
                is passed to the "rcNumberOfPhysicalMonitors" list corresponding to the HMONITOR order.
            */
            foreach (HMONITOR hMon in rcHMONITOR)
            {
                DWORD _rcNumberOfPhysicalMonitors = 0;
                if (!Dxva2.GetNumberOfPhysicalMonitorsFromHMONITOR(hMon, ref _rcNumberOfPhysicalMonitors))
                {
                    MessageBox.Show("Unable to retrieve the number of physical monitors.", msgCaption, MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }
                rcNumberOfPhysicalMonitors.Add(_rcNumberOfPhysicalMonitors);
            }

            /*
                SECOND, ACQUIRE the array of physical monitor information associated to the HMONITOR.
                Each element of the "rcNumberOfPhysicalMonitors" list acts as an array of the
                corresponding HMONITOR containing the "rcNumberOfPhysicalMonitors" number of physical
                monitor information.
            */
            for (int index = 0; index < rcHMONITOR.Count; index++)
            {
                Dxva2.PHYSICAL_MONITOR[] _rcArrayOfPhysicalMonitors = new Dxva2.PHYSICAL_MONITOR[rcNumberOfPhysicalMonitors[index]];
                if (!Dxva2.GetPhysicalMonitorsFromHMONITOR(rcHMONITOR[index], rcNumberOfPhysicalMonitors[index], ref _rcArrayOfPhysicalMonitors[0]))
                {
                    MessageBox.Show("Unable to retrieve the physical monitors.", msgCaption, MessageBoxButtons.OK, MessageBoxIcon.Error);
                    return;
                }
                rcArrayOfPhysicalMonitors.Add(_rcArrayOfPhysicalMonitors);
            }

            //==============================================================
            // MANIPULATE THE MONITOR VIA DDC/CI
            //==============================================================
            for (int index = 0; index < rcHMONITOR.Count; index++)
            {
                for (int nMonitor = 0; nMonitor < rcNumberOfPhysicalMonitors[index]; nMonitor++)
                {
                    Console.WriteLine(string.Format("[{0}] {1} x {2}\t({3}, {4})",
                        (int)rcArrayOfPhysicalMonitors[index][nMonitor].hPhysicalMonitor, Screen.AllScreens[index].Bounds.Width, Screen.AllScreens[index].Bounds.Height,
                        Screen.AllScreens[index].Bounds.Left, Screen.AllScreens[index].Bounds.Top)
                    );
                }
            }
            Console.WriteLine("--------------------------------");

            /*
                SELECT THE MONITOR to communicate DDC/CI.
                Beware, this may not be available for a television which is kinda bit different from a monitor.
            */
            int nSelect;
            HMONITOR hMonitor = INVALID_HANDLE_VALUE;
            while (true)
            {
                Console.Write("Select the monitor to communicate DDC/CI: ");
                nSelect = Convert.ToInt32(Console.ReadLine());
                for (int index = 0; index < rcHMONITOR.Count; index++)
                {
                    for (int nMonitor = 0; nMonitor < rcNumberOfPhysicalMonitors[index]; nMonitor++)
                    {
                        if ((int)rcArrayOfPhysicalMonitors[index][nMonitor].hPhysicalMonitor == nSelect)
                        {
                            hMonitor = rcArrayOfPhysicalMonitors[index][nMonitor].hPhysicalMonitor;
                            break;
                        }
                    }
                    if (hMonitor != INVALID_HANDLE_VALUE) break;
                }
                if (hMonitor != INVALID_HANDLE_VALUE) break;
            }

            cmdInterface cmdSelect;
            while (true)
            {
                Console.Write("\n 1. GET\n 2. SET\n 0. EXIT\nSelect the configuration option: ");
                try { cmdSelect = (cmdInterface)Convert.ToInt32(Console.ReadLine()); }
                catch (Exception) { continue; }

                if ((int)cmdSelect >= (int)cmdInterface.MAX_COUNT) continue;
                else if (cmdSelect == cmdInterface.EXIT) break;

                /*
                    CONVERTS TWO-DIGIT hexadecimal string vcp code to numerical BYTE which ranges from 0 ~ 255.
                    The format of the hexadecimal must be "0x00", otherwise cannot be recognized by the program.
                */
                string vpc; byte opcode;
                Console.Write(" - Enter the VCP code for DDC/CI: ");
                vpc = Console.ReadLine();
                if ((vpc.Contains("0x") || vpc.Contains("0X")) && vpc.Length == 4)
                {
                    opcode = Convert.ToByte(vpc.Substring(2), 16);
                }
                else
                {
                    Console.WriteLine(" [INFO] Please enter the hexadecimal in \"0x00\" format!");
                    continue;
                }

                /*
                    DEPENDING ON THE selection, either GET or SET monitor configuration such as Dim.Brightness
                    using universal "0x10" VCP code. The input and output values from the DDC/CI are decimal.
                */
                if (cmdSelect == cmdInterface.GET)
                {
                    DWORD currentValue = 0, maximumValue = 0;
                    if (!Dxva2.GetVCPFeatureAndVCPFeatureReply(hMonitor, opcode, 0, ref currentValue, ref maximumValue))
                        Console.WriteLine(" [INFO] Failed to get the configuration from the monitor with DDC/CI!");
                    else
                        Console.WriteLine(string.Format(" >> {0} [MAX.{1}]", currentValue, maximumValue));
                }
                else
                {
                    Console.Write(" - Enter the value for the VCP code: ");
                    DWORD setValue = Convert.ToUInt32(Console.ReadLine());
                    if (!Dxva2.SetVCPFeature(hMonitor, opcode, setValue))
                        Console.WriteLine(" [INFO] Failed to set the configuration from the monitor with DDC/CI!");
                    else
                        Console.WriteLine(string.Format(" >> {0} set to {1}", vpc, setValue));
                }
            }
        }

        private static readonly HANDLE INVALID_HANDLE_VALUE = (HANDLE)(-1);
    }
}
