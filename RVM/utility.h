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
	__in PVOID Object,
	__in PLIST_ENTRY Entry
	);