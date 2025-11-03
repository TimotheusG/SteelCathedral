// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TerrainSpawner.generated.h"

/**
 * Spawns terrain debris and rubble using CityKit assets
 * Can be swapped to use different asset packs easily
 */
UCLASS()
class STEELCATHEDRALS_API ATerrainSpawner : public AActor
{
	GENERATED_BODY()

public:
	ATerrainSpawner();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	// ============================================================
	// Configuration
	// ============================================================

	/** Auto-spawn terrain on construction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	bool bAutoSpawn = true;

	/** Spawn area size (square) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain", meta = (ClampMin = "1000", ClampMax = "50000"))
	float SpawnAreaSize = 10000.0f;

	/** Number of rubble pieces to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain", meta = (ClampMin = "5", ClampMax = "200"))
	int32 RubbleCount = 30;

	/** Random seed for consistent placement */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
	int32 RandomSeed = 12345;

	// ============================================================
	// Asset References (CityKit)
	// ============================================================

	/** Array of rubble mesh paths to randomly spawn */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Assets")
	TArray<FSoftObjectPath> RubbleMeshPaths;

	/** Ground plane mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain|Assets")
	FSoftObjectPath GroundMeshPath;

	// ============================================================
	// Functions
	// ============================================================

	UFUNCTION(BlueprintCallable, Category = "Terrain")
	void SpawnTerrain();

	UFUNCTION(BlueprintCallable, Category = "Terrain")
	void ClearTerrain();

	/** Load default CityKit asset paths */
	UFUNCTION(BlueprintCallable, Category = "Terrain")
	void LoadDefaultCityKitAssets();

private:
	UPROPERTY()
	TArray<UStaticMeshComponent*> SpawnedMeshes;

	void SpawnGroundPlane();
	void SpawnRubblePieces();
};
