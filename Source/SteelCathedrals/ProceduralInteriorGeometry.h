// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralInteriorGeometry.generated.h"

/**
 * Procedurally generates mech interior (cockpit, corridors, reactor room)
 * Can be swapped with modular static mesh pieces
 */
UCLASS()
class STEELCATHEDRALS_API AProceduralInteriorGeometry : public AActor
{
	GENERATED_BODY()

public:
	AProceduralInteriorGeometry();

protected:
	virtual void BeginPlay() override;
	virtual void OnConstruction(const FTransform& Transform) override;

public:
	// ============================================================
	// Configuration
	// ============================================================

	/** Generate interior on construction */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
	bool bAutoGenerate = true;

	/** Use procedural geometry vs static meshes */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior")
	bool bUseProceduralGeometry = true;

	/** Scale multiplier */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior", meta = (ClampMin = "0.5", ClampMax = "2.0"))
	float InteriorScale = 1.0f;

	/** Interior wall color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior|Appearance")
	FLinearColor WallColor = FLinearColor(0.25f, 0.25f, 0.28f, 1.0f);

	/** Floor color */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interior|Appearance")
	FLinearColor FloorColor = FLinearColor(0.18f, 0.18f, 0.20f, 1.0f);

	// ============================================================
	// Components
	// ============================================================

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* InteriorRoot;

	// Procedural meshes
	UPROPERTY()
	UProceduralMeshComponent* CockpitFloor;

	UPROPERTY()
	UProceduralMeshComponent* CockpitWalls;

	UPROPERTY()
	UProceduralMeshComponent* CockpitCeiling;

	UPROPERTY()
	UProceduralMeshComponent* CockpitWindow;

	UPROPERTY()
	UProceduralMeshComponent* ReactorRoom;

	UPROPERTY()
	UProceduralMeshComponent* Corridors;

	// Spawned stations
	UPROPERTY()
	TArray<class AMechStation*> Stations;

	// Spawned hazards for testing
	UPROPERTY()
	TArray<class AFireHazardActor*> TestHazards;

	// ============================================================
	// Generation Functions
	// ============================================================

	UFUNCTION(BlueprintCallable, Category = "Interior")
	void GenerateInterior();

	UFUNCTION(BlueprintCallable, Category = "Interior")
	void ClearInterior();

private:
	void CreateCockpitGeometry();
	void CreateReactorRoomGeometry();
	void CreateCorridorGeometry();
	void CreateWindowGeometry();

	// Spawn stations in the cockpit
	void SpawnStations();

	// Spawn test hazards for debugging
	void SpawnTestHazards();

	// Helper to create a room
	void CreateRoom(UProceduralMeshComponent* MeshComp, FVector RoomSize, bool bHasWindow = false);

	// Helper to create flat panel
	void CreatePanel(TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals,
	                 TArray<FVector2D>& UVs, FVector Center, FVector Size, FRotator Rotation);

	void AddQuad(TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals,
	             TArray<FVector2D>& UVs, FVector V0, FVector V1, FVector V2, FVector V3, FVector Normal);
};
