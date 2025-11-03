// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralMechGeometry.generated.h"

/**
 * Generates procedural geometry for mech exterior
 * Fully replaceable with static/skeletal meshes later
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STEELCATHEDRALS_API UProceduralMechGeometry : public UActorComponent
{
	GENERATED_BODY()

public:
	UProceduralMechGeometry();

	virtual void OnComponentCreated() override;

protected:
	virtual void BeginPlay() override;

public:
	// ============================================================
	// Geometry Configuration
	// ============================================================

	/** Use procedural geometry (false = use static meshes) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech Geometry")
	bool bUseProceduralGeometry = true;

	/** Scale multiplier for entire mech */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech Geometry|Scale", meta = (ClampMin = "0.1", ClampMax = "5.0"))
	float MechScale = 1.0f;

	/** Color tint for mech materials */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech Geometry|Appearance")
	FLinearColor MechColor = FLinearColor(0.15f, 0.15f, 0.18f, 1.0f);

	// ============================================================
	// Static Mesh Overrides (for asset swapping)
	// ============================================================

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech Geometry|Mesh Overrides")
	UStaticMesh* TorsoMeshOverride = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech Geometry|Mesh Overrides")
	UStaticMesh* HeadMeshOverride = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech Geometry|Mesh Overrides")
	UStaticMesh* LegMeshOverride = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mech Geometry|Mesh Overrides")
	UStaticMesh* ArmMeshOverride = nullptr;

	// ============================================================
	// Generation Functions
	// ============================================================

	/** Generate all mech geometry */
	UFUNCTION(BlueprintCallable, Category = "Mech Geometry")
	void GenerateMechGeometry();

	/** Clear all generated geometry */
	UFUNCTION(BlueprintCallable, Category = "Mech Geometry")
	void ClearGeometry();

	/** Swap to static mesh assets */
	UFUNCTION(BlueprintCallable, Category = "Mech Geometry")
	void SwapToStaticMeshes();

	/** Return to procedural geometry */
	UFUNCTION(BlueprintCallable, Category = "Mech Geometry")
	void SwapToProceduralGeometry();

private:
	// ============================================================
	// Procedural Mesh Components
	// ============================================================

	UPROPERTY()
	UProceduralMeshComponent* TorsoMesh;

	UPROPERTY()
	UProceduralMeshComponent* HeadMesh;

	UPROPERTY()
	UProceduralMeshComponent* LegLeftMesh;

	UPROPERTY()
	UProceduralMeshComponent* LegRightMesh;

	UPROPERTY()
	UProceduralMeshComponent* ArmLeftMesh;

	UPROPERTY()
	UProceduralMeshComponent* ArmRightMesh;

	// Static mesh components (for when using real assets)
	UPROPERTY()
	UStaticMeshComponent* TorsoStaticMesh;

	UPROPERTY()
	UStaticMeshComponent* HeadStaticMesh;

	UPROPERTY()
	UStaticMeshComponent* LegLeftStaticMesh;

	UPROPERTY()
	UStaticMeshComponent* LegRightStaticMesh;

	UPROPERTY()
	UStaticMeshComponent* ArmLeftStaticMesh;

	UPROPERTY()
	UStaticMeshComponent* ArmRightStaticMesh;

	// ============================================================
	// Mesh Generation Helpers
	// ============================================================

	/** Create a beveled box (looks better than plain cube) */
	void CreateBeveledBox(UProceduralMeshComponent* MeshComp, FVector Size, float BevelAmount = 5.0f);

	/** Create angled cockpit head */
	void CreateCockpitHead(UProceduralMeshComponent* MeshComp, FVector Size);

	/** Create leg segment with joints */
	void CreateLegSegment(UProceduralMeshComponent* MeshComp, FVector Size);

	/** Create arm segment */
	void CreateArmSegment(UProceduralMeshComponent* MeshComp, FVector Size);

	/** Apply material to procedural mesh */
	void ApplyProceduralMaterial(UProceduralMeshComponent* MeshComp);

	/** Load downloaded materials from Poly Haven */
	void LoadDownloadedMaterials();

	/** Create material instance with textures */
	UMaterialInstanceDynamic* CreateMaterialWithTextures(UTexture2D* Diffuse, UTexture2D* Normal, UTexture2D* Roughness);

	/** Helper: Add a quad to mesh data */
	void AddQuad(TArray<FVector>& Vertices, TArray<int32>& Triangles, TArray<FVector>& Normals,
	             TArray<FVector2D>& UVs, FVector V0, FVector V1, FVector V2, FVector V3, FVector Normal);

	/** Helper: Create UV coordinates for a plane */
	void GeneratePlaneUVs(TArray<FVector2D>& UVs, int32 Count);

	// ============================================================
	// Downloaded Materials
	// ============================================================

	UPROPERTY()
	UMaterialInstanceDynamic* RustyMetalMaterial;

	UPROPERTY()
	UMaterialInstanceDynamic* MetalPlateMaterial;
};
