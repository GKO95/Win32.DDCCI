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
struct MonitorList {

	std::vector<HMONITOR>			rcHMONITOR;					// Container for the monitor handle.
	std::vector<DWORD>				rcNumberOfPhysicalMonitors;	// Container for the number of physical monitor associated to each HMONITOR.
	std::vector<LPPHYSICAL_MONITOR>	rcArrayOfPhysicalMonitors;	// Container for the structure data array of the physical monitors for each HMONITOR.

	/*
		CONSTRUCTOR AUTOMATICALLY searches every detectable screen in the system when
		"hdc" and "lprcClip" are assigned as NULL. The callback function "MonitorEnum()"
		is called with current structure as its argument on each monitor screen enumerated
		by the "EnumDisplayMonitors()" function.

		BEWARE, THE FUNCTION does not enumerates "physical" monitor! Meaning, the function
		cannot distinguish the monitors that duplicates/mirrors screen from other monitors.
	*/
	MonitorList()
	{
		EnumDisplayMonitors(NULL, NULL, MonitorEnum, (LPARAM)this);
	}

	/*
		THE PARAMETERS OF the callback function are automatically assigned as follows.
		* hMon			: handle to the monitor.
		* hdc			: NULL when the parameter of the same name from the "EnumDisplayMonitor()" is assigned NULL.
		* lprcMonitor	: pointer to the RECT that specifies the coordinates of the monitor screen.
		* pData			: argument passed from the parameter of the same name from the "EnumDisplayMonitor()".
		
		This callback function is designed to store every HMONITOR within the system.
	*/
	static BOOL CALLBACK MonitorEnum(HMONITOR hMon, HDC hdc, LPRECT lprcMonitor, LPARAM pData)
	{
		MonitorList* pThis = reinterpret_cast<MonitorList*>(pData);
		pThis->rcHMONITOR.push_back(hMon);
		/* ALTERNATIVE:
		pThis->rcHMONITOR.push_back(MonitorFromRect(lprcMonitor, MONITOR_DEFAULTTONULL));
		*/
		return TRUE;
	}

	/*
		DESTRUCTOR AUTOMATICALLY destroys/releases the physical monitor handles,
		and deallocate the heap memory for PHYSICAL_MONITOR from "MonitorList.rcArrayOfPhysicalMonitors" vector.
	*/
	~MonitorList()
	{
		for (int index = 0; index < rcHMONITOR.size(); index++)
		{
			DestroyPhysicalMonitors(rcNumberOfPhysicalMonitors[index], rcArrayOfPhysicalMonitors[index]);
			free(rcArrayOfPhysicalMonitors[index]);
		}
	}
};

int main() {

	//==============================================================
	// GET PHYSICAL MONITORS TO ACQUIRE DISPLAY DATA CHANNEL (DDC)
	//==============================================================
	const LPCWSTR msgCaption = L"DDCCI.cpp";

	// DECLARATION automatically initiate HMONITOR collection.
	MonitorList monitors;

	/*
		FIRST, GET the number of physical monitors associated to the HMONITOR. The number of the associated physical monitor
		is passed to the "MonitorList.rcNumberOfPhysicalMonitors" vector corresponding to the HMONITOR order.
	*/
	for (int index = 0; index < monitors.rcHMONITOR.size(); index++)
	{
		monitors.rcNumberOfPhysicalMonitors.push_back(0);
		if (!GetNumberOfPhysicalMonitorsFromHMONITOR(monitors.rcHMONITOR[index], &monitors.rcNumberOfPhysicalMonitors[index]))
		{
			MessageBoxW(NULL, L"Unable to retreive the number of physical monitors from the HMONITOR.", msgCaption, MB_ICONERROR);
			return GetLastError();
		}
		else
		{
			/*
				SECOND, ACQUIRE the array of physical monitor information associated to the HMONITOR.
				Each element of the "MonitorList.rcNumberOfPhysicalMonitors" vector acts as an array of the
				corresponding HMONITOR containing the "MonitorList.rcNumberOfPhysicalMonitors" number of physical
				monitor information.
			*/
			monitors.rcArrayOfPhysicalMonitors.push_back(
				(LPPHYSICAL_MONITOR)malloc(monitors.rcNumberOfPhysicalMonitors[index] * sizeof(PHYSICAL_MONITOR)));
			if (!GetPhysicalMonitorsFromHMONITOR(monitors.rcHMONITOR[index], monitors.rcNumberOfPhysicalMonitors[index], monitors.rcArrayOfPhysicalMonitors[index]))
			{
				MessageBoxW(NULL, L"Unable to retreive the information of physical monitors from the HMONITOR.", msgCaption, MB_ICONERROR);
				return GetLastError();
			}
		}
	}

	//==============================================================
	// MANIPULATE THE MONITOR VIA DDC/CI
	//==============================================================

	return 0;
}