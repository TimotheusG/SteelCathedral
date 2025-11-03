// Copyright Epic Games, Inc. All Rights Reserved.

#include "CrewMember.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MechStation.h"

ACrewMember::ACrewMember()
{
	PrimaryActorTick.bCanEverTick = true;

	// Configure character capsule (INTERIOR - crew member collision)
	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f); // Standard human size

	// Configure character movement (INTERIOR - walking around)
	GetCharacterMovement()->bOrientRotationToMovement = false; // Don't rotate to movement direction
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bConstrainToPlane = false;
	GetCharacterMovement()->bSnapToPlaneAtStart = false;

	// Movement speeds
	GetCharacterMovement()->MaxWalkSpeed = 600.0f; // Standard walk speed
	GetCharacterMovement()->MaxAcceleration = 2048.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2048.0f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->GravityScale = 1.0f;

	// Configure controller rotation (first-person)
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Create first-person camera
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetCapsuleComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.0f, 0.0f, 64.0f)); // Eye height
	FirstPersonCamera->bUsePawnControlRotation = true; // Camera follows controller rotation

	// No station initially
	CurrentStation = nullptr;

	UE_LOG(LogTemp, Warning, TEXT("CrewMember created (INTERIOR character)"));
}

void ACrewMember::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("CrewMember spawned - Ready to explore INTERIOR"));
}

void ACrewMember::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ACrewMember::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Bind movement axes (WASD)
	PlayerInputComponent->BindAxis("MoveForward", this, &ACrewMember::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ACrewMember::MoveRight);

	// Bind look axes (Mouse)
	PlayerInputComponent->BindAxis("Turn", this, &ACrewMember::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &ACrewMember::LookUp);

	// Bind jump (space bar - inherited from Character)
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	UE_LOG(LogTemp, Warning, TEXT("CrewMember input configured (WASD + Mouse)"));
}

// ============================================================
// Input Handlers
// ============================================================

void ACrewMember::MoveForward(float Value)
{
	if (Controller && Value != 0.0f)
	{
		// Get forward direction
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// Add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ACrewMember::MoveRight(float Value)
{
	if (Controller && Value != 0.0f)
	{
		// Get right direction
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// Add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void ACrewMember::Turn(float Value)
{
	if (Controller && Value != 0.0f)
	{
		AddControllerYawInput(Value);
	}
}

void ACrewMember::LookUp(float Value)
{
	if (Controller && Value != 0.0f)
	{
		AddControllerPitchInput(Value);
	}
}

// ============================================================
// Station Interaction (stub for future implementation)
// ============================================================

void ACrewMember::InteractWithStation()
{
	// TODO: Implement station interaction
	// - Raycast forward to find nearby AMechStation
	// - If found, call station->ActivateStation(this)
	// - Lock player to station position
	// - Switch controls to operate HULL systems
	UE_LOG(LogTemp, Warning, TEXT("Station interaction not yet implemented"));
}

void ACrewMember::LeaveStation()
{
	if (CurrentStation)
	{
		// TODO: Implement leaving station
		// - Call station->DeactivateStation()
		// - Unlock player from station
		// - Restore normal movement controls
		CurrentStation = nullptr;
		UE_LOG(LogTemp, Warning, TEXT("Left station"));
	}
}
