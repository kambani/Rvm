#include <stdio.h>
#include "rvmapi.h"
#include "ioctl.h"

VOID
RvmAppWorkingSetCreateDispatch(__in PWCHAR VolumeLetter)

/*++

Routine Description:

	User mode call for creating working set.

Arguments:

	VolumeLetter - Volume Letter. E.g. A: (Null terminated)

Return Value:

	Returns the Status of the operation.

--*/

{
	RVM_IOCTL_BUFFER Ioctl;
	HRESULT Result;
	ULONG Size;

	if (VolumeLetter == NULL || wcslen(VolumeLetter) != 2) {
		return;
	}

	Ioctl.Operation = RvmOpWorkingSetCreate;
	Size = RTL_SIZEOF_THROUGH_FIELD(RVM_IOCTL_BUFFER, WorkingSetCreate.VolumeName);

	//
	// Convert Drive letter to NT style
	//
	swprintf(&Ioctl.WorkingSetCreate.VolumeName[0], 
			 sizeof(Ioctl.WorkingSetCreate.VolumeName)/sizeof(WCHAR), 
			 L"%ls%ls",L"\\??\\",
			 VolumeLetter);

	//
	// Fire the IOCTL
	//
	Result = SendIoctl(&Ioctl,
					   Size,
					   &Size,
					   IOCTL_RVM_WRITE,
					   GENERIC_READ | GENERIC_WRITE,
					   FILE_SHARE_READ);
}