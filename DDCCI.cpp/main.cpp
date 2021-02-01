#include <iostream>
#include <vector>

/* Essential headers for "SetupAPI.lib"; do not forget to include library to the project! */
#include <Windows.h>
#include <SetupAPI.h>

#include <physicalmonitorenumerationapi.h>
#include <lowlevelmonitorconfigurationapi.h>

//==============================================================
// STRUCTURE THAT STORES THE MONITOR INFORMATION
//==============================================================
struct MonitorRects {

	std::vector<HMONITOR>			rcHMONITOR;
	std::vector<DWORD>				rcNumberOfPhysicalMonitors;
	std::vector<LPPHYSICAL_MONITOR>	rcArrayOfPhysicalMonitors;

	/*
		Constructor automatically searches every detectable screen in the system when
		"hdc" and "lprcClip" are assigned as NULL. The callback function "MonitorEnum()"
		is called with current structure as its argument on each screen enumerated by the
		"EnumDisplayMonitors()" function.

		Beware, the function does not enumerates "physical" monitor! Meaning, the function
		cannot distinguish the monitors that duplicates/mirrors screen from other monitors.
		Microsoft Docs describes this as a "virtual-screen".
	*/
	MonitorRects()
	{
		EnumDisplayMonitors(NULL, NULL, MonitorEnum, (LPARAM)this);
	}

	/*
		The arguments for the callback function are designated as follows:
		* hMon			: handle to the virtual-screen.
		* hdc			: NULL when the parameter of the same name from the "EnumDisplayMonitor()" is assigned NULL.
		* lprcMonitor	: pointer to the RECT that specifies the coordinates of the virtual-screen.
		* pData			: identical argument to the parameter of the same name from the "EnumDisplayMonitor()".
	*/
	static BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
	{
		MonitorRects* pThis = reinterpret_cast<MonitorRects*>(pData);
		pThis->rcHMONITOR.push_back(hMon);
		/* ALTERNATIVE:
		pThis->rcHMONITOR.push_back(MonitorFromRect(lprcMonitor, MONITOR_DEFAULTTONULL));
		*/
		pThis->rcNumberOfPhysicalMonitors.push_back(1);
		pThis->rcArrayOfPhysicalMonitors.push_back(nullptr);
		return TRUE;
	}
};

int main() {
	
	MonitorRects monitors;
	LPPHYSICAL_MONITOR pPhysicalMonitors = NULL;

	return 0;
}