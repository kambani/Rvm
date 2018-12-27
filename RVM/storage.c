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
RvmStorageInitializeVolume(
	__in PUNICODE_STRING VolumeName,
	__in PRVM_DISK_STORE DiskStore
	)

{
	ULONG access;
	PFILE_OBJECT FileObject;
	HANDLE Handle;
	IO_STATUS_BLOCK ioStatus;
	OBJECT_ATTRIBUTES objectAttributes;
	ULONG64 sizeInBlocks;
	NTSTATUS status;

	//status = RvmStorageRetrieveVolumeIdentifier(VolumeName, &DiskStore->VolumeIdentifier);
	FileObject = NULL;

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