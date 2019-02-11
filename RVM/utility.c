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
	__in PVOID Object
)

/*++

Routine Description:

	Adds Object into the Global Handle Table and returns a unique Handle.

Arguments:

	ObjectType - Rvm Object Type.

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

	case RvmSegment:
		Index = Slot = Slot % RVM_MAX_SEGMENTS;
		do {
			if (RvmGlobalData.SegmentHandleTable[Index].Address == NULL) {
				Found = TRUE;
				RvmGlobalData.SegmentHandleTable[Index].Address = Object;
				Slot = Index;
				break;
			}

			Index += 1;
			if (Index >= RVM_MAX_SEGMENTS) {
				Index = 0;
			}
		} while (Index != Slot);

		break;

	default:
		break;
	}

	if (Found != FALSE) {
		return Slot;
	}

	return RVM_INVALID_INDEX;
}

PVOID
RvmGetObject(
	__in RVM_OBJECT_ENUM ObjectType,
	__in RVM_HANDLE Handle
)

/*++

Routine Description:

	Given a object handle, returns the address of the object
	from the Global Handle Table.

Arguments:

	ObjectType - Rvm Object Type.

	Handle - RVM_HANDLE.

Return Value:

	Memory Address of the object.

--*/

{

	switch (ObjectType) {
	case RvmWorkingSet:
		if (Handle < RVM_MAX_WORKING_SETS) {
			return RvmGlobalData.WorkingSetHandleTable[Handle].Address;
		} else {
			return NULL;
		}

		break;
	case RvmSegment:
		if (Handle < RVM_MAX_SEGMENTS) {
			return RvmGlobalData.SegmentHandleTable[Handle].Address;
		} else {
			return NULL;
		}

		break;

	default:
		return NULL;
	}
}

PVOID
RvmWorkingSetCheckSegmentExists(
	__in PRVM_WORKING_SET WorkingSet,
	__in PGUID SegmentName
)

/*++

Routine Description:

	Checks if a segment of given name exists in
	the working set. If yes returns pointer to that segment
	else returns NULL.

Arguments:

	WorkingSet - WorkingSet.

	SegmentName - Segment Name in GUID.

Return Value:

	Memory Address of Segment object.

--*/

{
	PLIST_ENTRY Head;
	PLIST_ENTRY Entry;
	PRVM_SEGMENT Segment;
	if (WorkingSet == NULL || SegmentName == NULL) {
		return NULL;
	}

	Head = &WorkingSet->SegmentList;
	Entry = Head->Flink;
	while (Entry != Head) {
		Segment = CONTAINING_RECORD(Entry, RVM_SEGMENT, NextSegment);
		if (memcmp(&Segment->SegmentName, SegmentName, sizeof(GUID)) == 0) {
			return Segment;
		}
		Entry = Entry->Flink;
	}

	return NULL;
}