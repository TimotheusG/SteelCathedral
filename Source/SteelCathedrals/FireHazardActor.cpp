// Copyright Epic Games, Inc. All Rights Reserved.

#include "FireHazardActor.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AFireHazardActor::AFireHazardActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Create root
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Create fire particles
	FireParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FireParticles"));
	FireParticles->SetupAttachment(RootComponent);
	FireParticles->bAutoActivate = true;

	// Create fire audio
	FireAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("FireAudio"));
	FireAudio->SetupAttachment(RootComponent);
	FireAudio->bAutoActivate = true;

	// Create damage sphere
	DamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DamageSphere"));
	DamageSphere->SetupAttachment(RootComponent);
	DamageSphere->SetSphereRadius(200.0f);
	DamageSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Bind overlap events
	DamageSphere->OnComponentBeginOverlap.AddDynamic(this, &AFireHazardActor::OnDamageSphereBeginOverlap);
	DamageSphere->OnComponentEndOverlap.AddDynamic(this, &AFireHazardActor::OnDamageSphereEndOverlap);
}

void AFireHazardActor::BeginPlay()
{
	Super::BeginPlay();

	CurrentIntensity = InitialIntensity;
	DamageSphere->SetSphereRadius(DamageRadius);

	UpdateFireVisuals();
	UpdateFireAudio();

	OnFireStarted.Broadcast(this);
	UE_LOG(LogTemp, Warning, TEXT("🔥 Fire hazard started at %s - Intensity: %s"),
	       *GetActorLocation().ToString(),
	       *UEnum::GetValueAsString(CurrentIntensity));
}

void AFireHazardActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	BurnTime += DeltaTime;

	// Apply damage to nearby actors
	ApplyFireDamage(DeltaTime);

	// Check for spreading
	if (bCanSpread && !bHasSpread && BurnTime > TimeBeforeSpreading)
	{
		CheckForSpreading(DeltaTime);
	}

	// Handle extinguishing progress
	if (bIsBeingExtinguished)
	{
		ExtinguishProgress += DeltaTime / ExtinguishDuration;

		if (ExtinguishProgress >= 1.0f)
		{
			// Decrease intensity
			DecreaseIntensity();
			ExtinguishProgress = 0.0f;
			bIsBeingExtinguished = false;

			// Check if fully extinguished
			if (CurrentIntensity == EFireIntensity::Small)
			{
				// One more reduction to extinguish
				HandleExtinguished();
			}
		}
	}
}

void AFireHazardActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFireHazardActor, CurrentIntensity);
	DOREPLIFETIME(AFireHazardActor, bIsBeingExtinguished);
	DOREPLIFETIME(AFireHazardActor, ExtinguishProgress);
}

// ============================================================
// Interaction
// ============================================================

void AFireHazardActor::StartExtinguishing()
{
	if (!CanBeExtinguished())
		return;

	bIsBeingExtinguished = true;
	UE_LOG(LogTemp, Log, TEXT("🧯 Starting to extinguish fire"));
}

void AFireHazardActor::StopExtinguishing()
{
	bIsBeingExtinguished = false;
	ExtinguishProgress = 0.0f;
	UE_LOG(LogTemp, Log, TEXT("🧯 Stopped extinguishing fire"));
}

void AFireHazardActor::ApplyExtinguisher(float DeltaTime)
{
	if (!bIsBeingExtinguished)
	{
		StartExtinguishing();
	}

	// Progress is handled in Tick()
}

void AFireHazardActor::ExtinguishImmediately()
{
	HandleExtinguished();
}

void AFireHazardActor::IncreaseIntensity()
{
	EFireIntensity OldIntensity = CurrentIntensity;

	switch (CurrentIntensity)
	{
	case EFireIntensity::Small:
		CurrentIntensity = EFireIntensity::Medium;
		break;
	case EFireIntensity::Medium:
		CurrentIntensity = EFireIntensity::Large;
		break;
	case EFireIntensity::Large:
		CurrentIntensity = EFireIntensity::Inferno;
		break;
	case EFireIntensity::Inferno:
		// Already max
		return;
	}

	UpdateFireVisuals();
	UpdateFireAudio();
	OnIntensityChanged.Broadcast(OldIntensity, CurrentIntensity);

	UE_LOG(LogTemp, Warning, TEXT("🔥 Fire intensity increased to %s"), *UEnum::GetValueAsString(CurrentIntensity));
}

void AFireHazardActor::DecreaseIntensity()
{
	EFireIntensity OldIntensity = CurrentIntensity;

	switch (CurrentIntensity)
	{
	case EFireIntensity::Inferno:
		CurrentIntensity = EFireIntensity::Large;
		break;
	case EFireIntensity::Large:
		CurrentIntensity = EFireIntensity::Medium;
		break;
	case EFireIntensity::Medium:
		CurrentIntensity = EFireIntensity::Small;
		break;
	case EFireIntensity::Small:
		// Will be extinguished
		HandleExtinguished();
		return;
	}

	UpdateFireVisuals();
	UpdateFireAudio();
	OnIntensityChanged.Broadcast(OldIntensity, CurrentIntensity);

	UE_LOG(LogTemp, Log, TEXT("🧯 Fire intensity decreased to %s"), *UEnum::GetValueAsString(CurrentIntensity));
}

