// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MechLevelSetup.generated.h"

class ATerrainSpawner;

/**
 * Level setup actor - automatically configures the level
 * Place one of these in each level to auto-setup terrain, lighting, etc.
 */
UCLASS()
class MECHINTERIOR_API AMechLevelSetup : public AActor
{
	GENERATED_BODY()

public:
	AMechLevelSetup();

protected:
	virtual void BeginPlay() override;

public:
	/** Auto-spawn terrain */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Setup")
	bool bAutoSpawnTerrain = true;

	/** Auto-setup lighting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Setup")
	bool bAutoSetupLighting = true;

	/** Terrain spawner reference (spawned at runtime) */
	UPROPERTY(BlueprintReadOnly, Category = "Level Setup")
	ATerrainSpawner* TerrainSpawner;

private:
	void SpawnTerrain();
	void SetupLighting();
};
