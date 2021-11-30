///
/// @file storage.c
///
/// Implements Disk related APIs for RVM.
///
/// Author: Kaushal Ambani (2018)
///

#include "precomp.h"

DECLARE_CONST_UNICODE_STRING(RVM_RAW, L"RAW");

//NTSTATUS
//RvmStorageRetrieveVolumeIdentifier(
//	__in PUNICODE_STRING VolumeName,
//	__out PRVM_VOLUME_IDENTIFIER VolumeIdentifier
//)
//
///*++
//
//Routine Description:
//
//This function accepts the name of a volume and generates its unique volume
//identifier.
//
//Arguments:
//
//VolumeName - Supplies a pointer to a unicode string representing the name
//of the volume for which a unique identifier is to be generated.
//
//VolumeIdentifier - Supplies a pointer to a buffer that will contain a
//unique, persistent (even across host OS updates) identifier of the
//volume.
//
//Return Value:
//
//Returns the final status of the operation.
//
//--*/
//
//{
//	PDISK_EXTENT diskExtent;
//	PVOLUME_DISK_EXTENTS diskExtents;
//	ULONG diskNumber;
//	PUCHAR pch;
//	NTSTATUS status;
//
//	//
//	// The volume identifier consists of the serial number of the physical
//	// drive underlying the first extent of the volume.
//	//
//	// First, retrieve a description of the volume's disk extents.
//	//
//
//	diskExtents = NULL;
//	status = STATUS_DEVICE_CONFIGURATION_ERROR;//BcCacheStoreRetrieveDiskExtentsForVolume(VolumeName,
//			//										  &diskExtents);
//
//	if (!NT_SUCCESS(status)) {
//		goto done;
//	}
//
//	//
//	// If there are no extents, something is wrong.
//	// 
//
//	if (diskExtents->NumberOfDiskExtents == 0) {
//		status = STATUS_DEVICE_CONFIGURATION_ERROR;
//		goto done;
//	}
//
//	//
//	// Retrieve the storage device identifier from the physical disk.
//	//
//
//	diskExtent = &diskExtents->Extents[0];
//	diskNumber = diskExtent->DiskNumber;
//
//	VolumeIdentifier->VolumeOffset = diskExtent->StartingOffset.QuadPart;
//
//done:
//	if (diskExtents != NULL) {
//		RvmFree(diskExtents);
//	}
//
//	return status;
//}

NTSTATUS
RvmDiskStoreInitialize(
	__in PUNICODE_STRING VolumeName,
	__in PRVM_DISK_STORE DiskStore
	)

/*++

Routine Description:

	Initializes disk store of a working set

Arguments:

	VolumeName -  Volume Name in NT Path. For e.g \??\O:
	
	DiskStore - Disk Store.

Return Value:

	Returns the Status of the operation.

--*/

