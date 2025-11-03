// Copyright Epic Games, Inc. All Rights Reserved.

#include "SteelCathedralsGameMode.h"
#include "Mech.h"
#include "CrewMember.h"
#include "MapSetupActor.h"
#include "UObject/ConstructorHelpers.h"

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
	// HIERARCHY:
	// World -> Mech (at MechSpawnLocation) -> Interior (at +200,0,+2500 from mech) -> Crew (offsets from interior)
	// ============================================================

	// Interior cockpit is at this offset from mech origin
	const FVector InteriorOffsetFromMech = FVector(200.0f, 0.0f, 2500.0f);

	// Crew spawn positions RELATIVE to mech origin (which includes interior offset)

	// Player 1: Front-left, near Pilot station
	CrewSpawnLocations.Add(InteriorOffsetFromMech + FVector(-300.0f, -200.0f, 100.0f));
	CrewSpawnRotations.Add(FRotator(0.0f, 0.0f, 0.0f)); // Forward

	// Player 2: Front-right, near Gunner station
	CrewSpawnLocations.Add(InteriorOffsetFromMech + FVector(-300.0f, 200.0f, 100.0f));
	CrewSpawnRotations.Add(FRotator(0.0f, 0.0f, 0.0f)); // Forward

	// Player 3: Center cockpit, navigation position
	CrewSpawnLocations.Add(InteriorOffsetFromMech + FVector(0.0f, 0.0f, 100.0f));
	CrewSpawnRotations.Add(FRotator(0.0f, 0.0f, 0.0f)); // Forward

	UE_LOG(LogTemp, Log, TEXT("Initialized 3 crew spawn locations RELATIVE to mech origin"));
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
	// HIERARCHY: World -> Mech -> Crew (relative to mech)

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

	// Get mech's actual world position (may differ from MechSpawnLocation if moved)
	FVector MechWorldLocation = MechActor->GetActorLocation();
	FRotator MechWorldRotation = MechActor->GetActorRotation();

	UE_LOG(LogTemp, Warning, TEXT("Positioning crew relative to Mech at world location: %s"),
		*MechWorldLocation.ToString());

	int32 PlayerIndex = 0;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{
			// Get next crew spawn (RELATIVE to mech origin)
			FTransform RelativeToMech = GetNextCrewSpawnTransform();

			// Transform from mech-space to world-space
			FVector WorldLocation = MechWorldLocation + RelativeToMech.GetLocation();
			FRotator WorldRotation = MechWorldRotation + RelativeToMech.GetRotation().Rotator();

			PC->GetPawn()->SetActorLocationAndRotation(WorldLocation, WorldRotation);

			UE_LOG(LogTemp, Warning, TEXT("Crew %d: Relative=%s -> World=%s"),
				PlayerIndex + 1,
				*RelativeToMech.GetLocation().ToString(),
				*WorldLocation.ToString());

			PlayerIndex++;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("✅ Positioned %d crew member(s) in cockpit"), PlayerIndex);
}
