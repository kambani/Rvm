///
/// @file memory.c
///
/// Implements Memory related APIs for RVM.
///
/// Author: Kaushal Ambani (2018)
///

#include "precomp.h"

NTSTATUS
RvmMemoryStoreInitialize(
	__in PRVM_MEMORY_STORE MemoryStore
)

/*++

Routine Description:

	Initializes memory store of a working set

Arguments:

	MemoryStore - Memory Store

Return Value:

	Returns the Status of the operation.

--*/

{
	ULONG32 Index;
	NTSTATUS Status;
	ULONG64 sizeInBlocks;

	sizeInBlocks = RVM_MAX_MEMORY_MAPPABLE / RVM_BLOCK_SIZE;

	if (sizeInBlocks > MAXULONG32) {
		sizeInBlocks = MAXULONG32;
	}

	if (sizeInBlocks == 0) {
		return STATUS_INVALID_PARAMETER;
	}

	//
	// Everything looks good
	// lets init the Memory store
	//

	MemoryStore->SizeInBlocks = sizeInBlocks;
	MemoryStore->UnusedMemoryFrames = sizeInBlocks;
	MemoryStore->MemoryFrameStack.Next = NULL;
	KeInitializeSpinLock(&MemoryStore->MemoryFrameStackLock);

	//
	// Allocate memory frame stack
	//

	Status = RvmAllocate(NonPagedPool,
						 sizeof(RVM_MEMORY_FRAME) * sizeInBlocks,
						 RVM_PT,
						 &MemoryStore->BaseMemoryFrame);

	memset(MemoryStore->BaseMemoryFrame,
		   0,
		   sizeof(RVM_MEMORY_FRAME) * sizeInBlocks);

	for (Index = 0; Index < sizeInBlocks; Index++) {
		MemoryStore->BaseMemoryFrame[Index].Index = Index;
		MemoryStore->BaseMemoryFrame[Index].MemoryStore = MemoryStore;
		MemoryStore->BaseMemoryFrame[Index].DiskFrame = RVM_INVALID_INDEX;
		PushEntryList(&MemoryStore->MemoryFrameStack,
					  &MemoryStore->BaseMemoryFrame[Index].Next);
	}

	return Status;
}

FORCEINLINE
VOID
RvmMemoryStoreAcquireLock(
	__in PRVM_MEMORY_STORE MemoryStore
)

/*++

Routine Description:

	Acquires locks on the MemoryStore.

Arguments:

	MemoryStore - Rvm Memory Store.

Return Value:

	None.

--*/

{
	KIRQL Irql;

	if (MemoryStore != NULL) {
		KeAcquireSpinLock(&MemoryStore->MemoryFrameStackLock, &Irql);
	}
}

FORCEINLINE
VOID
RvmMemoryStoreReleaseLock(
	__in PRVM_MEMORY_STORE MemoryStore
)

/*++

Routine Description:

	Releases locks on the MemoryStore.

Arguments:

	MemoryStore - Rvm Memory Store.

Return Value:

	None.

--*/

{
	KIRQL Irql;

	memset(&Irql, 0, sizeof(KIRQL));
	if (MemoryStore != NULL) {
		KeReleaseSpinLock(&MemoryStore->MemoryFrameStackLock, Irql);
	}
}

NTSTATUS
RvmMemoryStoreGetFrames(
	__in PRVM_MEMORY_STORE MemoryStore,
	__in PSINGLE_LIST_ENTRY  InputStack,
	__in size_t NumFrames
)

/*++

Routine Description:

	Pours required memory frames from the stack of MemoryStore
	into InputStack

Arguments:

	MemoryStore - Rvm Memory Store.
	InputStack - Stack into which you want the memory frames.
	NumFrames - Number of frames needed.

Return Value:

	Returns the Status of the operation.

--*/

{
	PSINGLE_LIST_ENTRY SEntry;
	NTSTATUS  Status;

	if (MemoryStore == NULL || InputStack == NULL ||
		NumFrames <= 0) {
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RvmMemoryStoreAcquireLock(MemoryStore);
	if (MemoryStore->UnusedMemoryFrames >= NumFrames) {
		while (NumFrames > 0) {
			SEntry = PopEntryList(&MemoryStore->MemoryFrameStack);
			PushEntryList(InputStack,
						  SEntry);
			NumFrames--;
		}

		Status = STATUS_SUCCESS;
	} else {
		Status = STATUS_INSUFFICIENT_RESOURCES;
	}

	RvmMemoryStoreReleaseLock(MemoryStore);
	return Status;
}

FORCEINLINE
VOID
RvmiMemoryStoreCleanFrame(
	__in PRVM_MEMORY_FRAME MemoryFrame
	)

/*++

Routine Description:

	Cleans up a memory frame for reuse.

Arguments:

	MemoryFrame - Rvm Memory frame.

Return Value:

	None.

--*/

{
	if (MemoryFrame != NULL) {
		MemoryFrame->DiskFrame = RVM_INVALID_INDEX;

		//
		// Undo Log memory to be freed by the Transactional process
		//
		MemoryFrame->UndoLog = NULL;
		MemoryFrame->BaseAddress = NULL;
		MemoryFrame->UnderTransaction = FALSE;
		//Add for other flags as they are introduced.
	}
}

NTSTATUS
RvmMemoryStoreReturnFrames(
	__in PRVM_MEMORY_STORE MemoryStore,
	__in PSINGLE_LIST_ENTRY Stack,
	__in size_t NumFrames
)

/*++

Routine Description:

	Returns all the memory frames from Stack into
	Memory Store

Arguments:

	MemoryStore - Rvm Memory Store.
	Stack - Stack from which you want to return memory frames.
	NumFrames - Number of frames.

Return Value:

	Returns the Status of the operation.

--*/

{
	PRVM_MEMORY_FRAME MemoryFrame;
	PSINGLE_LIST_ENTRY SEntry;

	if (MemoryStore == NULL || Stack == NULL ||
		NumFrames <= 0) {
		return STATUS_UNSUCCESSFUL;
	}

	RvmMemoryStoreAcquireLock(MemoryStore);
	while (NumFrames > 0) {
		SEntry = PopEntryList(Stack);
		MemoryFrame = CONTAINING_RECORD(SEntry, RVM_MEMORY_FRAME, Next);
		RvmiMemoryStoreCleanFrame(MemoryFrame);
		PushEntryList(&MemoryStore->MemoryFrameStack,
					  SEntry);
		NumFrames--;
	}

	RvmMemoryStoreReleaseLock(MemoryStore);
	return STATUS_SUCCESS;
}