{
	ULONG access;
	PFILE_OBJECT FileObject;
	HANDLE Handle;
	ULONG Index;
	IO_STATUS_BLOCK ioStatus;
	OBJECT_ATTRIBUTES objectAttributes;
	ULONG64 sizeInBlocks;
	NTSTATUS status;

	//status = RvmStorageRetrieveVolumeIdentifier(VolumeName, &DiskStore->VolumeIdentifier);
	FileObject = NULL;
	Index = 0;

	union {
		FILE_FS_SIZE_INFORMATION SizeInfo;
		struct {
			FILE_FS_ATTRIBUTE_INFORMATION AttributeInfo;
			WCHAR Name[4];
		};

	} local;

	InitializeObjectAttributes(&objectAttributes,
							   VolumeName,
							   OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
							   NULL,
							   NULL);

	access = GENERIC_READ | GENERIC_WRITE;
	status = ZwCreateFile(&Handle,
						  access,
						  &objectAttributes,
						  &ioStatus,
						  NULL,
						  FILE_ATTRIBUTE_NORMAL,
						  FILE_SHARE_READ,
						  FILE_OPEN,
						  FILE_NON_DIRECTORY_FILE,
						  NULL,
						  0);

	if (!NT_SUCCESS(status)) {
		goto error;
	}

	status = ObReferenceObjectByHandle(Handle,
									   access,
									   *IoFileObjectType,
									   KernelMode,
									   &FileObject,
									   NULL);

	if (!NT_SUCCESS(status)) {
		goto error;
	}

	//
	// Make sure this is a RAW partition.
	//

	status = ZwQueryVolumeInformationFile(Handle,
										  &ioStatus,
										  &local.AttributeInfo,
										  sizeof(local),
										  FileFsAttributeInformation);

	if (!NT_SUCCESS(status)) {
		if ((status == STATUS_BUFFER_TOO_SMALL) ||
			(status == STATUS_BUFFER_OVERFLOW)) {

			status = STATUS_UNSUCCESSFUL;
		}

		goto error;
	}

	if ((local.AttributeInfo.FileSystemNameLength != RVM_RAW.Length) ||
		(RtlEqualMemory(&local.AttributeInfo.FileSystemName[0],
						RVM_RAW.Buffer,
						RVM_RAW.Length) == FALSE)) {

		//
		// Not RAW.
		//

		status = STATUS_UNSUCCESSFUL;
		goto error;
	}

	//
	// Get the volume size in blocks.
	// 

	status = ZwQueryVolumeInformationFile(Handle,
										  &ioStatus,
										  &local.SizeInfo,
										  sizeof(local),
										  FileFsSizeInformation);
	if (!NT_SUCCESS(status)) {
		goto error;
	}

	sizeInBlocks = (local.SizeInfo.TotalAllocationUnits.QuadPart *
					local.SizeInfo.SectorsPerAllocationUnit *
					local.SizeInfo.BytesPerSector) / RVM_BLOCK_SIZE;

	if (sizeInBlocks > MAXULONG32) {
		sizeInBlocks = MAXULONG32;
	}

	if (sizeInBlocks == 0) {
		status = STATUS_INVALID_PARAMETER;
		goto error;
	}

	//
	// Everything looks good
	// lets init the disk store
	//

	DiskStore->FileObject = FileObject;
	DiskStore->Handle = Handle;
	DiskStore->SizeInBlocks = sizeInBlocks;
	DiskStore->UnusedDiskFrames = sizeInBlocks;
	InitializeListHead(&DiskStore->DiskFrameStack);
	KeInitializeSpinLock(&DiskStore->DiskFrameStackLock);
	InitializeListHead(&DiskStore->UtilizedDiskFrames);
	KeInitializeSpinLock(&DiskStore->UtilizedDiskFramesLock);
	
	//
	// Allocate disk frame stack
	//

	RvmAllocate(NonPagedPool, 
				sizeof(RVM_DISK_FRAME) * sizeInBlocks,
				RVM_PT, 
				&DiskStore->BaseDiskFrame);

	memset(DiskStore->BaseDiskFrame, 
		   0, 
	       sizeof(RVM_DISK_FRAME) * sizeInBlocks);

	for (Index = 0; Index < sizeInBlocks; Index++) {
		DiskStore->BaseDiskFrame[Index].Index = Index;
		DiskStore->BaseDiskFrame[Index].DiskStore = DiskStore;
		InsertHeadList(&DiskStore->DiskFrameStack,
					   &DiskStore->BaseDiskFrame[Index].Next);
	}

	return STATUS_SUCCESS;

error:
	if (FileObject != NULL) {
		ObDereferenceObject(FileObject);
	}

	if (Handle != NULL) {
		ZwClose(Handle);
	}

	return status;
}

FORCEINLINE
VOID
RvmDiskStoreAcquireLock(
	__in PRVM_DISK_STORE DiskStore
)

/*++

Routine Description:

	Acquires locks on the DiskStore.

Arguments:

	DiskStore - Rvm Disk Store.

Return Value:

	None.

--*/

{
	KIRQL Irql;

	if (DiskStore != NULL) {
		KeAcquireSpinLock(&DiskStore->DiskFrameStackLock, &Irql);
	}
}

FORCEINLINE
VOID
RvmDiskStoreReleaseLock(
	__in PRVM_DISK_STORE DiskStore
)

/*++

Routine Description:

	Releases locks on the DiskStore.

Arguments:

	DiskStore - Rvm Disk Store.

Return Value:

	None.

--*/

{
	KIRQL Irql;

	memset(&Irql, 0, sizeof(KIRQL));
	if (DiskStore != NULL) {
		KeReleaseSpinLock(&DiskStore->DiskFrameStackLock, Irql);
	}
}


NTSTATUS
RvmDiskStoreGetFrames(
	__in PRVM_DISK_STORE DiskStore,
	__in PLIST_ENTRY InputStack,
	__in size_t NumFrames
)

/*++

Routine Description:

	Pours required disk frames from the stack of DiskStore
	into InputStack

Arguments:

	DiskStore - Rvm Disk Store.
	InputStack - Stack into which you want the memory frames.
	NumFrames - Number of frames needed.

Return Value:

	Returns the Status of the operation.

--*/

