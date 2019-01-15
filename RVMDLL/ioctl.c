#include <stdio.h>
#include "ioctl.h"

PCWSTR const RvmDriverPath = L"\\\\.\\rvm";

HRESULT
IoctlOpen(
	__in PCWSTR Path,
	__in DWORD Access,
	__in DWORD Share,
	__out HANDLE *Handle
)

/*++

Routine Description:

...

Arguments:

...

Return Value:

...

--*/

{
	HRESULT Result;

	(*Handle) =
		CreateFileW(
			Path,
			Access,
			Share,
			NULL,
			OPEN_EXISTING,
			0,
			NULL);

	if ((*Handle) == INVALID_HANDLE_VALUE)
	{
		Result = HRESULT_FROM_WIN32(GetLastError());

		if (Result == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
			wprintf(L"RVM driver could not be located."
					L"Make sure it is installed and started.\n");

		return ERROR_FILE_NOT_FOUND;
	}

	return S_OK;
}

HRESULT
SendIoctl(
	__inout PRVM_IOCTL_BUFFER Buffer,
	__in ULONG InputBufferSize,
	__inout PULONG OutputBufferSize,
	__in DWORD IoctlCode,
	__in DWORD Access,
	__in DWORD Share
)

{
	HANDLE Handle = INVALID_HANDLE_VALUE;
	HRESULT Result = S_OK;

	Result = IoctlOpen(RvmDriverPath, Access, Share, &Handle);

	if (Result == S_OK) {
		DeviceIoControl(Handle,
						IoctlCode,
						Buffer,
						InputBufferSize,
						Buffer,
						*OutputBufferSize,
						OutputBufferSize,
						NULL);
	} else {
		return Result;
	}

	CloseHandle(Handle);
	return HRESULT_FROM_WIN32(GetLastError());
}