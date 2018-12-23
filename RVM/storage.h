#pragma once
#include <ntddk.h>

#define RVM_DRIVE_SERIAL_SIZE 32
typedef ULONG32 RVM_FRAME_INDEX;

struct _RVM_DISK_STORE;
typedef struct _RVM_DISK_STORE *PRVM_DISK_STORE;

typedef struct _RVM_VOLUME_IDENTIFIER {

	//
	// Null-terminated serial number of the physical drive upon which the first
	// volume extent resides.
	//

	UCHAR DriveSerialNumber[RVM_DRIVE_SERIAL_SIZE];

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
	// Disk Frame
	//

	RVM_FRAME_INDEX DiskFrame;

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

	RVM_VOLUME_IDENTIFIER VolumeIdentifier;

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
	// Size in number of disk frames
	//

	size_t TotalDiskFrames;

	//
	// Currently unused disk frames
	//

	size_t UnusedDiskFrames;

} RVM_DISK_STORE, *PRVM_DISK_STORE;
