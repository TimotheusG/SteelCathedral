// Copyright Epic Games, Inc. All Rights Reserved.

#include "ElectricalArcActor.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

AElectricalArcActor::AElectricalArcActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Create root
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Create arc particles
	ArcParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ArcParticles"));
	ArcParticles->SetupAttachment(RootComponent);
	ArcParticles->bAutoActivate = true;

	// Create arc audio
	ArcAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("ArcAudio"));
	ArcAudio->SetupAttachment(RootComponent);
	ArcAudio->bAutoActivate = true;

	// Create flickering light
	ArcLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("ArcLight"));
	ArcLight->SetupAttachment(RootComponent);
	ArcLight->SetLightColor(FLinearColor(0.5f, 0.7f, 1.0f)); // Blue-white electrical color
	ArcLight->SetIntensity(2000.0f);
	ArcLight->SetAttenuationRadius(500.0f);
	ArcLight->SetCastShadows(false); // Performance

	// Create damage sphere
	DamageSphere = CreateDefaultSubobject<USphereComponent>(TEXT("DamageSphere"));
	DamageSphere->SetupAttachment(RootComponent);
	DamageSphere->SetSphereRadius(150.0f);
	DamageSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DamageSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	DamageSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Bind overlap events
	DamageSphere->OnComponentBeginOverlap.AddDynamic(this, &AElectricalArcActor::OnDamageSphereBeginOverlap);
	DamageSphere->OnComponentEndOverlap.AddDynamic(this, &AElectricalArcActor::OnDamageSphereEndOverlap);
}

void AElectricalArcActor::BeginPlay()
{
	Super::BeginPlay();

	CurrentIntensity = InitialIntensity;
	DamageSphere->SetSphereRadius(DamageRadius);

	UpdateArcVisuals();
	UpdateArcAudio();

	OnArcStarted.Broadcast(this);
	UE_LOG(LogTemp, Warning, TEXT("⚡ Electrical arc started at %s - Intensity: %s"),
	       *GetActorLocation().ToString(),
	       *UEnum::GetValueAsString(CurrentIntensity));
}

void AElectricalArcActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ArcTime += DeltaTime;
	TimeSinceLastPulse += DeltaTime;

	// Update flickering light
	UpdateArcLight(DeltaTime);

	// Trigger arc pulses
	if (TimeSinceLastPulse >= ArcPulseInterval)
	{
		TriggerArcPulse();
		TimeSinceLastPulse = 0.0f;
	}

	// Handle repair progress
	if (bIsBeingRepaired)
	{
		RepairProgress += DeltaTime / RepairDuration;

		if (RepairProgress >= 1.0f)
		{
			// Decrease intensity
			DecreaseIntensity();
			RepairProgress = 0.0f;
			bIsBeingRepaired = false;

			// Check if fully repaired
			if (CurrentIntensity == EArcIntensity::Weak)
			{
				// One more reduction to repair completely
				HandleRepaired();
			}
		}
	}
}

void AElectricalArcActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AElectricalArcActor, CurrentIntensity);
	DOREPLIFETIME(AElectricalArcActor, bIsBeingRepaired);
	DOREPLIFETIME(AElectricalArcActor, RepairProgress);
}

// ============================================================
// Interaction
// ============================================================

void AElectricalArcActor::StartRepairing()
{
	if (!CanBeRepaired())
		return;

	bIsBeingRepaired = true;
	UE_LOG(LogTemp, Log, TEXT("🔧 Starting to repair electrical arc"));
}

void AElectricalArcActor::StopRepairing()
{
	bIsBeingRepaired = false;
	RepairProgress = 0.0f;
	UE_LOG(LogTemp, Log, TEXT("🔧 Stopped repairing electrical arc"));
}

void AElectricalArcActor::ApplyRepair(float DeltaTime)
{
	if (!bIsBeingRepaired)
	{
		StartRepairing();
	}

	// Progress is handled in Tick()
}

