#pragma once

#define WIN32_LEAN_AND_MEAN  
#include <windows.h>
#include <winioctl.h>
#include "rvminterface.h"

#define IOCTL_RVM_READ                \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x8001, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_RVM_WRITE               \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x8001, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)

VOID
RvmAppWorkingSetCreateDispatch(__in PWCHAR VolumeLetter);
