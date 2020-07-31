#pragma once

#include "Source.h"
NTSTATUS RegisterALECallout(DEVICE_OBJECT* deviceObj);
NTSTATUS AddAleCallout(HANDLE filterEngine);
NTSTATUS AddAleSublayer(HANDLE filterEngine);
NTSTATUS AddAleFilter(HANDLE filterEngine);
void UnitAleLayer(HANDLE filterEngine);

