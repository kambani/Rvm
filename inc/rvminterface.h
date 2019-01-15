#pragma once

typedef ULONG RVM_HANDLE;
#define MAX_NT_VOLUME_LENGTH 7

typedef enum _RVM_OPERATION {
	RvmOpWorkingSetCreate
} RVM_OPERATION, *PRVM_OPERATION;

typedef struct _RVM_IOCTL_WORKING_SET_CREATE {

	//
	// Volume Name in NT Path style. For e.g \??\O: 
	// [In]
	//

	WCHAR VolumeName[MAX_NT_VOLUME_LENGTH];

	//
	// Handle to the newly created 
	// working set
	//

	ULONG Handle;

} RVM_IOCTL_WORKING_SET_CREATE, *PRVM_IOCTL_WORKING_SET_CREATE;

typedef struct _RVM_IOCTL_BUFFER {
	RVM_OPERATION Operation;
	union {
		RVM_IOCTL_WORKING_SET_CREATE WorkingSetCreate;
	};
} RVM_IOCTL_BUFFER, *PRVM_IOCTL_BUFFER;