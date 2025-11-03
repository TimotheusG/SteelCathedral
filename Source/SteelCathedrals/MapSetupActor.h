// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapSetupActor.generated.h"

/**
 * Automatically sets up a basic playable environment with lighting
 * Spawns this actor in BeginPlay to ensure maps have minimum viable setup
 */
UCLASS()
class STEELCATHEDRALS_API AMapSetupActor : public AActor
{
	GENERATED_BODY()

public:
	AMapSetupActor();

protected:
	virtual void BeginPlay() override;

private:
	/** Directional light for main scene lighting */
	UPROPERTY(VisibleAnywhere, Category = "Lighting")
	class UDirectionalLightComponent* DirectionalLight;

	/** Sky light for ambient lighting */
	UPROPERTY(VisibleAnywhere, Category = "Lighting")
	class USkyLightComponent* SkyLight;

	/** Atmospheric fog for sky rendering */
	UPROPERTY(VisibleAnywhere, Category = "Atmosphere")
	class USkyAtmosphereComponent* SkyAtmosphere;

	/** Exponential height fog */
	UPROPERTY(VisibleAnywhere, Category = "Atmosphere")
	class UExponentialHeightFogComponent* HeightFog;

	/** Post process volume for exposure settings */
	UPROPERTY(VisibleAnywhere, Category = "PostProcess")
	class UPostProcessComponent* PostProcess;

	/** Floor plane mesh */
	UPROPERTY(VisibleAnywhere, Category = "Environment")
	class UStaticMeshComponent* FloorPlane;

	void SetupLighting();
	void SetupAtmosphere();
	void SetupEnvironment();
};
