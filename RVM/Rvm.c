///
/// @file rvm.c
///
/// Contains the implementations of the RVM APIs.
///
/// Author: Kaushal Ambani (2018)
///

#include "precomp.h"

NTSTATUS
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

	//
	// Initialize Disk Store
	//
	Status = RvmStorageInitialize(VolumeName,
								  &WorkingSet->DiskStore);
	if (!NT_SUCCESS(Status)) {
		goto Done;
	}

	//
	// Initialize Memory Store
	//
	Status = RvmMemoryStoreInitialize(&WorkingSet->MemoryStore);
	if (!NT_SUCCESS(Status)) {
		//TODO Close Disk handles
		goto Done;
	}

	InitializeListHead(&WorkingSet->SegmentList);
	InitializeListHead(&WorkingSet->TransactionList);

	*Handle = RvmAddObject(RvmWorkingSet, WorkingSet);

	if (*Handle != RVM_INVALID_INDEX) {
		InsertTailList(&RvmGlobalData.WorkingSetHead,
					   &WorkingSet->Entry);
	} else {
		//TODO Close Disk Handle and Memory Handle
		Status = STATUS_INSUFFICIENT_RESOURCES;
		goto Done;
	}

Done:
	if (!NT_SUCCESS(Status)) {
		RvmFree(WorkingSet);
	}

	return Status;
}

NTSTATUS
RvmSegmentCreate(__in RVM_HANDLE WorkingSetHandle,
				 __in size_t Size,
				 __out PVOID *UserSpaceVA,
				 __inout PGUID SegmentName,
				 __out PRVM_HANDLE SegmentHandle)

/*++

Routine Description:

	This routine creates a segment i.e. allocates
	kernel mode memory for the user which is to be mapped
	into user space.

Arguments:

	WorkingSetHandle - Handle to already existing working set.

	Size - Size of the mapped memory needed in multiple of RVM_BLOCK_SIZE.
		   If size is not specified as a multiple of RVM_BLOCK_SIZE then 
		   its wrapped to nearest number of RVM_BLOCK_SIZE.

	UserSpaceVA - User Space VA representing this mapped memory.

	SegmentName - If supplied NULL, then indicates new mapping
				  If not NULL, then load the existing mapping from the disk

	SegmentHandle - Out for newly created Segment Handle.

Return Value:

	Returns the Status of the operation.

--*/

{
	PVOID AllocatedMemoryKernelVa;
	BOOLEAN CleanMemory;
	BOOLEAN FreeSegment;
	size_t NumMemoryBlock;
	BOOLEAN ReturnFrames;
	PRVM_SEGMENT Segment;
	NTSTATUS Status;
	PRVM_WORKING_SET WorkingSet;

	AllocatedMemoryKernelVa = NULL;
	CleanMemory = FALSE;
	FreeSegment = FALSE;
	ReturnFrames = FALSE;
	Status = STATUS_UNSUCCESSFUL;
	*UserSpaceVA = NULL;
	*SegmentHandle = RVM_INVALID_INDEX;
	WorkingSet = RvmGetObject(RvmWorkingSet,
							  WorkingSetHandle);
	NT_ASSERT(Size > 0);

	if (WorkingSet == NULL) {
		return STATUS_NOT_FOUND;
	}

	if (SegmentName != NULL) {

		//
		// Check if the Segment is already mapped
		//

		Segment = RvmWorkingSetCheckSegmentExists(WorkingSet, SegmentName);
		if (Segment != NULL) {

			//
			// Segment is already mapped. Return it
			//

			Status = STATUS_SUCCESS;
		}
		else {

			//
			// Check the config file if the segment
			// exists on the disk.
			//
		}

	} else {

		//
		// We have to create new mapping.
		//

		Status = RvmAllocate(NonPagedPool,
							 sizeof(RVM_SEGMENT),
							 RVM_PT,
							 &Segment);
		if (!NT_SUCCESS(Status)) {
			goto Done;
		}
		memset(Segment, 0, sizeof(RVM_SEGMENT));
		NumMemoryBlock = Size / RVM_BLOCK_SIZE;
		Segment->SegmentSize = NumMemoryBlock * sizeof(RVM_BLOCK_SIZE);
		ExUuidCreate(&Segment->SegmentName);

		//
		// Yank out equivalent memory frames
		//

		Status = RvmMemoryStoreGetFrames(&WorkingSet->MemoryStore,
									     &Segment->SegmentMemoryStack,
										 NumMemoryBlock);
		if (!NT_SUCCESS(Status)) {
			FreeSegment = TRUE;
			goto Done;
		}

		//
		// Allocate Non Paged memory of the size requested
		//

		Status = RvmAllocate(NonPagedPool,
							 Segment->SegmentSize,
							 RVM_PT,
							 &AllocatedMemoryKernelVa);
		if (!NT_SUCCESS(Status)) {
			FreeSegment = TRUE;
			ReturnFrames = TRUE;
			goto Done;
		}

		//
		// Map kernel mode memory into user space.
		//

		MmInitializeMdl(&Segment->SegmentMdl,
						AllocatedMemoryKernelVa,
						NumMemoryBlock * sizeof(RVM_BLOCK_SIZE));
		MmBuildMdlForNonPagedPool(&Segment->SegmentMdl);
		Segment->SegmentUserSpaceVA = MmMapLockedPagesSpecifyCache(&Segment->SegmentMdl,
																   UserMode,
																   MmCached,
																   NULL,
																   FALSE,
																   NormalPagePriority);
		if (Segment->SegmentUserSpaceVA == NULL) {
			FreeSegment = TRUE;
			CleanMemory = TRUE;
			ReturnFrames = TRUE;
			IoFreeMdl(&Segment->SegmentMdl);
			Status = STATUS_INSUFFICIENT_RESOURCES;
			goto Done;
		}

		//
		// Map Memory Frames into Disk
		//

		//
		// We now have the segment ready
		//

		Segment->SegmentHandle = RvmAddObject(RvmSegment, Segment);

	}

Done:
	if (CleanMemory == TRUE) {
		NT_ASSERT(AllocatedMemoryKernelVa != NULL);
		RvmFree(AllocatedMemoryKernelVa);
	}

	if (ReturnFrames == TRUE) {
		RvmMemoryStoreReturnFrames(&WorkingSet->MemoryStore,
								   &Segment->SegmentMemoryStack,
								   Segment->SegmentSize);
	}

	if (FreeSegment == TRUE) {
		RvmFree(Segment);
	}

	if (NT_SUCCESS(Status)) {
		*SegmentHandle = Segment->SegmentHandle;
		*UserSpaceVA = Segment->SegmentUserSpaceVA;
	}

	return Status;
}