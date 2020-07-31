#include "StreamCallout.h"

UINT32 StreamRegCalloutId = 0, StreamAddCountId = 0;
UINT64 StreamFilterId = 0;

NTSTATUS NTAPI StreamNotifyCallback(_In_ FWPS_CALLOUT_NOTIFY_TYPE notifyType,
	_In_ const GUID* filterKey,
	_Inout_ FWPS_FILTER* filter)
{
	UNREFERENCED_PARAMETER(notifyType);
	UNREFERENCED_PARAMETER(filterKey);
	UNREFERENCED_PARAMETER(filter);

	return STATUS_SUCCESS;
}

void NTAPI StreamFilterCallback(
	_In_ const FWPS_INCOMING_VALUES* Values,
	_In_ const FWPS_INCOMING_METADATA_VALUES* MetaData,
	_Inout_opt_ void* layerData,
	_In_opt_ const void* context,
	_In_ const FWPS_FILTER* filter,
	_In_ UINT64 flowContext,
	_Inout_ FWPS_CLASSIFY_OUT* classifyOut)
{
	UNREFERENCED_PARAMETER(Values);
	UNREFERENCED_PARAMETER(MetaData);
	UNREFERENCED_PARAMETER(layerData);
	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(filter);
	UNREFERENCED_PARAMETER(flowContext);
	UNREFERENCED_PARAMETER(classifyOut);
	
	KdPrint(("data is here\r\n"));
	classifyOut->actionType = FWP_ACTION_PERMIT;
}
NTSTATUS RegisterStreamCallout(DEVICE_OBJECT* deviceObj)
{
	NTSTATUS status = STATUS_SUCCESS;
	FWPS_CALLOUT Callout = { 0 };
	Callout.calloutKey = WFP_SAMPLE_ESTABLISHED_CALLOUT_V4_GUID;
	Callout.flags = 0;
	Callout.classifyFn = StreamFilterCallback;
	Callout.flowDeleteFn = NULL;
	Callout.notifyFn = StreamNotifyCallback;

	status = FwpsCalloutRegister(deviceObj, &Callout, &StreamRegCalloutId);
	return status;
}

NTSTATUS AddStreamCallout(HANDLE filterEngine)
{
	FWPM_CALLOUT callout = { 0 };

	callout.flags = 0;
	callout.displayData.name = L"EstablishedStreamCalloutName";
	callout.displayData.description = L"EstablishedStreamCalloutName";
	callout.calloutKey = WFP_SAMPLE_ESTABLISHED_CALLOUT_V4_GUID;
	callout.applicableLayer = FWPM_LAYER_STREAM_V4;  //layer

	return FwpmCalloutAdd(filterEngine, &callout, NULL, &StreamAddCountId);
}

NTSTATUS AddStreamSublayer(HANDLE filterEngine)
{
	FWPM_SUBLAYER SubLayer = { 0 };

	SubLayer.displayData.name = L"EstablishedStreamCalloutName";
	SubLayer.displayData.description = L"EstablishedStreamCalloutName";
	SubLayer.subLayerKey = WFP_SAMPLE_SUB_LAYER_GUID;
	SubLayer.weight = 65500;  //first priorty

	return FwpmSubLayerAdd(filterEngine, &SubLayer, NULL);
}

NTSTATUS AddStreamFilter(HANDLE filterEngine)
{
	FWPM_FILTER filter = { 0 };
	FWPM_FILTER_CONDITION condition[1] = { 0 };

	filter.displayData.name = L"FilterCalloutName";
	filter.displayData.description = L"FilterCalloutName";
	filter.layerKey = FWPM_LAYER_STREAM_V4;//和哪一层关联起来
	filter.subLayerKey = WFP_SAMPLE_SUB_LAYER_GUID;//子层关联
	filter.weight.type = FWP_EMPTY;
	filter.numFilterConditions = 1;
	filter.filterCondition = condition;
	filter.action.type = FWP_ACTION_CALLOUT_TERMINATING;
	filter.action.calloutKey = WFP_SAMPLE_ESTABLISHED_CALLOUT_V4_GUID;

	condition[0].fieldKey = FWPM_CONDITION_IP_LOCAL_PORT;
	condition[0].matchType = FWP_MATCH_LESS_OR_EQUAL; //小于或者等于
	condition[0].conditionValue.type = FWP_UINT16;
	condition[0].conditionValue.uint16 = 65000;  //可以拦截大多数的端口

	return FwpmFilterAdd(
		filterEngine,
		&filter,
		NULL,
		&StreamFilterId);
}

void UnitStreamLayer(HANDLE filterEngine)
{
	if (StreamFilterId != 0) {
		FwpmFilterDeleteById(filterEngine, StreamFilterId);
		FwpmSubLayerDeleteByKey(filterEngine, &WFP_SAMPLE_SUB_LAYER_GUID);
	}
	if (StreamAddCountId != 0) {
		FwpmCalloutDeleteById(filterEngine, StreamAddCountId);
	}
	if (StreamRegCalloutId != 0) {
		FwpsCalloutUnregisterById(StreamRegCalloutId);
	}
}