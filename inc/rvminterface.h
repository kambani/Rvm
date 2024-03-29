///
/// @file rvminterface.h
///
/// IOCTL Interface to talk to RVM Driver.
///
/// Author: Kaushal Ambani (2018)
///

#pragma once

typedef ULONG RVM_HANDLE;
typedef PULONG PRVM_HANDLE;

#define MAX_NT_VOLUME_LENGTH 7

#define IOCTL_RVM_READ                \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x8001, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_RVM_WRITE               \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x8001, METHOD_BUFFERED, FILE_READ_DATA | FILE_WRITE_DATA)

typedef enum _RVM_OPERATION {
	RvmOpWorkingSetCreate,
	RvmOpSegmentCreate
} RVM_OPERATION, *PRVM_OPERATION;

typedef struct _RVM_IOCTL_WORKING_SET_CREATE {

	//
	// [In] Volume Name in NT Path style. For e.g \??\O: 
	//

	WCHAR VolumeName[MAX_NT_VOLUME_LENGTH];

	//
	// [Out] Handle to the newly created 
	// working set
	//

	RVM_HANDLE Handle;

} RVM_IOCTL_WORKING_SET_CREATE, *PRVM_IOCTL_WORKING_SET_CREATE;

typedef struct _RVM_IOCTL_SEGMENT_CREATE {

	//
	// [Out] User Space address to new mapped memory
	// in kernel mode.
	//

	PVOID UserSpaceVA;

	//
	// [In] Size in multiple of RVM_BLOCK_SIZE
	//

	size_t Size;

	//
	// [In][Out] Segment Identifier
	// [In] When trying to map existing segment
	// [Out] New mapping when you get new Name
	//

	GUID SegmentName;

    //
    // [In] Active Working Set Handle
    //

    RVM_HANDLE WorkingSethandle;

	//
	// [Out] Handle to already created Segment
	//

	RVM_HANDLE SegmentHandle;

} RVM_IOCTL_SEGMENT_CREATE, *PRVM_IOCTL_SEGMENT_CREATE;

typedef struct _RVM_IOCTL_BUFFER {
	RVM_OPERATION Operation;
	union {
		RVM_IOCTL_WORKING_SET_CREATE WorkingSetCreate;
		RVM_IOCTL_SEGMENT_CREATE SegmentCreate;
	};
} RVM_IOCTL_BUFFER, *PRVM_IOCTL_BUFFER;