#include <iostream>
#include <vector>

/* Essential headers for "SetupAPI.lib"; do not forget to include library to the project! */
#include <Windows.h>

/* Essential headers for DDC/CI using "Dxva2.lib"; do not forget to include library to the project! */
#include <physicalmonitorenumerationapi.h>
#include <lowlevelmonitorconfigurationapi.h>

//==============================================================
// ENUMERATION FOR COMMAND OPTION SELECTION
//==============================================================
enum class cmdInterface : BYTE {
	EXIT,
	GET,
	SET,
	MAX_COUNT,
};

//==============================================================
// STRUCTURE THAT STORES THE MONITOR INFORMATION
//==============================================================
struct MonitorList {

	std::vector<HMONITOR>			rcHMONITOR;					// Container for the monitor handle.
	std::vector<RECT>				rcArrayOfMonitorRects;		// Container for the RECT data array of the physical monitors for each HMONITOR.
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
		pThis->rcArrayOfMonitorRects.push_back(*lprcMonitor);
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
	for (int index = 0; index < monitors.rcHMONITOR.size(); index++)
	{
		for (int nMonitor = 0; nMonitor < monitors.rcNumberOfPhysicalMonitors[index]; nMonitor++)
		{
			std::cout << "[" << (int)monitors.rcArrayOfPhysicalMonitors[index]->hPhysicalMonitor << "] " 
				<< monitors.rcArrayOfMonitorRects[index].right - monitors.rcArrayOfMonitorRects[index].left << " x " 
				<< monitors.rcArrayOfMonitorRects[index].bottom - monitors.rcArrayOfMonitorRects[index].top 
				<< "\t(" << monitors.rcArrayOfMonitorRects[index].left << ", " << monitors.rcArrayOfMonitorRects[index].top << ")" << std::endl;
		}
	} std::cout << "--------------------------------" << std::endl;


	/*
		SELECT THE MONITOR to communicate DDC/CI.
		Beware, this may not be available for a television which is kinda bit different from a monitor.
	*/
	int nSelect;
	HANDLE hMonitor = INVALID_HANDLE_VALUE;
	while (true)
	{
		std::cout << "Select the monitor to communicate DDC/CI: ";
		std::cin >> nSelect;
		for (int index = 0; index < monitors.rcHMONITOR.size(); index++)
		{
			for (int nMonitor = 0; nMonitor < monitors.rcNumberOfPhysicalMonitors[index]; nMonitor++)
			{
				if ((int)monitors.rcArrayOfPhysicalMonitors[index]->hPhysicalMonitor == nSelect)
				{
					hMonitor = monitors.rcArrayOfPhysicalMonitors[index]->hPhysicalMonitor;
					break;
				}
			}
			if (hMonitor != INVALID_HANDLE_VALUE) break;
		}
		if (hMonitor != INVALID_HANDLE_VALUE) break;
	}
	
	cmdInterface cmdSelect = cmdInterface::EXIT;
	while (true)
	{
		std::cout << std::endl << " 1. GET" << std::endl << " 2. SET" << std::endl << " 0. EXIT" << std::endl << "Select the configuration option: ";
		std::cin >> nSelect; cmdSelect = cmdInterface(nSelect);

		if (static_cast<int>(cmdSelect) >= static_cast<int>(cmdInterface::MAX_COUNT)) continue;
		else if (cmdSelect == cmdInterface::EXIT) break;

		/*
			CONVERTS TWO-DIGIT hexadecimal string vcp code to numerical BYTE which ranges from 0 ~ 255.
			The format of the hexadecimal must be "0x00", otherwise cannot be recognized by the program.
		*/
		std::string vpc; BYTE opcode;
		std::cout << " - Enter the VCP code for DDC/CI: ";
		std::cin >> vpc;
		if ((vpc.c_str()[1] == 'x' || vpc.c_str()[1] == 'X') && vpc.size() == 4)
		{
			opcode = std::strtoul(vpc.c_str(), nullptr, 16);
		}
		else
		{
			std::cout << " [INFO] Please enter the hexadecimal in \"0x00\" format!" << std::endl;
			continue;
		}

		/*
			DEPENDING ON THE selection, either GET or SET monitor configuration such as Dim.Brightness
			using universal "0x10" VCP code. The input and output values from the DDC/CI are decimal.
		*/
		if (cmdSelect == cmdInterface::GET)
		{
			DWORD currentValue, maximumValue;
			if (!GetVCPFeatureAndVCPFeatureReply(hMonitor, opcode, NULL, &currentValue, &maximumValue))
				std::cout << " [INFO] Failed to get the configuration from the monitor with DDC/CI!" << std::endl;
			else
				std::cout << " >> " << currentValue << " [MAX." << maximumValue << "]" << std::endl;
		}
		else
		{
			DWORD setValue;
			std::cout << " - Enter the value for the VCP code: ";
			std::cin >> setValue;
			if (!SetVCPFeature(hMonitor, opcode, setValue))
				std::cout << " [INFO] Failed to set the configuration from the monitor with DDC/CI!" << std::endl;
			else
				std::cout << " >> " << vpc << "set to " << setValue << std::endl;
		}
	}

	return 0;
}