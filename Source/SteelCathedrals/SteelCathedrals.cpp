// Copyright Epic Games, Inc. All Rights Reserved.

#include "SteelCathedrals.h"
#include "Modules/ModuleManager.h"

IMPLEMENT_PRIMARY_GAME_MODULE(FSteelCathedralsModule, SteelCathedrals, "SteelCathedrals");

void FSteelCathedralsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory
	UE_LOG(LogTemp, Warning, TEXT("Steel Cathedrals module has started!"));
}

void FSteelCathedralsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module
	UE_LOG(LogTemp, Warning, TEXT("Steel Cathedrals module has shut down"));
}
