#include "FloorCheck.h"

DEFINE_LOG_CATEGORY(LogFloorCheck);

#define LOCTEXT_NAMESPACE "FFloorCheckModule"

void FFloorCheckModule::StartupModule()
{
	UE_LOG(LogFloorCheck, Log, TEXT("Floor Check module started"));
}

void FFloorCheckModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFloorCheckModule, FloorCheck)
