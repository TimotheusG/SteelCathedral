// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MechGameMode.generated.h"

/**
 * Steel Cathedral Game Mode
 * Handles spawning player in mech, level setup
 */
UCLASS()
class MECHINTERIOR_API AMechGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMechGameMode();
};
