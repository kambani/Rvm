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

Return Value:

	RVM_HANDLE.

--*/

{
	InsertTailList(&RvmGlobalData.WorkingSetHead, Entry);
}