void AElectricalArcActor::RepairImmediately()
{
	HandleRepaired();
}

void AElectricalArcActor::IncreaseIntensity()
{
	EArcIntensity OldIntensity = CurrentIntensity;

	switch (CurrentIntensity)
	{
	case EArcIntensity::Weak:
		CurrentIntensity = EArcIntensity::Moderate;
		break;
	case EArcIntensity::Moderate:
		CurrentIntensity = EArcIntensity::Strong;
		break;
	case EArcIntensity::Strong:
		CurrentIntensity = EArcIntensity::Lethal;
		break;
	case EArcIntensity::Lethal:
		// Already max
		return;
	}

	UpdateArcVisuals();
	UpdateArcAudio();
	OnIntensityChanged.Broadcast(OldIntensity, CurrentIntensity);

	UE_LOG(LogTemp, Warning, TEXT("⚡ Arc intensity increased to %s"), *UEnum::GetValueAsString(CurrentIntensity));
}

void AElectricalArcActor::DecreaseIntensity()
{
	EArcIntensity OldIntensity = CurrentIntensity;

	switch (CurrentIntensity)
	{
	case EArcIntensity::Lethal:
		CurrentIntensity = EArcIntensity::Strong;
		break;
	case EArcIntensity::Strong:
		CurrentIntensity = EArcIntensity::Moderate;
		break;
	case EArcIntensity::Moderate:
		CurrentIntensity = EArcIntensity::Weak;
		break;
	case EArcIntensity::Weak:
		// Will be repaired
		HandleRepaired();
		return;
	}

	UpdateArcVisuals();
	UpdateArcAudio();
	OnIntensityChanged.Broadcast(OldIntensity, CurrentIntensity);

	UE_LOG(LogTemp, Log, TEXT("🔧 Arc intensity decreased to %s"), *UEnum::GetValueAsString(CurrentIntensity));
}

// ============================================================
// Query
// ============================================================

float AElectricalArcActor::GetCurrentDamagePerSecond() const
{
	return DamagePerSecond * GetIntensityMultiplier();
}

float AElectricalArcActor::GetCurrentDisruptionChance() const
{
	return SystemDisruptionChance * GetIntensityMultiplier();
}

bool AElectricalArcActor::CanBeRepaired() const
{
	return !bIsBeingRepaired;
}

float AElectricalArcActor::GetIntensityMultiplier() const
{
	switch (CurrentIntensity)
	{
	case EArcIntensity::Weak:
		return 1.0f;
	case EArcIntensity::Moderate:
		return 2.0f;
	case EArcIntensity::Strong:
		return 3.0f;
	case EArcIntensity::Lethal:
		return 4.0f;
	default:
		return 1.0f;
	}
}

// ============================================================
// Internal Functions
// ============================================================

void AElectricalArcActor::UpdateArcVisuals()
{
	if (!ArcParticles)
		return;

	float Multiplier = GetIntensityMultiplier();

	// Scale particle system
	ArcParticles->SetRelativeScale3D(FVector(Multiplier, Multiplier, Multiplier));

	// Adjust spawn rate and intensity
}

void AElectricalArcActor::UpdateArcAudio()
{
	if (!ArcAudio)
		return;

	float Multiplier = GetIntensityMultiplier();

	// Adjust volume based on intensity
	ArcAudio->SetVolumeMultiplier(0.6f * Multiplier);

	// Adjust pitch - more intense = higher frequency
	ArcAudio->SetPitchMultiplier(0.8f + (Multiplier * 0.15f));
}

