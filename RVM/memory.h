///
/// @file memory.h
///
/// Memory related activities of RVM.
///
/// Author: Kaushal Ambani (2018)
///

#pragma once
#include <ntifs.h>
#include "globals.h"

struct _RVM_MEMORY_STORE;
typedef struct _RVM_MEMORY_STORE *PRVM_MEMORY_STORE;

//
// Represents a Memory block of size RVM_BLOCK_SIZE
//

typedef struct _RVM_MEMORY_FRAME {

	//
	// Pointer to Next Entry
	//

	SINGLE_LIST_ENTRY Next;

	//
	// Pointer to Memory Store
	//

	PRVM_MEMORY_STORE MemoryStore;

	//
	// Index of this memory frame
	//

	RVM_FRAME_INDEX Index;

	//
	// Link to Corresponding Disk Frame
	//

	RVM_FRAME_INDEX DiskFrame;

	//
	// Base Memory Address
	//

	PVOID BaseAddress;

	//
	// UndoLog Copy
	//

	PVOID UndoLog;

	union {
		struct {
			BOOLEAN UnderTransaction : 1;
			BOOLEAN Reserved : 7;
		};

		BOOLEAN Flags;
	};
} RVM_MEMORY_FRAME, *PRVM_MEMORY_FRAME;

typedef struct _RVM_MEMORY_STORE {

	//
	// Base Memory Frame
	//

	PRVM_MEMORY_FRAME BaseMemoryFrame;

	//
	// Stack of Memory Frames
	// To be used under MemoryFrameStackLock.
	//

	SINGLE_LIST_ENTRY MemoryFrameStack;

	//
	// Lock for MemoryFrameStack
	//

	KSPIN_LOCK MemoryFrameStackLock;

	//
	// Currently unused memory frames
	//

	size_t UnusedMemoryFrames;

	//
	// Size of the Memory Store in multiples of
	// RVM_BLOCK_SIZE
	//

	ULONG64 SizeInBlocks;
} RVM_MEMORY_STORE, *PRVM_MEMORY_STORE;

VOID
RvmMemoryStoreAcquireLock(
	__in PRVM_MEMORY_STORE MemoryStore
);

VOID
RvmMemoryStoreReleaseLock(
	__in PRVM_MEMORY_STORE MemoryStore
	);

NTSTATUS
RvmMemoryStoreInitialize(
	__in PRVM_MEMORY_STORE MemoryStore
	);

NTSTATUS
RvmMemoryStoreGetFrames(
	__in PRVM_MEMORY_STORE MemoryStore,
	__in PSINGLE_LIST_ENTRY  InputStack,
	__in size_t NumFrames
	);

NTSTATUS
RvmMemoryStoreReturnFrames(
	__in PRVM_MEMORY_STORE MemoryStore,
	__in PSINGLE_LIST_ENTRY Stack,
	__in size_t NumFrames
	);
