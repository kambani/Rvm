#pragma once
#include <ntifs.h>

VOID
RvmFree(
	__in PVOID Memory
);

NTSTATUS
RvmAllocate(
	__in POOL_TYPE PoolType,
	__in SIZE_T SizeInBytes,
	__in ULONG PoolTag,
	__out PVOID *Memory
	);

RVM_HANDLE
RvmAddObject(
	__in RVM_OBJECT_ENUM ObjectType,
	__in PVOID Object
	);

PVOID
RvmGetObject(
	__in RVM_OBJECT_ENUM ObjectType,
	__in RVM_HANDLE Handle
	);

PVOID
RvmWorkingSetCheckSegmentExists(
	__in PRVM_WORKING_SET WorkingSet,
	__in PGUID SegmentName
	);