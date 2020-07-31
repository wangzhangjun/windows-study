#include "AleCallout.h"

UINT32 AleRegCalloutId = 0, AleAddCountId = 0;
UINT64 AleFilterId = 0;
#define FORMAT_ADDR(x) (x>>24)&0xFF, (x>>16)&0xFF, (x>>8)&0xFF, x&0xFF
void NTAPI AleFilterCallback(
	_In_ const FWPS_INCOMING_VALUES* Values,
	_In_ const FWPS_INCOMING_METADATA_VALUES* MetaData,
	_Inout_opt_ void* layerData,
	_In_opt_ const void* context,
	_In_ const FWPS_FILTER* filter,
	_In_ UINT64 flowContext,
	_Inout_ FWPS_CLASSIFY_OUT* classifyOut)
{
	UNREFERENCED_PARAMETER(MetaData);
	UNREFERENCED_PARAMETER(layerData);
	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(filter);
	UNREFERENCED_PARAMETER(flowContext);
	UNREFERENCED_PARAMETER(classifyOut);

	UINT32 LocalIP;
	UINT16 LocalPort;
	KdPrint(("ALE is here\r\n"));

	if (!(classifyOut->rights & FWPS_RIGHT_ACTION_WRITE)) {
		classifyOut->actionType = FWP_ACTION_PERMIT;
		if (filter->flags & FWPS_FILTER_FLAG_CLEAR_ACTION_RIGHT) {
			classifyOut->rights &= ~FWPS_RIGHT_ACTION_WRITE;
		}
		return;
	}

	LocalIP = Values->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_LOCAL_ADDRESS].value.uint32;
	LocalPort = Values->incomingValue[FWPS_FIELD_ALE_AUTH_CONNECT_V4_IP_LOCAL_PORT].value.uint16;
	KdPrint(("local conn is: %d.%d.%d.%d:%hu\n", FORMAT_ADDR(LocalIP), LocalPort));
	classifyOut->actionType = FWP_ACTION_PERMIT;
}

NTSTATUS NTAPI AleNotifyCallback(_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	_In_ const GUID* filterKey,
	_Inout_ FWPS_FILTER* filter)
{
	UNREFERENCED_PARAMETER(notifyType);
	UNREFERENCED_PARAMETER(filterKey);
	UNREFERENCED_PARAMETER(filter);

	return STATUS_SUCCESS;
}

NTSTATUS RegisterALECallout(DEVICE_OBJECT* deviceObj)
{
	NTSTATUS status = STATUS_SUCCESS;
	FWPS_CALLOUT Callout = { 0 };
	Callout.calloutKey = WFP_ALE_AUTH_CONNECT_CALLOUT_V4_GUID;
	Callout.flags = 0;
	Callout.classifyFn = AleFilterCallback;
	Callout.flowDeleteFn = NULL;
	Callout.notifyFn = AleNotifyCallback;

	status = FwpsCalloutRegister(deviceObj, &Callout, &AleRegCalloutId);
	return status;
}
NTSTATUS AddAleCallout(HANDLE filterEngine) 
{
	FWPM_CALLOUT callout = { 0 };

	callout.flags = 0;
	callout.displayData.name = L"AleCalloutName";
	callout.displayData.description = L"AleCalloutName";
	callout.calloutKey = WFP_ALE_AUTH_CONNECT_CALLOUT_V4_GUID;
	callout.applicableLayer = FWPM_LAYER_ALE_AUTH_CONNECT_V4;  //layer

	return FwpmCalloutAdd(filterEngine, &callout, NULL, &AleAddCountId);
}
NTSTATUS AddAleSublayer(HANDLE filterEngine)
{
	FWPM_SUBLAYER SubLayer = { 0 };

	SubLayer.subLayerKey = WFP_ALE_AUTH_CONNEC_SUB_LAYER_GUID;
	SubLayer.displayData.name = L"AleCalloutName";
	SubLayer.displayData.description = L"AleCalloutName";
	SubLayer.weight = 65500;

	return FwpmSubLayerAdd(filterEngine, &SubLayer, NULL);
}
NTSTATUS AddAleFilter(HANDLE filterEngine)
{
		FWPM_FILTER filter = { 0 };

	filter.displayData.name = L"AleFilterCalloutName";
	filter.displayData.description = L"AleFilterCalloutName";
	filter.layerKey = FWPM_LAYER_ALE_AUTH_CONNECT_V4;


	filter.subLayerKey = WFP_ALE_AUTH_CONNEC_SUB_LAYER_GUID;//×Ó²ã¹ØÁª
	filter.weight.type = FWP_EMPTY;
	filter.numFilterConditions = 0;
	filter.rawContext = 0;
	filter.filterCondition = NULL;
	filter.action.type = FWP_ACTION_CALLOUT_TERMINATING;
	filter.action.calloutKey = WFP_ALE_AUTH_CONNECT_CALLOUT_V4_GUID;


	return FwpmFilterAdd(
		filterEngine,
		&filter,
		NULL,
		&AleFilterId);
}

void UnitAleLayer(HANDLE filterEngine)
{
	if (AleFilterId != 0) {
		FwpmFilterDeleteById(filterEngine, AleFilterId);
		FwpmSubLayerDeleteByKey(filterEngine, &WFP_ALE_AUTH_CONNEC_SUB_LAYER_GUID);
	}
	if (AleAddCountId != 0) {
		FwpmCalloutDeleteById(filterEngine, AleAddCountId);
	}
	if (AleRegCalloutId != 0) {
		FwpsCalloutUnregisterById(AleRegCalloutId);
	}
}