{
	PLIST_ENTRY SEntry;
	NTSTATUS  Status;

	if (DiskStore == NULL || InputStack == NULL ||
		NumFrames <= 0) {
		return STATUS_INSUFFICIENT_RESOURCES;
	}

	RvmDiskStoreAcquireLock(DiskStore);
	if (DiskStore->UnusedDiskFrames >= NumFrames) {
		while (NumFrames > 0) {
			SEntry = RemoveHeadList(&DiskStore->DiskFrameStack);
			InsertHeadList(InputStack,
						   SEntry);
			NumFrames--;
		}

		Status = STATUS_SUCCESS;
	}
	else {
		Status = STATUS_INSUFFICIENT_RESOURCES;
	}

	RvmDiskStoreReleaseLock(DiskStore);
	return Status;
}

NTSTATUS
RvmDiskStoreReturnFrames(
	__in PRVM_DISK_STORE DiskStore,
	__in PLIST_ENTRY Stack,
	__in size_t NumFrames
)

/*++

Routine Description:

	Returns all the memory frames from Stack into
	Disk Store

Arguments:

	MemoryStore - Rvm Disk Store.
	Stack - Stack from which you want to return disk frames.
	NumFrames - Number of frames.

Return Value:

	Returns the Status of the operation.

--*/

{
	PLIST_ENTRY SEntry;

	if (DiskStore == NULL || Stack == NULL ||
		NumFrames <= 0) {
		return STATUS_UNSUCCESSFUL;
	}

	RvmDiskStoreAcquireLock(DiskStore);
	while (NumFrames > 0) {
		//TODO. Clean up frames before returning them
		SEntry = RemoveHeadList(Stack);
		InsertHeadList(&DiskStore->DiskFrameStack,
					   SEntry);
		NumFrames--;
	}

	RvmDiskStoreReleaseLock(DiskStore);
	return STATUS_SUCCESS;
}

NTSTATUS
RvmiBuildDiskDriverRequest(
    __out PIRP *Irp,
    __in UCHAR MajorFunction,
    __in BOOLEAN Paging,
    __in PFILE_OBJECT FileObject,
    __in PMDL Mdl,
    __in ULONG Length,
    __in ULONG64 StartingOffset,
    __in PIO_STATUS_BLOCK IoStatusBlock
)
{

    PVOID buffer;
    PVOID callerBuffer;
    PDEVICE_OBJECT deviceObject;
    PIRP irp;
    PIO_STACK_LOCATION irpSp;

    buffer = NULL;
    deviceObject = IoGetRelatedDeviceObject(FileObject);
    irp = IoAllocateIrp(deviceObject->StackSize, FALSE);
    if (irp == NULL) {
        goto error;
    }

    irp->UserIosb = IoStatusBlock;
    irp->Flags |= IRP_NOCACHE;
    if (Paging != FALSE) {
        irp->Flags |= IRP_PAGING_IO;
    }

    irp->Tail.Overlay.Thread = PsGetCurrentThread();

    irpSp = IoGetNextIrpStackLocation(irp);
    irpSp->MajorFunction = MajorFunction;
    irpSp->FileObject = FileObject;
    irpSp->DeviceObject = deviceObject;
    if ((MajorFunction == IRP_MJ_READ) || (MajorFunction == IRP_MJ_WRITE)) {

        C_ASSERT(FIELD_OFFSET(IO_STACK_LOCATION, Parameters.Read.Length) ==
            FIELD_OFFSET(IO_STACK_LOCATION, Parameters.Write.Length));

        C_ASSERT(FIELD_OFFSET(IO_STACK_LOCATION, Parameters.Read.ByteOffset) ==
            FIELD_OFFSET(IO_STACK_LOCATION, Parameters.Write.ByteOffset));

        irpSp->Parameters.Read.Length = Length;
        irpSp->Parameters.Read.ByteOffset.QuadPart = StartingOffset;

        //
        // If the target device requests direct I/O or this is a paging I/O then
        // a pointer to the MDL is passed down.
        //

        irp->MdlAddress = Mdl;
        callerBuffer = NULL;
    }

    *Irp = irp;
    return STATUS_SUCCESS;

error:

    if (irp != NULL) {
        IoFreeIrp(irp);
    }

    if (buffer != NULL) {
        ExFreePool(buffer);
    }

    return STATUS_INSUFFICIENT_RESOURCES;
}