// ============================================================
// Query
// ============================================================

float AFireHazardActor::GetCurrentDamagePerSecond() const
{
	return DamagePerSecond * GetIntensityMultiplier();
}

float AFireHazardActor::GetCurrentHeatGeneration() const
{
	return HeatGenerationPerSecond * GetIntensityMultiplier();
}

bool AFireHazardActor::CanBeExtinguished() const
{
	return !bIsBeingExtinguished;
}

float AFireHazardActor::GetIntensityMultiplier() const
{
	switch (CurrentIntensity)
	{
	case EFireIntensity::Small:
		return 1.0f;
	case EFireIntensity::Medium:
		return 2.0f;
	case EFireIntensity::Large:
		return 3.0f;
	case EFireIntensity::Inferno:
		return 4.0f;
	default:
		return 1.0f;
	}
}

// ============================================================
// Internal Functions
// ============================================================

void AFireHazardActor::UpdateFireVisuals()
{
	if (!FireParticles)
		return;

	float Multiplier = GetIntensityMultiplier();

	// Scale particle system
	FireParticles->SetRelativeScale3D(FVector(Multiplier, Multiplier, Multiplier));

	// Adjust spawn rate (placeholder - would need actual particle system parameter)
	// FireParticles->SetFloatParameter(FName("SpawnRate"), 50.0f * Multiplier);
}

void AFireHazardActor::UpdateFireAudio()
{
	if (!FireAudio)
		return;

	float Multiplier = GetIntensityMultiplier();

	// Adjust volume based on intensity
	FireAudio->SetVolumeMultiplier(0.5f * Multiplier);

	// Adjust pitch slightly
	FireAudio->SetPitchMultiplier(0.9f + (Multiplier * 0.1f));
}

void AFireHazardActor::ApplyFireDamage(float DeltaTime)
{
	AccumulatedDamageTime += DeltaTime;

	// Apply damage every 0.5 seconds
	if (AccumulatedDamageTime >= 0.5f)
	{
		float DamageToApply = GetCurrentDamagePerSecond() * AccumulatedDamageTime;
		AccumulatedDamageTime = 0.0f;

		for (AActor* Actor : ActorsInRange)
		{
			if (!Actor)
				continue;

			// Apply damage to characters
			ACharacter* Character = Cast<ACharacter>(Actor);
			if (Character)
			{
				UGameplayStatics::ApplyDamage(Character, DamageToApply, nullptr, this, UDamageType::StaticClass());
				OnCharacterBurned.Broadcast(Character);
			}
		}
	}

	// TODO: Apply heat to reactor system if in range
}

void AFireHazardActor::CheckForSpreading(float DeltaTime)
{
	// Roll for spread chance
	float Roll = FMath::FRand();
	float SpreadChance = SpreadChancePerSecond * DeltaTime;

	if (Roll < SpreadChance)
	{
		// Find random spread location
		FVector RandomDirection = FMath::VRand();
		RandomDirection.Z = 0.0f; // Keep on same level
		RandomDirection.Normalize();

		float SpreadDistance = FMath::RandRange(100.0f, MaxSpreadDistance);
		FVector SpreadLocation = GetActorLocation() + (RandomDirection * SpreadDistance);

		SpreadFire(SpreadLocation);
		bHasSpread = true; // Only spread once
	}
}

void AFireHazardActor::SpreadFire(FVector SpreadLocation)
{
	if (!GetWorld())
		return;

	// Spawn new fire
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AFireHazardActor* NewFire = GetWorld()->SpawnActor<AFireHazardActor>(
		AFireHazardActor::StaticClass(),
		SpreadLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);

	if (NewFire)
	{
		NewFire->ParentFire = this;
		NewFire->InitialIntensity = EFireIntensity::Small; // New fires start small
		NewFire->bCanSpread = true; // Can continue spreading

		OnFireSpread.Broadcast(this, NewFire);
		UE_LOG(LogTemp, Warning, TEXT("🔥 Fire spread from %s to %s"),
		       *GetActorLocation().ToString(),
		       *SpreadLocation.ToString());
	}
}

void AFireHazardActor::HandleExtinguished()
{
	OnFireExtinguished.Broadcast(this);
	UE_LOG(LogTemp, Log, TEXT("✅ Fire extinguished at %s"), *GetActorLocation().ToString());

	// Destroy actor
	Destroy();
}

// ============================================================
// Overlap Events
// ============================================================

void AFireHazardActor::OnDamageSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                                    bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
		return;

	// Only care about characters
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character)
		return;

	ActorsInRange.AddUnique(OtherActor);
	UE_LOG(LogTemp, Log, TEXT("🔥 %s entered fire damage range"), *OtherActor->GetName());
}

void AFireHazardActor::OnDamageSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
		return;

	ActorsInRange.Remove(OtherActor);
	UE_LOG(LogTemp, Log, TEXT("🔥 %s left fire damage range"), *OtherActor->GetName());
}
