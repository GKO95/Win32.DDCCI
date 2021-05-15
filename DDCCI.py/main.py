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
    ...


if __name__ == "__main__":
    main()
    input("\nPress Enter to exit.")