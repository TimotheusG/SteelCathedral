// Copyright Epic Games, Inc. All Rights Reserved.

#include "MapSetupActor.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PostProcessComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AMapSetupActor::AMapSetupActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create root component
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Create directional light (sun) - MUCH BRIGHTER
	DirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("DirectionalLight"));
	DirectionalLight->SetupAttachment(RootComponent);
	DirectionalLight->SetRelativeRotation(FRotator(-40.0f, 0.0f, 0.0f)); // Angled down
	DirectionalLight->Intensity = 50.0f; // Much brighter!
	DirectionalLight->SetLightColor(FLinearColor(1.0f, 1.0f, 1.0f)); // Pure white
	DirectionalLight->SetCastShadows(true);
	DirectionalLight->bUseTemperature = false;
	DirectionalLight->ForwardShadingPriority = 1; // High priority for forward shading

	// Create sky light (ambient) - MUCH BRIGHTER
	SkyLight = CreateDefaultSubobject<USkyLightComponent>(TEXT("SkyLight"));
	SkyLight->SetupAttachment(RootComponent);
	SkyLight->Intensity = 5.0f; // Much brighter!
	SkyLight->SetLightColor(FLinearColor(1.0f, 1.0f, 1.0f)); // Pure white
	SkyLight->bRealTimeCapture = true;

	// Create sky atmosphere
	SkyAtmosphere = CreateDefaultSubobject<USkyAtmosphereComponent>(TEXT("SkyAtmosphere"));
	SkyAtmosphere->SetupAttachment(RootComponent);

	// Create height fog
	HeightFog = CreateDefaultSubobject<UExponentialHeightFogComponent>(TEXT("HeightFog"));
	HeightFog->SetupAttachment(RootComponent);
	HeightFog->FogDensity = 0.02f;
	HeightFog->FogHeightFalloff = 0.2f;

	// Create post process component for auto-exposure
	PostProcess = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcess"));
	PostProcess->SetupAttachment(RootComponent);
	PostProcess->bUnbound = true;
	PostProcess->Settings.bOverride_AutoExposureMethod = true;
	PostProcess->Settings.AutoExposureMethod = EAutoExposureMethod::AEM_Basic;
	PostProcess->Settings.bOverride_AutoExposureBias = true;
	PostProcess->Settings.AutoExposureBias = 0.0f;

	// Create floor plane
	FloorPlane = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FloorPlane"));
	FloorPlane->SetupAttachment(RootComponent);

	// Try to load engine cube mesh as floor
	static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane"));
	if (PlaneMesh.Succeeded())
	{
		FloorPlane->SetStaticMesh(PlaneMesh.Object);
		FloorPlane->SetRelativeScale3D(FVector(500.0f, 500.0f, 1.0f)); // 50000cm x 50000cm floor
		FloorPlane->SetRelativeLocation(FVector(0.0f, 0.0f, -5000.0f)); // 50m BELOW origin so mech stands above it
		FloorPlane->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		FloorPlane->SetCollisionResponseToAllChannels(ECR_Block);
	}

	UE_LOG(LogTemp, Warning, TEXT("MapSetupActor created"));
}

void AMapSetupActor::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("MapSetupActor BeginPlay - Setting up environment..."));

	SetupLighting();
	SetupAtmosphere();
	SetupEnvironment();

	UE_LOG(LogTemp, Warning, TEXT("MapSetupActor setup complete!"));
}

void AMapSetupActor::SetupLighting()
{
	// Lights are already configured in constructor
	// This function can be used for runtime adjustments
	UE_LOG(LogTemp, Log, TEXT("Lighting configured: Directional + Sky"));
}

void AMapSetupActor::SetupAtmosphere()
{
	// Atmosphere is already configured in constructor
	UE_LOG(LogTemp, Log, TEXT("Atmosphere configured: Sky + Fog"));
}

void AMapSetupActor::SetupEnvironment()
{
	// Environment (floor plane) is already configured
	UE_LOG(LogTemp, Log, TEXT("Environment configured: Floor plane"));
}
