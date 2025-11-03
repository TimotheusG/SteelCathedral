// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerMechPawn.h"
#include "MechMovementComponent.h"
#include "ReactorSystemComponent.h"
#include "WeaponSystemComponent.h"
#include "DamageManagementComponent.h"
#include "ProceduralMechGeometry.h"
#include "ProceduralInteriorGeometry.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/DamageEvents.h"

// Sets default values
APlayerMechPawn::APlayerMechPawn()
{
	// Set this pawn to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	// Enable replication
	bReplicates = true;
	SetReplicatingMovement(true);

	// Create root component
	MechRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MechRoot"));
	RootComponent = MechRoot;

	// Create capsule component for collision
	MechCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("MechCapsule"));
	MechCapsule->SetupAttachment(MechRoot);
	MechCapsule->SetCapsuleSize(400.0f, 2100.0f); // Radius: 4m, Half-height: 21m
	MechCapsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MechCapsule->SetCollisionResponseToAllChannels(ECR_Block);

	// Create skeletal mesh component for mech exterior
	MechMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MechMesh"));
	MechMesh->SetupAttachment(MechRoot);
	MechMesh->SetRelativeLocation(FVector(0, 0, 0));

	// Create movement component
	MechMovement = CreateDefaultSubobject<UMechMovementComponent>(TEXT("MechMovement"));

	// Create reactor system component
	ReactorSystem = CreateDefaultSubobject<UReactorSystemComponent>(TEXT("ReactorSystem"));

	// Create weapon system component
	WeaponSystem = CreateDefaultSubobject<UWeaponSystemComponent>(TEXT("WeaponSystem"));

	// Create damage management component
	DamageManagement = CreateDefaultSubobject<UDamageManagementComponent>(TEXT("DamageManagement"));

	// Create cockpit camera (first-person view)
	CockpitCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CockpitCamera"));
	CockpitCamera->SetupAttachment(MechRoot);
	// Position camera in cockpit - center, at pilot eye level
	CockpitCamera->SetRelativeLocation(FVector(200.0f, 0.0f, 2600.0f)); // Front of cockpit, head height
	CockpitCamera->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f)); // Look forward
	CockpitCamera->bUsePawnControlRotation = true; // Allow looking around

	// Create procedural geometry component
	ProceduralGeometry = CreateDefaultSubobject<UProceduralMechGeometry>(TEXT("ProceduralGeometry"));

	UE_LOG(LogTemp, Warning, TEXT("PlayerMechPawn created"));
}

// Called when the game starts or when spawned
void APlayerMechPawn::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("PlayerMechPawn BeginPlay - Mech Height: %f cm"), MechHeight);

	// Spawn interior environment inside the mech head/cockpit
	if (GetWorld())
	{
		FVector InteriorLocation = FVector(200, 0, 2500); // Inside cockpit
		FRotator InteriorRotation = FRotator::ZeroRotator;
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;

		InteriorEnvironment = GetWorld()->SpawnActor<AProceduralInteriorGeometry>(
			AProceduralInteriorGeometry::StaticClass(),
			InteriorLocation,
			InteriorRotation,
			SpawnParams
		);

		if (InteriorEnvironment)
		{
			InteriorEnvironment->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
			UE_LOG(LogTemp, Warning, TEXT("Interior environment spawned successfully"));
		}
	}
}

// Called every frame
void APlayerMechPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void APlayerMechPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (PlayerInputComponent)
	{
		// Bind movement axes
		PlayerInputComponent->BindAxis("MoveForward", this, &APlayerMechPawn::InputMoveForward);
		PlayerInputComponent->BindAxis("MoveRight", this, &APlayerMechPawn::InputMoveRight);
		PlayerInputComponent->BindAxis("Turn", this, &APlayerMechPawn::InputTurn);
		PlayerInputComponent->BindAxis("LookUp", this, &APlayerMechPawn::InputLookUp);

		// Bind actions
		PlayerInputComponent->BindAction("Brace", IE_Pressed, this, &APlayerMechPawn::HandleBracePressed);
		PlayerInputComponent->BindAction("Boost", IE_Pressed, this, &APlayerMechPawn::HandleBoostPressed);
		PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &APlayerMechPawn::InputFireWeapon);

		UE_LOG(LogTemp, Warning, TEXT("✅ PlayerMechPawn input bindings configured"));
	}
}

void APlayerMechPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlayerMechPawn, CurrentMechState);
}

float APlayerMechPawn::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (DamageManagement)
	{
		// Apply damage to location based on hit
		FVector HitLocation = GetActorLocation();

		// Try to get precise hit location from damage event
		if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
		{
			const FPointDamageEvent* PointDamageEvent = (FPointDamageEvent*)&DamageEvent;
			HitLocation = PointDamageEvent->HitInfo.ImpactPoint;
		}

		DamageManagement->ApplyDamageAtLocation(ActualDamage, EDamageType::Ballistic, HitLocation);

		// Call Blueprint event
		OnMechDamaged(ActualDamage, HitLocation);
	}

	return ActualDamage;
}

void APlayerMechPawn::HandleMoveInput(FVector2D MovementVector)
{
	if (MechMovement)
	{
		MechMovement->ProcessMoveInput(MovementVector);
	}
}

void APlayerMechPawn::HandleTurnInput(float TurnValue)
{
	if (MechMovement)
	{
		MechMovement->ProcessTurnInput(TurnValue);
	}
}

void APlayerMechPawn::HandleBracePressed()
{
	if (MechMovement)
	{
		bIsBraced = !bIsBraced;
		MechMovement->SetBraced(bIsBraced);

		UE_LOG(LogTemp, Warning, TEXT("Brace toggled: %s"), bIsBraced ? TEXT("ON") : TEXT("OFF"));
	}
}

void APlayerMechPawn::HandleBoostPressed()
{
	if (MechMovement && MechMovement->CanBoost())
	{
		// Boost in forward direction
		FVector BoostDirection = GetActorForwardVector();
		MechMovement->ActivateBoost(BoostDirection);
	}
}

float APlayerMechPawn::GetHeatPercentage() const
{
	if (ReactorSystem)
	{
		return ReactorSystem->GetHeatPercentage();
	}
	return 0.0f;
}

float APlayerMechPawn::GetPowerPercentage() const
{
	if (ReactorSystem)
	{
		return ReactorSystem->GetPowerPercentage();
	}
	return 100.0f;
}

bool APlayerMechPawn::IsSystemOperational(const FString& SystemName) const
{
	// Note: This function uses string names for Blueprint compatibility
	// For a more robust implementation, consider mapping system names to ESubsystemType
	// For now, assume all systems operational unless DamageManagement indicates otherwise
	if (DamageManagement)
	{
		// Could map string names to ESubsystemType enum values
		// For basic implementation, return true if total integrity > 0
		return DamageManagement->GetTotalIntegrityPercent() > 0.0f;
	}
	return true;
}

// ============================================================
// Internal Input Handlers
// ============================================================

void APlayerMechPawn::InputMoveForward(float Value)
{
	ForwardInputValue = Value;
	UpdateMovementInput();
}

void APlayerMechPawn::InputMoveRight(float Value)
{
	RightInputValue = Value;
	UpdateMovementInput();
}

void APlayerMechPawn::UpdateMovementInput()
{
	FVector2D MovementVector(ForwardInputValue, RightInputValue);
	HandleMoveInput(MovementVector);
}

void APlayerMechPawn::InputTurn(float Value)
{
	HandleTurnInput(Value);
}

void APlayerMechPawn::InputLookUp(float Value)
{
	// Add pitch input to controller
	if (Controller)
	{
		AddControllerPitchInput(Value);
	}
}

void APlayerMechPawn::InputFireWeapon()
{
	if (WeaponSystem)
	{
		WeaponSystem->FireWeapon();
		UE_LOG(LogTemp, Warning, TEXT("🔫 Weapon fired"));
	}
}
