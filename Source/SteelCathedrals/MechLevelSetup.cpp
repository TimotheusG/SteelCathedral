// Copyright Epic Games, Inc. All Rights Reserved.

#include "MechLevelSetup.h"
#include "TerrainSpawner.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

AMechLevelSetup::AMechLevelSetup()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AMechLevelSetup::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("MechLevelSetup: Configuring level..."));

	if (bAutoSpawnTerrain)
	{
		SpawnTerrain();
	}

	if (bAutoSetupLighting)
	{
		SetupLighting();
	}

	UE_LOG(LogTemp, Warning, TEXT("✅ MechLevelSetup: Level configuration complete"));
}

void AMechLevelSetup::SpawnTerrain()
{
	if (!GetWorld())
		return;

	UE_LOG(LogTemp, Log, TEXT("Spawning terrain..."));

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	TerrainSpawner = GetWorld()->SpawnActor<ATerrainSpawner>(
		ATerrainSpawner::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (TerrainSpawner)
	{
		UE_LOG(LogTemp, Warning, TEXT("✅ Terrain spawned automatically"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("❌ Failed to spawn terrain"));
	}
}

void AMechLevelSetup::SetupLighting()
{
	if (!GetWorld())
		return;

	// Check if directional light already exists
	bool bHasDirectionalLight = false;
	for (TActorIterator<ADirectionalLight> It(GetWorld()); It; ++It)
	{
		bHasDirectionalLight = true;
		break;
	}

	if (!bHasDirectionalLight)
	{
		UE_LOG(LogTemp, Log, TEXT("No directional light found, creating one..."));

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;

		ADirectionalLight* Sun = GetWorld()->SpawnActor<ADirectionalLight>(
			ADirectionalLight::StaticClass(),
			FVector(0, 0, 500),
			FRotator(-50.0f, 0.0f, 0.0f),
			SpawnParams
		);

		if (Sun && Sun->GetLightComponent())
		{
			Sun->GetLightComponent()->SetIntensity(10.0f);
			Sun->GetLightComponent()->SetLightColor(FLinearColor(1.0f, 0.95f, 0.9f)); // Warm sunlight
			Sun->GetLightComponent()->CastShadows = true;

			UE_LOG(LogTemp, Warning, TEXT("✅ Directional light created"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("Directional light already exists"));
	}
}
