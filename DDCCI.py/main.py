from ctypes import *
from Dxva2 import *

class MonitorList:
    
    """
		THE CLASS AUTOMATICALLY searches every detectable screen in the system when
		"hdc" and "lprcClip" are assigned as 0 or None. The callback function "MonitorEnum()"
		is called with current structure as its argument on each monitor screen enumerated
		by the "EnumDisplayMonitors()" function.

		BEWARE, THE FUNCTION does not enumerates "physical" monitor! Meaning, the function
		cannot distinguish the monitors that duplicates/mirrors screen from other monitors.
	"""

    def __init__(self):
        self.rcHMONITOR                  = []
        self.rcArrayOfMonitorRects       = []
        self.rcNumberOfPhysicalMonitors  = []
        self.rcArrayOfPhysicalMonitors   = []
        User32.EnumDisplayMonitors(0, None, User32.MonitorEnumProc(self.MonitorEnum), 0)

    """
		THE PARAMETERS OF the callback function are automatically assigned as follows.
		* hMon			: handle to the monitor.
		* hdc			: None when the parameter of the same name from the "EnumDisplayMonitor()" is assigned None.
		* lprcMonitor	: pointer to the RECT that specifies the coordinates of the monitor screen.
		* pData			: argument passed from the parameter of the same name from the "EnumDisplayMonitor()".
		
		This callback function is designed to store every HMONITOR within the system.
    """

    def MonitorEnum(self, hMon, hdc, lprcMonitor, pData):
        self.rcHMONITOR.append(hMon)
        rcMonitor = lprcMonitor.contents
        self.rcArrayOfMonitorRects.append({"left": rcMonitor.left, "top": rcMonitor.top, "right": rcMonitor.right, "bottom": rcMonitor.bottom})

def main():

	#==============================================================
	# GET PHYSICAL MONITORS TO ACQUIRE DISPLAY DATA CHANNEL (DDC)
	#==============================================================
    msgCaption = "DDCCI.py"

	# DECLARATION automatically initiate HMONITOR sequence.
    monitors = MonitorList()
    
    """
		FIRST, GET the number of physical monitors associated to the HMONITOR. The number of the associated physical monitor
		is passed to the "MonitorList.rcNumberOfPhysicalMonitors" sequence corresponding to the HMONITOR order.
	"""
    for index in range(len(monitors.rcHMONITOR)):
        monitors.rcNumberOfPhysicalMonitors.append(DWORD())

        if not Dxva2.GetNumberOfPhysicalMonitorsFromHMONITOR(monitors.rcHMONITOR[index], pointer(monitors.rcNumberOfPhysicalMonitors[index])):
            MessageBoxW(0, "Unable to retreive the number of physical monitors from the HMONITOR.", msgCaption, MB_ICONERROR)
            return GetLastError()
        else:
            """
				SECOND, ACQUIRE the array of physical monitor information associated to the HMONITOR.
				Each element of the "MonitorList.rcNumberOfPhysicalMonitors" sequence acts as an array of the
				corresponding HMONITOR containing the "MonitorList.rcNumberOfPhysicalMonitors" number of physical
				monitor information.
			"""
            monitors.rcArrayOfPhysicalMonitors.append(list())
            for nMonitor in range(monitors.rcNumberOfPhysicalMonitors[index].value):
                monitors.rcArrayOfPhysicalMonitors[index].append(PHYSICAL_MONITOR())

                if not Dxva2.GetPhysicalMonitorsFromHMONITOR(
                    monitors.rcHMONITOR[index], 
                    monitors.rcNumberOfPhysicalMonitors[index], 
                    pointer(monitors.rcArrayOfPhysicalMonitors[index][nMonitor])):
                    MessageBoxW(0, "Unable to retreive the information of physical monitors from the HMONITOR.", msgCaption, MB_ICONERROR)
                    return GetLastError()
    
    #==============================================================
	# GET PHYSICAL MONITORS TO ACQUIRE DISPLAY DATA CHANNEL (DDC)
	#==============================================================
    for index in range(len(monitors.rcHMONITOR)):
        for nMonitor in range(monitors.rcNumberOfPhysicalMonitors[index].value):
            print("[{0}] {1} x {2}\t({3}, {4})".format(
                0 if monitors.rcArrayOfPhysicalMonitors[index][nMonitor].hPhysicalMonitor == None else monitors.rcArrayOfPhysicalMonitors[index][nMonitor].hPhysicalMonitor, 
                monitors.rcArrayOfMonitorRects[index]["right"] - monitors.rcArrayOfMonitorRects[index]["left"],
                monitors.rcArrayOfMonitorRects[index]["bottom"] - monitors.rcArrayOfMonitorRects[index]["top"],
                monitors.rcArrayOfMonitorRects[index]["left"], monitors.rcArrayOfMonitorRects[index]["top"]
                ))
    print("--------------------------------")

    """
        SELECT THE MONITOR to communicate DDC/CI.
        Beware, this may not be available for a television which is kinda bit different from a monitor.
    """
    hMonitor = None
    while(True):
        nSelect = int(input("Select the monitor to communicate DDC/CI: "))
        for index in range(len(monitors.rcHMONITOR)):
            for nMonitor in range((monitors.rcNumberOfPhysicalMonitors[index].value)):
                if monitors.rcArrayOfPhysicalMonitors[index][nMonitor].hPhysicalMonitor == (None if nSelect == 0 else nSelect):
                    hMonitor = HANDLE(monitors.rcArrayOfPhysicalMonitors[index][nMonitor].hPhysicalMonitor)
                    break
            if hMonitor != None: break
        if hMonitor != None: break

    while(True):
        print("\n 1. GET\n 2. SET\n 0. EXIT")
        nSelect = int(input("Select the configuration option: "))

        if nSelect != CMD_EXIT and nSelect != CMD_GET and nSelect != CMD_SET: continue
        elif nSelect == CMD_EXIT: break
        
        """
            CONVERTS TWO-DIGIT hexadecimal string vcp code to numerical BYTE which ranges from 0 ~ 255.
            The format of the hexadecimal must be "0x00", otherwise cannot be recognized by the program.
        """
        vcp = input(" - Enter the VCP code for DDC/CI: ")
        if (vcp[0:2] == "0x" or vcp [0:2] == "0X") and len(vcp) == 4:
            opcode = int(vcp[2:], 16)
        else:
            print(" [INFO] Please enter the hexadecimal in \"0x00\" format!")
            continue

        """
            DEPENDING ON THE selection, either GET or SET monitor configuration such as Dim.Brightness
            using universal "0x10" VCP code. The input and output values from the DDC/CI are decimal.
        """
        if nSelect == CMD_GET:
            currentValue = DWORD()
            maximumValue = DWORD()
            if not Dxva2.GetVCPFeatureAndVCPFeatureReply(hMonitor, opcode, 0, pointer(currentValue), pointer(maximumValue)):
                print(" [INFO] Failed to get the configuration from the monitor with DDC/CI!")
            else:
                print(f" >> {currentValue.value} [MAX.{maximumValue.value}]")
        else:
            setValue = DWORD(int(input(" - Enter the value for the VCP code: ")))
            if not Dxva2.SetVCPFeature(hMonitor, opcode, setValue):
                print(" [INFO] Failed to set the configuration from the monitor with DDC/CI!")
            else:
                print(f" >> {vcp} set to {setValue}")


if __name__ == "__main__":
    main()
    input("\nPress Enter to exit.")
