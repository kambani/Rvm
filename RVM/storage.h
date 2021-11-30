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

	LIST_ENTRY Next;

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

	LIST_ENTRY DiskFrameStack;

	//
	// Lock for DiskFrameStack
	//

	KSPIN_LOCK DiskFrameStackLock;

	//
	// Disk Frames under utilization
	//

	LIST_ENTRY UtilizedDiskFrames;

	//
	// Lock for UtilizedDiskFrames
	//

	KSPIN_LOCK UtilizedDiskFramesLock;

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
RvmDiskStoreInitialize(
	__in PUNICODE_STRING VolumeName,
	__in PRVM_DISK_STORE DiskStore
	);

FORCEINLINE
VOID
RvmDiskStoreAcquireLock(
	__in PRVM_DISK_STORE DiskStore
	);

FORCEINLINE
VOID
RvmDiskStoreReleaseLock(
	__in PRVM_DISK_STORE DiskStore
	);

NTSTATUS
RvmDiskStoreGetFrames(
	__in PRVM_DISK_STORE DiskStore,
	__in PLIST_ENTRY InputStack,
	__in size_t NumFrames
	);

NTSTATUS
RvmDiskStoreReturnFrames(
	__in PRVM_DISK_STORE DiskStore,
	__in PLIST_ENTRY Stack,
	__in size_t NumFrames
	);
