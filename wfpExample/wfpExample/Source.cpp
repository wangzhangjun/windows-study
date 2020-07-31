#include "Source.h"
#include "AleCallout.h"
#include "StreamCallout.h"

PDEVICE_OBJECT DeviceObject = NULL;
HANDLE EngineHandle = NULL;

EXTERN_C_START
DRIVER_INITIALIZE DriverEntry;
EXTERN_C_END

void UninitWfp()
{
	if (EngineHandle != NULL) {
		//stream layer uninit
		UnitStreamLayer(EngineHandle);
		//ale layer unint
		UnitAleLayer(EngineHandle);
	}
	FwpmEngineClose(EngineHandle);
}

VOID DriverUnload(PDRIVER_OBJECT driverObj)
{
	UNREFERENCED_PARAMETER(driverObj);
	UninitWfp();
	IoDeleteDevice(DeviceObject);
	KdPrint(("wfpdemo driver unload\r\n"));
	return;
}

NTSTATUS WfpOpenEngine()
{
	return FwpmEngineOpen(
		NULL,
		RPC_C_AUTHN_WINNT,
		NULL,
		NULL,
		&EngineHandle);
}

NTSTATUS WfpRegisterCallout()
{
	NTSTATUS status = STATUS_SUCCESS;
	status = RegisterALECallout(DeviceObject);
	if (!NT_SUCCESS(status)) {
		KdPrint(("RegisterALECallout error!, status is  0x%08x\n", status));
		return status;
	}
	status = RegisterStreamCallout(DeviceObject);
	if (!NT_SUCCESS(status)) {
		KdPrint(("RegisterStreamCallout error!, status is  0x%08x\n", status));
		return status;
	}
	return status;
}

NTSTATUS WfpAddCallout()
{
	NTSTATUS status = STATUS_SUCCESS;
	status = AddStreamCallout(EngineHandle);
	if (!NT_SUCCESS(status)) {
		KdPrint(("AddStreamCallout error!, status is 0x%08x\n", status));
		return status;
	}
	status = AddAleCallout(EngineHandle);
	if (!NT_SUCCESS(status)) {
		KdPrint(("AddAleCallout error!, status is 0x%08x\n", status));
		return status;
	}
	return status;
}

NTSTATUS WfpAddSubLayer()
{
	NTSTATUS status = STATUS_SUCCESS;
	status = AddStreamSublayer(EngineHandle);
	if (!NT_SUCCESS(status)) {
		KdPrint(("AddStreamSublayer error!, status is 0x%08x\n", status));
		return status;
	}
	status = AddAleSublayer(EngineHandle);
	if (!NT_SUCCESS(status)) {
		KdPrint(("AddAleSublayer error!, status is 0x%08x\n", status));
		return status;
	}
	return status;
}

NTSTATUS WfpAddFilter()
{
	NTSTATUS status = STATUS_SUCCESS;
	status = AddStreamFilter(EngineHandle);
	if (!NT_SUCCESS(status)) {
		KdPrint(("AddStreamFilter error!, status is 0x%08x\n", status));
		return status;
	}
	status = AddAleFilter(EngineHandle);
	if (!NT_SUCCESS(status)) {
		KdPrint(("AddAleFilter error!, status is 0x%08x\n", status));
		return status;
	}
	return status;
}

NTSTATUS InitializeWfp()
{
	if (!NT_SUCCESS(WfpOpenEngine())) {
		goto end;
	}
	if (!NT_SUCCESS(WfpRegisterCallout())) {
		goto end;
	}
	if (!NT_SUCCESS(WfpAddCallout())) {
		goto end;
	}
	if (!NT_SUCCESS(WfpAddSubLayer())) {
		goto end;
	}
	if (!NT_SUCCESS(WfpAddFilter())) {
		goto end;
	}
	return STATUS_SUCCESS;

end:
	UninitWfp();
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject, IN PUNICODE_STRING registryPath)
{
	UNREFERENCED_PARAMETER(DriverObject);
	UNREFERENCED_PARAMETER(registryPath);
	KdPrint(("wfpdemo driver load"));
	NTSTATUS status = STATUS_SUCCESS;
	DriverObject->DriverUnload = DriverUnload;

	//create device for this driver
	status = IoCreateDevice(
		DriverObject,
		0, NULL,
		FILE_DEVICE_UNKNOWN,
		0, FALSE,
		&DeviceObject
	);
	if (!NT_SUCCESS(status))
	{
		return status;
	}
	status = InitializeWfp();
	if (!NT_SUCCESS(status))
	{
		IoDeleteDevice(DeviceObject);
	}

	return status;
}
