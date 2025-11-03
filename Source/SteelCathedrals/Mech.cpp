// Copyright Epic Games, Inc. All Rights Reserved.

#include "Mech.h"
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
#include "DrawDebugHelpers.h"

// Sets default values
AMech::AMech()
{
	// Set this pawn to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	// Enable replication (for 3-player multiplayer)
	bReplicates = true;
	SetReplicatingMovement(true);

	// Create root component
	MechRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MechRoot"));
	RootComponent = MechRoot;

	// Create capsule component for collision (HULL)
	MechCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("MechCapsule"));
	MechCapsule->SetupAttachment(MechRoot);
	MechCapsule->SetCapsuleSize(400.0f, 2100.0f); // Radius: 4m, Half-height: 21m
	MechCapsule->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MechCapsule->SetCollisionResponseToAllChannels(ECR_Block);

	// Create skeletal mesh component for mech exterior (HULL)
	MechMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MechMesh"));
	MechMesh->SetupAttachment(MechRoot);
	MechMesh->SetRelativeLocation(FVector(0, 0, 0));

	// Create movement component (HULL)
	MechMovement = CreateDefaultSubobject<UMechMovementComponent>(TEXT("MechMovement"));

	// Create reactor system component (HULL)
	ReactorSystem = CreateDefaultSubobject<UReactorSystemComponent>(TEXT("ReactorSystem"));

	// Create weapon system component (HULL)
	WeaponSystem = CreateDefaultSubobject<UWeaponSystemComponent>(TEXT("WeaponSystem"));

	// Create damage management component (HULL - bridges to INTERIOR hazards)
	DamageManagement = CreateDefaultSubobject<UDamageManagementComponent>(TEXT("DamageManagement"));

	// Create cockpit camera (INTERIOR - first-person view)
	CockpitCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CockpitCamera"));
	CockpitCamera->SetupAttachment(MechRoot);
	// Position camera in cockpit - center, at pilot eye level
	CockpitCamera->SetRelativeLocation(FVector(200.0f, 0.0f, 2600.0f)); // Front of cockpit, head height
	CockpitCamera->SetRelativeRotation(FRotator(0.0f, 0.0f, 0.0f)); // Look forward
	CockpitCamera->bUsePawnControlRotation = true; // Allow looking around

	// Create procedural geometry component (HULL)
	ProceduralGeometry = CreateDefaultSubobject<UProceduralMechGeometry>(TEXT("ProceduralGeometry"));

	UE_LOG(LogTemp, Warning, TEXT("Mech created (supports 3 crew members)"));
}

// Called when the game starts or when spawned
void AMech::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("🤖 Mech BeginPlay - Location: %s, Height: %f cm"),
		*GetActorLocation().ToString(), MechHeight);

	// Spawn INTERIOR environment inside the mech head/cockpit
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
			UE_LOG(LogTemp, Warning, TEXT("INTERIOR environment spawned (cockpit, stations, hazards)"));
		}

		// Draw debug visualization to see where mech is
		DrawDebugSphere(GetWorld(), GetActorLocation(), 500.0f, 16, FColor::Cyan, false, 30.0f, 0, 10.0f);
		DrawDebugSphere(GetWorld(), GetActorLocation() + FVector(0, 0, 2000), 300.0f, 16, FColor::Red, false, 30.0f, 0, 10.0f);
		UE_LOG(LogTemp, Warning, TEXT("🔵 Debug spheres drawn at mech location (cyan) and head (red)"));
	}
}

// Called every frame
void AMech::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AMech::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (PlayerInputComponent)
	{
		// Bind movement axes
		PlayerInputComponent->BindAxis("MoveForward", this, &AMech::InputMoveForward);
		PlayerInputComponent->BindAxis("MoveRight", this, &AMech::InputMoveRight);
		PlayerInputComponent->BindAxis("Turn", this, &AMech::InputTurn);
		PlayerInputComponent->BindAxis("LookUp", this, &AMech::InputLookUp);

		// Bind actions
		PlayerInputComponent->BindAction("Brace", IE_Pressed, this, &AMech::HandleBracePressed);
		PlayerInputComponent->BindAction("Boost", IE_Pressed, this, &AMech::HandleBoostPressed);
		PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AMech::InputFireWeapon);

		UE_LOG(LogTemp, Warning, TEXT("✅ Mech input bindings configured"));
	}
}

void AMech::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMech, CurrentMechState);
}

float AMech::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (DamageManagement)
	{
		// HULL damage: Apply damage to location based on hit
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

void AMech::HandleMoveInput(FVector2D MovementVector)
{
	if (MechMovement)
	{
		MechMovement->ProcessMoveInput(MovementVector);
	}
}

void AMech::HandleTurnInput(float TurnValue)
{
	if (MechMovement)
	{
		MechMovement->ProcessTurnInput(TurnValue);
	}
}

void AMech::HandleBracePressed()
{
	if (MechMovement)
	{
		bIsBraced = !bIsBraced;
		MechMovement->SetBraced(bIsBraced);

		UE_LOG(LogTemp, Warning, TEXT("Brace toggled: %s"), bIsBraced ? TEXT("ON") : TEXT("OFF"));
	}
}

void AMech::HandleBoostPressed()
{
	if (MechMovement && MechMovement->CanBoost())
	{
		// Boost in forward direction
		FVector BoostDirection = GetActorForwardVector();
		MechMovement->ActivateBoost(BoostDirection);
	}
}

float AMech::GetHeatPercentage() const
{
	if (ReactorSystem)
	{
		return ReactorSystem->GetHeatPercentage();
	}
	return 0.0f;
}

float AMech::GetPowerPercentage() const
{
	if (ReactorSystem)
	{
		return ReactorSystem->GetPowerPercentage();
	}
	return 100.0f;
}

bool AMech::IsSystemOperational(const FString& SystemName) const
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

void AMech::InputMoveForward(float Value)
{
	ForwardInputValue = Value;
	UpdateMovementInput();
}

void AMech::InputMoveRight(float Value)
{
	RightInputValue = Value;
	UpdateMovementInput();
}

void AMech::UpdateMovementInput()
{
	FVector2D MovementVector(ForwardInputValue, RightInputValue);
	HandleMoveInput(MovementVector);
}

void AMech::InputTurn(float Value)
{
	HandleTurnInput(Value);
}

void AMech::InputLookUp(float Value)
{
	// Add pitch input to controller
	if (Controller)
	{
		AddControllerPitchInput(Value);
	}
}

void AMech::InputFireWeapon()
{
	if (WeaponSystem)
	{
		WeaponSystem->FireWeapon();
		UE_LOG(LogTemp, Warning, TEXT("🔫 Weapon fired"));
	}
}
