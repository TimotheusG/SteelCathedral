// Copyright Epic Games, Inc. All Rights Reserved.

#include "MechInterior.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FMechInteriorModule, MechInterior, "MechInterior");

void FMechInteriorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	UE_LOG(LogTemp, Warning, TEXT("MechInterior module has started!"));
}

void FMechInteriorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	UE_LOG(LogTemp, Warning, TEXT("MechInterior module has shut down"));
}
