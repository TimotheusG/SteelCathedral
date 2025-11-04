// Copyright Epic Games, Inc. All Rights Reserved.

#include "SteelCathedralsGameMode.h"
#include "Mech.h"
#include "CrewMember.h"
#include "MapSetupActor.h"
#include "ProceduralInteriorGeometry.h"
#include "UObject/ConstructorHelpers.h"
#include "TimerManager.h"

ASteelCathedralsGameMode::ASteelCathedralsGameMode()
{
	// Set default pawn to CrewMember (crew walking around INTERIOR)
	// The Mech is a separate HULL actor in the world
	DefaultPawnClass = ACrewMember::StaticClass();

	// Initialize default spawn locations for 3 crew members
	InitializeSpawnLocations();

	UE_LOG(LogTemp, Warning, TEXT("SteelCathedralsGameMode created - Default pawn: CrewMember (INTERIOR character)"));
}

void ASteelCathedralsGameMode::InitializeSpawnLocations()
{
	// ============================================================
	// SPAWN LOCATIONS ARE NOW DETERMINED BY INTERIOR GEOMETRY
	// The interior knows where stations are, so it knows where crew should spawn
	// No more magic numbers!
	// ============================================================

	// We'll populate spawn locations in StartPlay() once the interior exists
	CrewSpawnLocations.Empty();
	CrewSpawnRotations.Empty();

	UE_LOG(LogTemp, Log, TEXT("Spawn locations will be determined by interior geometry"));
}

FTransform ASteelCathedralsGameMode::GetNextCrewSpawnTransform()
{
	// If we've spawned more players than we have spawn points, wrap around
	int32 SpawnIndex = SpawnedPlayerCount % FMath::Max(1, CrewSpawnLocations.Num());

	FVector Location = (CrewSpawnLocations.IsValidIndex(SpawnIndex))
		? CrewSpawnLocations[SpawnIndex]
		: FVector(200.0f, 0.0f, 2600.0f); // Fallback

	FRotator Rotation = (CrewSpawnRotations.IsValidIndex(SpawnIndex))
		? CrewSpawnRotations[SpawnIndex]
		: FRotator::ZeroRotator; // Fallback

	SpawnedPlayerCount++;

	UE_LOG(LogTemp, Warning, TEXT("Spawning crew member %d at location: %s"),
		SpawnedPlayerCount, *Location.ToString());

	return FTransform(Rotation, Location, FVector::OneVector);
}

void ASteelCathedralsGameMode::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("SteelCathedralsGameMode BeginPlay - Spawning map setup..."));

	// Spawn map setup actor to configure lighting and environment
	if (GetWorld())
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Name = FName(TEXT("MapSetup"));

		MapSetup = GetWorld()->SpawnActor<AMapSetupActor>(
			AMapSetupActor::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParams
		);

		if (MapSetup)
		{
			UE_LOG(LogTemp, Warning, TEXT("Map setup spawned successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn map setup actor!"));
		}

		// Spawn the Mech HULL actor at configured location
		SpawnParams.Name = FName(TEXT("MechHull"));

		// Find ground level at spawn location
		FVector SpawnLocation = MechSpawnLocation;
		FHitResult HitResult;
		FVector TraceStart = FVector(MechSpawnLocation.X, MechSpawnLocation.Y, 10000.0f); // Start high above
		FVector TraceEnd = FVector(MechSpawnLocation.X, MechSpawnLocation.Y, -10000.0f); // End far below

		// Trace down to find ground
		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility))
		{
			// Found ground - spawn mech standing on it
			SpawnLocation.Z = HitResult.Location.Z;
			UE_LOG(LogTemp, Warning, TEXT("Ground found at Z=%.2f, spawning mech there"), SpawnLocation.Z);
		}
		else
		{
			// No ground found - use configured Z or default to 0
			UE_LOG(LogTemp, Warning, TEXT("No ground found, using configured spawn height Z=%.2f"), SpawnLocation.Z);
		}

		MechActor = GetWorld()->SpawnActor<AMech>(
			AMech::StaticClass(),
			SpawnLocation,
			MechSpawnRotation,
			SpawnParams
		);

		if (MechActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("Mech HULL spawned at: %s (Ground-aligned)"), *SpawnLocation.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to spawn Mech HULL actor at %s!"), *SpawnLocation.ToString());
		}
	}
}

void ASteelCathedralsGameMode::StartPlay()
{
	Super::StartPlay();

	// Position crew members inside mech cockpit
	// HIERARCHY: World -> Mech -> Interior -> Crew

	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("No world! Cannot spawn crew!"));
		return;
	}

	if (!MechActor)
	{
		UE_LOG(LogTemp, Error, TEXT("MechActor is NULL! Cannot spawn crew in cockpit!"));
		return;
	}

	// Get interior geometry from mech (it spawns it in BeginPlay)
	AProceduralInteriorGeometry* Interior = MechActor->GetInteriorEnvironment();
	if (!Interior)
	{
		UE_LOG(LogTemp, Error, TEXT("Interior is NULL! Cannot get spawn positions!"));
		return;
	}

	// Get spawn positions from interior (these are relative to interior origin)
	TArray<FVector> InteriorSpawnPositions = Interior->GetCrewSpawnPositions();

	// Get mech's actual world position
	FVector MechWorldLocation = MechActor->GetActorLocation();
	FRotator MechWorldRotation = MechActor->GetActorRotation();

	// Get interior's actual world position (it's attached to mech)
	FVector InteriorWorldLocation = Interior->GetActorLocation();

	UE_LOG(LogTemp, Warning, TEXT("Positioning crew:"));
	UE_LOG(LogTemp, Warning, TEXT("  Mech at: %s"), *MechWorldLocation.ToString());
	UE_LOG(LogTemp, Warning, TEXT("  Interior at: %s"), *InteriorWorldLocation.ToString());

	// Populate spawn locations array from interior
	CrewSpawnLocations.Empty();
	CrewSpawnRotations.Empty();

	for (const FVector& SpawnPos : InteriorSpawnPositions)
	{
		// Spawn positions are relative to interior, convert to world
		FVector WorldPos = InteriorWorldLocation + SpawnPos;
		CrewSpawnLocations.Add(WorldPos);
		CrewSpawnRotations.Add(FRotator::ZeroRotator);

		UE_LOG(LogTemp, Warning, TEXT("  Spawn point: Interior=%s -> World=%s"),
			*SpawnPos.ToString(), *WorldPos.ToString());
	}

	// Now position all players
	int32 PlayerIndex = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			// Get next crew spawn
			FTransform SpawnTransform = GetNextCrewSpawnTransform();

			PC->GetPawn()->SetActorLocationAndRotation(
				SpawnTransform.GetLocation(),
				SpawnTransform.GetRotation().Rotator()
			);

			UE_LOG(LogTemp, Warning, TEXT("  Crew %d positioned at: %s"),
				PlayerIndex + 1,
				*SpawnTransform.GetLocation().ToString());

			PlayerIndex++;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Positioned %d crew member(s) in cockpit"), PlayerIndex);

}
