///
/// @file rvm.c
///
/// Contains the implementations of the RVM APIs.
///
/// Author: Kaushal Ambani (2018)
///

#include "precomp.h"

RVM_HANDLE
RvmWorkingSetCreate(__in PUNICODE_STRING VolumeName,
					__out PRVM_HANDLE Handle)

/*++

Routine Description:

	This routine creates a working set. As an input takes in
	Volume name which it will use for persistence.

Arguments:

	VolumeName - Volume Name in NT Path. For e.g \??\O:

	Handle - Out Parameter to get Handle to Working Set

Return Value:

	Returns the Status of the operation.

--*/

{
	NTSTATUS Status;
	PRVM_WORKING_SET WorkingSet;

	*Handle = RVM_INVALID_INDEX;
	Status = RvmAllocate(NonPagedPool, 
						 sizeof(RVM_WORKING_SET), 
						 RVM_PT, 
						 &WorkingSet);

	if (!NT_SUCCESS(Status)) {
		Status = STATUS_INSUFFICIENT_RESOURCES;
		goto Done;
	}

	RtlZeroMemory(WorkingSet, sizeof(RVM_WORKING_SET));
	Status = RvmStorageInitializeVolume(VolumeName, 
										&WorkingSet->DiskStore);

	if (!NT_SUCCESS(Status)) {
		RvmFree(WorkingSet);
		goto Done;
	}

	*Handle = RvmAddObject(RvmWorkingSet, WorkingSet, &WorkingSet->Entry);

Done:
	return Status;
}