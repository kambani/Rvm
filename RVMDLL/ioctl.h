#pragma once
#include <windows.h>
#include "rvminterface.h"

HRESULT
SendIoctl(
	__inout PRVM_IOCTL_BUFFER Buffer,
	__in ULONG InputBufferSize,
	__inout PULONG OutputBufferSize,
	__in DWORD IoctlCode,
	__in DWORD Access,
	__in DWORD Share
);