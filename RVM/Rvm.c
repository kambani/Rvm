///
/// @file rvm.c
///
/// Contains the implementations of the RVM APIs.
///
/// Author: Kaushal Ambani (2018)
///

#include "precomp.h"

NTSTATUS
RvmWorkingSetCreate(__in PUNICODE_STRING VolumeName)

/*++

Routine Description:

	This routine creates a working set. As an input takes in
	Volume name which it will use for persistence.

Arguments:

	VolumeName - Volume Name in NT Path. For e.g \??\O: 

Return Value:

	Returns the final status of the operation.

--*/

{
	NTSTATUS Status;
	RVM_VOLUME_IDENTIFIER VolumeIdentifier;
	PRVM_WORKING_SET WorkingSet;
	RVM_HANDLE Handle;

	Status = RvmStorageRetrieveVolumeIdentifier(VolumeName, &VolumeIdentifier);
	if (!NT_SUCCESS(Status)) {
		goto Done;
	}

	Status = RvmAllocate(NonPagedPool, 
						 sizeof(RVM_WORKING_SET), 
						 RVM_PT, 
						 WorkingSet);

	if (!NT_SUCCESS(Status)) {
		return Status;
	}

	RtlZeroMemory(WorkingSet, sizeof(RVM_WORKING_SET));
	Handle = RvmAddObject(RvmWorkingSet, WorkingSet);
	//Status = RvmStorageInitializeVolume();

Done:
	return Status;
}