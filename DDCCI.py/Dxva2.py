from ctypes import *
from ctypes.wintypes import *

class PHYSICAL_MONITOR(Structure):
    _fields_ = [
        ("hPhysicalMonitor", HANDLE),
        ("szPhysicalMonitorDescription", WCHAR * 128)
    ]


class Dxva2:
    @staticmethod
    def GetNumberOfPhysicalMonitorsFromHMONITOR(hMonitor, pdwNumberOfPhysicalMonitors):
        __function = windll.dxva2.GetNumberOfPhysicalMonitorsFromHMONITOR
        __function.argtypes = [HMONITOR, PDWORD]
        __function.restype = c_bool
        return __function(hMonitor, pdwNumberOfPhysicalMonitors)

    @staticmethod
    def GetPhysicalMonitorsFromHMONITOR(hMonitor, dwPhysicalMonitorArraySize, pPhysicalMonitorArray):
        __function = windll.dxva2.GetPhysicalMonitorsFromHMONITOR
        __function.argtypes = [HMONITOR, DWORD, POINTER(PHYSICAL_MONITOR)]
        __function.restype = c_bool
        return __function(hMonitor, dwPhysicalMonitorArraySize, pPhysicalMonitorArray)


class User32:
    @staticmethod
    def MonitorEnumProc(__function):
        return User32.MONITORENUMPROC(__function)

    @staticmethod
    def EnumDisplayMonitors(hdc, lprcClip, lpfnEnum, dwData):
        __function = windll.user32.EnumDisplayMonitors
        __function.argtypes = [HDC, LPRECT, User32.MONITORENUMPROC, LPARAM]
        __function.restype = c_bool
        return __function(hdc, lprcClip, lpfnEnum, dwData)

    MONITORENUMPROC = CFUNCTYPE(c_bool, HMONITOR, HDC, LPRECT, LPARAM)


def MessageBoxW(hWnd, lpText, lpCaption, uType):
	windll.user32.MessageBoxW(hWnd, lpText, lpCaption, uType)


MB_ICONERROR                = 0x00000010
MB_ICONQUESTION             = 0x00000020
MB_ICONWARNING              = 0x00000030