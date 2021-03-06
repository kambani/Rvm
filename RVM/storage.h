///
/// @file storage.h
///
/// Disk related activities of RVM.
///
/// Author: Kaushal Ambani (2018)
///

#pragma once
#include <ntifs.h>
#include "globals.h"

#define RVM_DRIVE_SERIAL_SIZE 32

struct _RVM_DISK_STORE;
typedef struct _RVM_DISK_STORE *PRVM_DISK_STORE;

typedef struct _RVM_VOLUME_IDENTIFIER {

	//
	// The offset on the drive at which that extent begins.
	//

	ULONG64 VolumeOffset;

} RVM_VOLUME_IDENTIFIER, *PRVM_VOLUME_IDENTIFIER;

typedef struct _RVM_DISK_FRAME {

	//
	// Pointer to Next Entry
	//

	SLIST_ENTRY Next;

	//
	// Disk Frame Index
	//

	RVM_FRAME_INDEX Index;

	//
	// Disk Store
	//

	PRVM_DISK_STORE DiskStore;

	//
	// Crc
	//

	ULONG64 Crc;

} RVM_DISK_FRAME, *PRVM_DISK_FRAME;


typedef struct _RVM_DISK_STORE {

	//
	// This is used to positively identify the disk backing this 
	// working set
	//

	//RVM_VOLUME_IDENTIFIER VolumeIdentifier;

	//
	// The volume path used.  Note that this is NOT the persistent
	// RAW volume identifier; that's what VolumeIdentifier is for.
	//
	// This is just the path that was used to open the RAW volume this
	// time around.
	//

	UNICODE_STRING Path;

	//
	// File handle and file object pointer.
	//

	HANDLE Handle;
	PFILE_OBJECT FileObject;

	//
	// Base Disk Frame
	//

	PRVM_DISK_FRAME BaseDiskFrame;

	//
	// Stack of Disk Frames
	//

	SLIST_HEADER DiskFrameStack;

	//
	// Lock for DiskFrameStack
	//

	KSPIN_LOCK DiskFrameStackLock;

	//
	// Currently unused disk frames
	//

	size_t UnusedDiskFrames;

	//
	// Size of the disk in multiples of
	// RVM_BLOCK_SIZE
	//

	ULONG64 SizeInBlocks;
} RVM_DISK_STORE, *PRVM_DISK_STORE;

//NTSTATUS
//RvmStorageRetrieveVolumeIdentifier(
//	__in PUNICODE_STRING VolumeName,
//	__out PRVM_VOLUME_IDENTIFIER VolumeIdentifier
//	);

NTSTATUS
RvmStorageInitialize(
	__in PUNICODE_STRING VolumeName,
	__in PRVM_DISK_STORE DiskStore
	);
