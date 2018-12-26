///
/// @file rvm.h
///
/// Declarations of RVM APIs.
///
/// Author: Kaushal Ambani (2018)
///

#pragma once
#include "storage.h"

#define IOCTL_RVM_WRITE               \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_RVM_READ                \
    CTL_CODE(FILE_DEVICE_UNKNOWN, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define RVM_BLOCK_SIZE_LOG2 16
#define RVM_BLOCK_SIZE (1 << RVM_BLOCK_SIZE_LOG2)

typedef ULONG RVM_HANDLE;

typedef struct _RVM_WORKING_SET {

	//
	// Pointer to next working set
	//

	LIST_ENTRY NextWorkingSet;

	//
	// List of all segments in this working set
	//

	LIST_ENTRY SegmentList;

	//
	// List of all transactions in this working set
	//

	LIST_ENTRY TransactionList;

	//
	// Volume Identifier
	//

	PRVM_VOLUME_IDENTIFIER Identifier;

	//
	// Disk store associated with this working set
	//

	PRVM_DISK_STORE DiskStore;

} RVM_WORKING_SET, *PRVM_WORKING_SET;

typedef struct _RVM_SEGMENT {

	//
	// Pointer to Next Segment
	//

	LIST_ENTRY NextSegment;

	//
	// Rtl Hash of the segment Name
	//

	ULONG SegmentNameHash;

	//
	// Size of Segment. Integer multiple of RVM_BLOCK_SIZE
	//

	ULONG SegmentSize;

	//
	// MDL representing this segment
	//

	PMDL SegmentMdl;

} RVM_SEGMENT, *PRVM_SEGMENT;

typedef enum _RVM_OPERATION {
	RvmOpWorkingSetCreate
} RVM_OPERATION, *PRVM_OPERATION;

typedef struct _RVM_IOCTL_WORKING_SET_CREATE {

	//
	// Volume Name in NT Path. For e.g \??\O: 
	//

	UNICODE_STRING VolumeName;

} RVM_IOCTL_WORKING_SET_CREATE, *PRVM_IOCTL_WORKING_SET_CREATE;

typedef struct _RVM_IOCTL_BUFFER {
	RVM_OPERATION Operation;
	union {
		RVM_IOCTL_WORKING_SET_CREATE WorkingSetCreate;
	};
} RVM_IOCTL_BUFFER, *PRVM_IOCTL_BUFFER;

RVM_HANDLE
RvmWorkingSetCreate(__in PUNICODE_STRING VolumeName);