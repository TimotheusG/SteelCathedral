// Copyright Epic Games, Inc. All Rights Reserved.

#include "MechGameMode.h"
#include "PlayerMechPawn.h"
#include "UObject/ConstructorHelpers.h"

AMechGameMode::AMechGameMode()
{
	// Set default pawn to mech
	DefaultPawnClass = APlayerMechPawn::StaticClass();

	UE_LOG(LogTemp, Warning, TEXT("MechGameMode created - Default pawn: PlayerMechPawn"));
}
