#include "precomp.h"

VOID
RvmFree(
	__in PVOID Memory
)

{

#if DBG

	//
	// Some logging.
	//

#endif

	ExFreePool(Memory);
	return;
}

NTSTATUS
RvmAllocate(
	__in POOL_TYPE PoolType,
	__in SIZE_T SizeInBytes,
	__in ULONG PoolTag,
	__out PVOID *Memory
)

/*++

Routine Description:

	Allocates memory from non paged pool.

Arguments:

	PoolType - Pool type.

	SizeInBytes - Size in Bytes.

	PoolTag - Unique to tag the memory.

	Memory - Pointer where you want your memory created.

Return Value:

	NTSTATUS

--*/

{
	PVOID memory;
	NTSTATUS status;

	memory = ExAllocatePoolWithTag(PoolType, SizeInBytes, PoolTag);
	status = STATUS_SUCCESS;
	if (memory == NULL) {
		status = STATUS_INSUFFICIENT_RESOURCES;
	}

	if (NT_SUCCESS(status)) {
		*Memory = memory;
	}

	return status;
}

RVM_HANDLE
RvmAddObject(
	__in RVM_OBJECT_ENUM ObjectType,
	__in PVOID Object,
	__in PLIST_ENTRY Entry
)

/*++

Routine Description:

	Adds Object into the Global Handle Table and returns a unique Handle.

Arguments:

	RVM_OBJECT_ENUM - Rvm Object Type.

	Object - Pointer to Object.

	Entry - Pointer to LIST_ENTRY inside the object so it can be added to 
			the global queue.

Return Value:

	RVM_HANDLE.

--*/

{
	ULONG Slot;
	ULONG Index;
	BOOLEAN Found;

	Slot = (ULONG)ReadTimeStampCounter();
	Found = FALSE;

	switch (ObjectType) {
	case RvmWorkingSet:
		Index = Slot = Slot % RVM_MAX_WORKING_SETS;
		do {
			if (RvmGlobalData.WorkingSetHandleTable[Index].Address == NULL) {
				Found = TRUE;
				RvmGlobalData.WorkingSetHandleTable[Index].Address = Object;
				Slot = Index;
				break;
			}

			Index += 1;
			if (Index >= RVM_MAX_WORKING_SETS) {
				Index = 0;
			}
		} while (Index != Slot);

		break;

	default:
		break;
	}

	if (Found != FALSE) {
		InsertTailList(&RvmGlobalData.WorkingSetHead, Entry);
		return Slot;
	}

	return RVM_INVALID_INDEX;
}