///
/// @file driver.c
///
/// Contains implementation of standard windows driver control routines.
///
/// Author: Kaushal Ambani (2018)
///

#include "precomp.h"
DRIVER_INITIALIZE DriverEntry;

_Dispatch_type_(IRP_MJ_DEVICE_CONTROL)
DRIVER_DISPATCH RvmDispatchDeviceControl;

RVM_GLOBAL_DATA RvmGlobalData = { 0 };

NTSTATUS
RvmDeviceControlRead(
	__in ULONG InputLength,
	__inout PULONG OutputLength,
	__in PRVM_IOCTL_BUFFER Buffer)

{
	NTSTATUS Status;
	UNREFERENCED_PARAMETER(InputLength);
	UNREFERENCED_PARAMETER(OutputLength);
	UNREFERENCED_PARAMETER(Buffer);
	Status = STATUS_SUCCESS;

	return Status;
}

NTSTATUS
RvmDeviceControlWrite(
	__in ULONG InputLength,
	__inout PULONG OutputLength,
	__in PRVM_IOCTL_BUFFER Buffer
	)

{
	ULONG BytesNeeded;
	NTSTATUS Status;
	UNICODE_STRING VolumeNameUnicode;

	Status = STATUS_UNSUCCESSFUL;

	switch (Buffer->Operation) 
	{
	case RvmOpWorkingSetCreate:
		UNREFERENCED_PARAMETER(OutputLength);
		BytesNeeded = RTL_SIZEOF_THROUGH_FIELD(RVM_IOCTL_BUFFER, 
											   WorkingSetCreate);

		if (BytesNeeded != InputLength) {
			Status = STATUS_INFO_LENGTH_MISMATCH;
            break;
		}

		RtlInitUnicodeString(&VolumeNameUnicode,
							 &Buffer->WorkingSetCreate.VolumeName[0]);

		Status = RvmWorkingSetCreate(&VolumeNameUnicode,
									 &Buffer->WorkingSetCreate.Handle);
		break;

	case RvmOpSegmentCreate:
		UNREFERENCED_PARAMETER(OutputLength);
		BytesNeeded = RTL_SIZEOF_THROUGH_FIELD(RVM_IOCTL_BUFFER,
											   SegmentCreate);

		if (BytesNeeded != InputLength) {
			Status = STATUS_INFO_LENGTH_MISMATCH;
			break;
		}

        Status = RvmSegmentCreate(Buffer->SegmentCreate.WorkingSethandle,
                                  Buffer->SegmentCreate.Size,
                                  &Buffer->SegmentCreate.UserSpaceVA,
                                  &Buffer->SegmentCreate.SegmentName,
                                  &Buffer->SegmentCreate.SegmentHandle);
		break;

	default:
		break;
	}

	return Status;
}

NTSTATUS
RvmDispatchDeviceControl(
	DEVICE_OBJECT *DeviceObject,
	IRP *Irp
)
{
	UNREFERENCED_PARAMETER(DeviceObject);
	PVOID Buffer;
	ULONG InputLength;
	ULONG OutputLength;
	NTSTATUS Status;
	CONST PIO_STACK_LOCATION Stack = IoGetCurrentIrpStackLocation(Irp);

	Buffer = Irp->AssociatedIrp.SystemBuffer;
	Irp->IoStatus.Information = 0;
	Status = STATUS_SUCCESS;
	InputLength = Stack->Parameters.DeviceIoControl.InputBufferLength;
	OutputLength = Stack->Parameters.DeviceIoControl.OutputBufferLength;

	switch (Stack->Parameters.DeviceIoControl.IoControlCode)
	{
	case IOCTL_RVM_WRITE:

		Status = RvmDeviceControlWrite(InputLength,
									   &OutputLength,
									   Buffer);
		break;

	case IOCTL_RVM_READ:
		
		Status = RvmDeviceControlRead(InputLength,
									  &OutputLength,
									  Buffer);
		break;
	}

	return Status;
}

NTSTATUS
DriverEntry(
	_In_ PDRIVER_OBJECT     DriverObject,
	_In_ PUNICODE_STRING    RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	UNICODE_STRING DeviceName;
	ULONG Index;
	NTSTATUS status = STATUS_SUCCESS;

	//
	// Create the device object
	//

	RtlInitUnicodeString(&DeviceName, RVM_DEVICE_NAME);
	InitializeListHead(&RvmGlobalData.WorkingSetHead);

	status = IoCreateDevice(DriverObject,
							0,
							&DeviceName,
							FILE_DEVICE_UNKNOWN,
							0,
							FALSE,
							&RvmGlobalData.RvmdeviceObject);

	if (!NT_SUCCESS(status))
	{
		goto done;
	}

	//
	// Set driver dispatch routines
	//

	for (Index = 0; Index <= IRP_MJ_MAXIMUM_FUNCTION; Index++) {
		DriverObject->MajorFunction[Index] = RvmDispatchDeviceControl;
	}

	//DriverObject->DriverUnload = DriverUnload;
	//DriverObject->MajorFunction[IRP_MJ_CREATE] = SmbdDispatchCreateCleanupClose;
	//DriverObject->MajorFunction[IRP_MJ_CLEANUP] = SmbdDispatchCreateCleanupClose;
	//DriverObject->MajorFunction[IRP_MJ_CLOSE] = SmbdDispatchCreateCleanupClose;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = RvmDispatchDeviceControl;

done:

	if (!NT_SUCCESS(status))
	{
		//
		// Cleanup
		//

	}

	return status;
}