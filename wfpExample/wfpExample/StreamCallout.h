#pragma once

#include "Source.h"

NTSTATUS RegisterStreamCallout(DEVICE_OBJECT* deviceObj);
NTSTATUS AddStreamCallout(HANDLE filterEngine);
NTSTATUS AddStreamSublayer(HANDLE filterEngine);
NTSTATUS AddStreamFilter(HANDLE filterEngine);
void UnitStreamLayer(HANDLE filterEngine);
