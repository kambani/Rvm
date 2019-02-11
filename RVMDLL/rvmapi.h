///
/// @file rvmapi.h
///
/// API to use RVM functionality.
///
/// Author: Kaushal Ambani (2018)
///

#pragma once

#define WIN32_LEAN_AND_MEAN  
#include <windows.h>
#include <winioctl.h>
#include "rvminterface.h"

VOID
RvmAppWorkingSetCreateDispatch(__in PWCHAR VolumeLetter);