void AElectricalArcActor::UpdateArcLight(float DeltaTime)
{
	if (!ArcLight)
		return;

	LightFlickerTimer += DeltaTime;

	// Randomize flicker interval
	if (LightFlickerTimer >= LightFlickerInterval)
	{
		LightFlickerInterval = FMath::RandRange(0.05f, 0.2f);
		LightFlickerTimer = 0.0f;

		// Random intensity flicker
		float BaseIntensity = 2000.0f * GetIntensityMultiplier();
		float FlickerAmount = FMath::RandRange(0.5f, 1.5f);
		ArcLight->SetIntensity(BaseIntensity * FlickerAmount);

		// Occasionally turn off briefly
		if (FMath::RandBool() && FMath::FRand() < 0.2f)
		{
			ArcLight->SetVisibility(false);
		}
		else
		{
			ArcLight->SetVisibility(true);
		}
	}
}

void AElectricalArcActor::TriggerArcPulse()
{
	bIsPulsing = true;

	// Apply damage to nearby characters
	ApplyElectricalDamage();

	// Attempt system disruption
	AttemptSystemDisruption();

	// Attempt fire ignition
	if (bCanIgniteFires)
	{
		AttemptFireIgnition();
	}

	OnArcPulse.Broadcast();

	// Visual pulse (brief light intensity spike)
	if (ArcLight)
	{
		float PulseIntensity = 5000.0f * GetIntensityMultiplier();
		ArcLight->SetIntensity(PulseIntensity);
	}

	bIsPulsing = false;
}

void AElectricalArcActor::ApplyElectricalDamage()
{
	float DamageToApply = GetCurrentDamagePerSecond() * ArcPulseInterval;

	for (AActor* Actor : ActorsInRange)
	{
		if (!Actor)
			continue;

		// Apply electrical damage to characters
		ACharacter* Character = Cast<ACharacter>(Actor);
		if (Character)
		{
			UGameplayStatics::ApplyDamage(Character, DamageToApply, nullptr, this, UDamageType::StaticClass());
			OnCharacterShocked.Broadcast(Character);

			UE_LOG(LogTemp, Log, TEXT("⚡ %s shocked for %.1f damage"), *Character->GetName(), DamageToApply);
		}
	}
}

void AElectricalArcActor::AttemptSystemDisruption()
{
	float DisruptionChance = GetCurrentDisruptionChance();
	float Roll = FMath::FRand();

	if (Roll < DisruptionChance)
	{
		// Randomly select a system to disrupt
		TArray<FString> PossibleSystems = {
			"Lighting System",
			"Door Controls",
			"Console Displays",
			"Communications",
			"Sensor Array"
		};

		int32 RandomIndex = FMath::RandRange(0, PossibleSystems.Num() - 1);
		FString DisruptedSystem = PossibleSystems[RandomIndex];

		OnSystemDisrupted.Broadcast(DisruptedSystem);
		UE_LOG(LogTemp, Warning, TEXT("⚡ System disrupted: %s (%.1fs)"), *DisruptedSystem, DisruptionDuration);

		// TODO: Actually disable the system for DisruptionDuration
	}
}

void AElectricalArcActor::AttemptFireIgnition()
{
	float Roll = FMath::FRand();

	if (Roll < FireIgnitionChance)
	{
		// Spawn fire nearby
		FVector FireLocation = GetActorLocation() + FMath::VRand() * 100.0f;

		// TODO: Spawn AFireHazardActor at FireLocation
		UE_LOG(LogTemp, Warning, TEXT("⚡ Electrical arc ignited fire at %s"), *FireLocation.ToString());
	}
}

void AElectricalArcActor::HandleRepaired()
{
	OnArcRepaired.Broadcast(this);
	UE_LOG(LogTemp, Log, TEXT("✅ Electrical arc repaired at %s"), *GetActorLocation().ToString());

	// Destroy actor
	Destroy();
}

// ============================================================
// Overlap Events
// ============================================================

void AElectricalArcActor::OnDamageSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
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
	UE_LOG(LogTemp, Log, TEXT("⚡ %s entered electrical arc range"), *OtherActor->GetName());
}

void AElectricalArcActor::OnDamageSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
		return;

	ActorsInRange.Remove(OtherActor);
	UE_LOG(LogTemp, Log, TEXT("⚡ %s left electrical arc range"), *OtherActor->GetName());
}
