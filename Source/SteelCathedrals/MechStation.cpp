// Copyright Epic Games, Inc. All Rights Reserved.

#include "MechStation.h"
#include "Mech.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Camera/CameraComponent.h"
#include "EngineUtils.h"

AMechStation::AMechStation()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create root
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Create station mesh
	StationMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StationMesh"));
	StationMesh->SetupAttachment(RootComponent);

	// Create interaction volume
	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(RootComponent);
	InteractionVolume->SetBoxExtent(FVector(150.0f, 150.0f, 100.0f));
	InteractionVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Bind overlap events
	InteractionVolume->OnComponentBeginOverlap.AddDynamic(this, &AMechStation::OnInteractionVolumeBeginOverlap);
	InteractionVolume->OnComponentEndOverlap.AddDynamic(this, &AMechStation::OnInteractionVolumeEndOverlap);
}

void AMechStation::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("MechStation '%s' initialized (%s)"),
	       *StationName,
	       *UEnum::GetValueAsString(StationType));

	// Find owning mech if not set
	if (!OwningMech)
	{
		// Look for mech in level
		for (TActorIterator<AMech> It(GetWorld()); It; ++It)
		{
			OwningMech = *It;
			UE_LOG(LogTemp, Log, TEXT("Station auto-linked to mech: %s"), *OwningMech->GetName());
			break;
		}
	}
}

void AMechStation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool AMechStation::CanUseStation(APawn* User) const
{
	if (!User)
		return false;

	if (bIsOccupied)
		return false;

	if (!IsPlayerInRange(User))
		return false;

	return true;
}

void AMechStation::UseStation(APawn* User)
{
	if (!CanUseStation(User))
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot use station: %s"), *StationName);
		return;
	}

	bIsOccupied = true;
	CurrentUser = User;

	UE_LOG(LogTemp, Warning, TEXT("✅ Player using %s station"), *StationName);

	// Get player controller
	APlayerController* PC = Cast<APlayerController>(User->GetController());
	if (!PC)
		return;

	// Disable character movement
	ACharacter* Character = Cast<ACharacter>(User);
	if (Character)
	{
		Character->DisableInput(PC);
		// Store original camera
		if (UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>())
		{
			OriginalCameraLocation = Camera->GetRelativeLocation();
			OriginalCameraRotation = Camera->GetRelativeRotation();
		}
	}

	// Position camera at station
	if (UCameraComponent* Camera = User->FindComponentByClass<UCameraComponent>())
	{
		Camera->SetWorldLocation(GetActorLocation() + SeatedCameraLocation);
		Camera->SetWorldRotation(GetActorRotation() + SeatedCameraRotation);
	}

	// Store controller and original pawn
	StationController = PC;
	OriginalPawn = User;

	// Enable control based on station type
	switch (StationType)
	{
	case EStationType::Pilot:
		UE_LOG(LogTemp, Warning, TEXT("🎮 Pilot controls enabled - WASD to move mech"));
		// Transfer control to the mech
		if (OwningMech)
		{
			PC->Possess(OwningMech);
			UE_LOG(LogTemp, Warning, TEXT("✅ Controller now possessing mech - input routing active"));

			// Position camera at station location (inside mech)
			if (UCameraComponent* MechCamera = OwningMech->FindComponentByClass<UCameraComponent>())
			{
				// Use the mech's cockpit camera
				PC->SetViewTarget(OwningMech);
			}
		}
		break;

	case EStationType::Gunner:
		UE_LOG(LogTemp, Warning, TEXT("🎯 Gunner controls enabled - Mouse to aim, LMB to fire"));
		// TODO: Enable weapon control (will possess mech later)
		break;

	case EStationType::Technician:
		UE_LOG(LogTemp, Warning, TEXT("🔧 Technician console active - Monitor systems"));
		// TODO: Enable repair/reactor control (no possession needed)
		break;

	case EStationType::Navigation:
		UE_LOG(LogTemp, Warning, TEXT("🗺️ Navigation console active"));
		// TODO: Enable map/waypoint control (no possession needed)
		break;
	}

	// Broadcast event
	OnStationUsed.Broadcast(User, StationType);
}

void AMechStation::LeaveStation()
{
	if (!bIsOccupied || !CurrentUser)
		return;

	UE_LOG(LogTemp, Warning, TEXT("Player left %s station"), *StationName);

	// Return control to original pawn if we possessed the mech
	if (StationController && OriginalPawn && StationType == EStationType::Pilot)
	{
		UE_LOG(LogTemp, Warning, TEXT("🔄 Returning control to character pawn"));
		StationController->Possess(OriginalPawn);

		// Reset view target to the character
		StationController->SetViewTarget(OriginalPawn);
	}

	// Re-enable character movement
	if (StationController)
	{
		ACharacter* Character = Cast<ACharacter>(OriginalPawn ? OriginalPawn : CurrentUser);
		if (Character)
		{
			Character->EnableInput(StationController);

			// Restore original camera
			if (UCameraComponent* Camera = Character->FindComponentByClass<UCameraComponent>())
			{
				Camera->SetRelativeLocation(OriginalCameraLocation);
				Camera->SetRelativeRotation(OriginalCameraRotation);
			}
		}
	}

	// Broadcast event
	OnStationExited.Broadcast(CurrentUser, StationType);

	// Clear state
	APawn* PreviousUser = CurrentUser;
	CurrentUser = nullptr;
	bIsOccupied = false;
	OriginalPawn = nullptr;
	StationController = nullptr;
}

bool AMechStation::IsPlayerInRange(APawn* Player) const
{
	if (!Player || !InteractionVolume)
		return false;

	TArray<AActor*> OverlappingActors;
	InteractionVolume->GetOverlappingActors(OverlappingActors, APawn::StaticClass());

	return OverlappingActors.Contains(Player);
}

void AMechStation::OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                                     bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* Player = Cast<APawn>(OtherActor);
	if (!Player)
		return;

	if (!Player->IsPlayerControlled())
		return;

	UE_LOG(LogTemp, Log, TEXT("Player in range of %s - %s"), *StationName, *InteractionPrompt);

	OnPlayerEnterRange.Broadcast(Player);

	// TODO: Show UI prompt "Press E to use Pilot Console"
}

void AMechStation::OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* Player = Cast<APawn>(OtherActor);
	if (!Player)
		return;

	if (!Player->IsPlayerControlled())
		return;

	UE_LOG(LogTemp, Log, TEXT("Player left range of %s"), *StationName);

	OnPlayerLeaveRange.Broadcast(Player);

	// TODO: Hide UI prompt
}
