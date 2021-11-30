///
/// @file rvm.h
///
/// Declarations of RVM APIs.
///
/// Author: Kaushal Ambani (2018)
///

#pragma once
#include "storage.h"
#include "memory.h"
#include "rvminterface.h"

#define RVM_BLOCK_SIZE_LOG2 16
#define RVM_BLOCK_SIZE (1 << RVM_BLOCK_SIZE_LOG2)
#define RVM_MAX_MEMORY_MAPPABLE_LOG2 32
#define RVM_MAX_MEMORY_MAPPABLE  (1UI64 << RVM_MAX_MEMORY_MAPPABLE_LOG2)

#define RVM_CONFIG_FILE_PATH L"\\global??\\c:\\rvm.cfg"

typedef enum _RVM_OBJECT_ENUM {
	RvmWorkingSet,
	RvmSegment,
	RvmTransaction,
	RvmLogRecord
} RVM_OBJECT_ENUM, *PRVM_OBJECT_ENUM;

typedef struct _RVM_OBJECT_HEADER {
	LIST_ENTRY Entry;
	RVM_OBJECT_ENUM ObjectType;
	ULONG32 ReferenceCount;
} RVM_OBJECT_HEADER, *PRVM_OBJECT_HEADER;

typedef struct _RVM_WORKING_SET {

	//
	// Object Header;
	//

	RVM_OBJECT_HEADER;

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

	RVM_DISK_STORE DiskStore;

	//
	// Memory Store associated with this
	// working set
	//

	RVM_MEMORY_STORE MemoryStore;

} RVM_WORKING_SET, *PRVM_WORKING_SET;

typedef struct _RVM_SEGMENT {

	//
	// Pointer to Next Segment
	//

	LIST_ENTRY NextSegment;

	//
	// Unique Name of the Segment
	//

	GUID SegmentName;

	//
	// Size of Segment. Integer multiple of RVM_BLOCK_SIZE
	//

	size_t SegmentSize;

	//
	// Handle of the segment if mapped
	//
	RVM_HANDLE SegmentHandle;

	//
	// MDL representing memory mapped
	// by this segment
	//

	MDL SegmentMdl;

	//
	// User Space VA
	//

	PVOID SegmentUserSpaceVA;

	//
	// Stack of RVM_MEMORY_FRAME representing 
	// this segment
	//
	
	SINGLE_LIST_ENTRY SegmentMemoryStack;

	//
	// Stack of Disk Frames held by this segment.
	//

	LIST_ENTRY SegmentDiskStack;

} RVM_SEGMENT, *PRVM_SEGMENT;

typedef struct _RVM_CONFIG_FILE {
	ULONG64 NumOfDiskFrames;
	ULONG DiskFrames[];
} RVM_CONFIG_FILE, *PRVM_CONFIG_FILE;

NTSTATUS
RvmWorkingSetCreate(__in PUNICODE_STRING VolumeName,
					__out PRVM_HANDLE Handle);

NTSTATUS
RvmSegmentCreate(__in RVM_HANDLE WorkingSetHandle,
				 __in size_t Size,
				 __out PVOID *UserSpaceVA,
				 __inout PGUID SegmentName,
				 __out PRVM_HANDLE SegmentHandle);