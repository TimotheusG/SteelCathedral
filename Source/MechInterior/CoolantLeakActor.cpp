// Copyright Epic Games, Inc. All Rights Reserved.

#include "CoolantLeakActor.h"
#include "ReactorSystemComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Components/DecalComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

ACoolantLeakActor::ACoolantLeakActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	// Create root
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	// Create vapor particles
	VaporParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("VaporParticles"));
	VaporParticles->SetupAttachment(RootComponent);
	VaporParticles->bAutoActivate = true;

	// Create leak audio
	LeakAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("LeakAudio"));
	LeakAudio->SetupAttachment(RootComponent);
	LeakAudio->bAutoActivate = true;

	// Create puddle decal
	PuddleDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("PuddleDecal"));
	PuddleDecal->SetupAttachment(RootComponent);
	PuddleDecal->DecalSize = FVector(200.0f, 200.0f, 200.0f);
	// Rotate to project onto floor
	PuddleDecal->SetRelativeRotation(FRotator(-90.0f, 0.0f, 0.0f));

	// Create vapor damage sphere
	VaporSphere = CreateDefaultSubobject<USphereComponent>(TEXT("VaporSphere"));
	VaporSphere->SetupAttachment(RootComponent);
	VaporSphere->SetSphereRadius(150.0f);
	VaporSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	VaporSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	VaporSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// Bind overlap events
	VaporSphere->OnComponentBeginOverlap.AddDynamic(this, &ACoolantLeakActor::OnVaporSphereBeginOverlap);
	VaporSphere->OnComponentEndOverlap.AddDynamic(this, &ACoolantLeakActor::OnVaporSphereEndOverlap);
}

void ACoolantLeakActor::BeginPlay()
{
	Super::BeginPlay();

	CurrentSeverity = InitialSeverity;
	VaporSphere->SetSphereRadius(VaporRadius);

	UpdateLeakVisuals();
	UpdateLeakAudio();
	UpdatePuddle();

	// Find reactor system
	AActor* OwningActor = GetOwner();
	if (OwningActor)
	{
		CachedReactor = OwningActor->FindComponentByClass<UReactorSystemComponent>();
	}

	OnLeakStarted.Broadcast(this);
	UE_LOG(LogTemp, Warning, TEXT("💧 Coolant leak started at %s - Severity: %s"),
	       *GetActorLocation().ToString(),
	       *UEnum::GetValueAsString(CurrentSeverity));
}

void ACoolantLeakActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	LeakTime += DeltaTime;

	// Apply heat to reactor
	ApplyHeatToReactor(DeltaTime);

	// Apply damage to characters in vapor
	ApplyVaporDamage(DeltaTime);

	// Check for worsening
	if (bCanWorsen && !bHasWorsened && LeakTime > TimeBeforeWorsening)
	{
		CheckForWorsening(DeltaTime);
	}

	// Handle sealing progress
	if (bIsBeingSealed)
	{
		SealProgress += DeltaTime / SealDuration;

		if (SealProgress >= 1.0f)
		{
			// Decrease severity
			ImproveLeak();
			SealProgress = 0.0f;
			bIsBeingSealed = false;

			// Check if fully sealed
			if (CurrentSeverity == ECoolantLeakSeverity::Minor)
			{
				// One more reduction to seal completely
				HandleSealed();
			}
		}
	}
}

void ACoolantLeakActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACoolantLeakActor, CurrentSeverity);
	DOREPLIFETIME(ACoolantLeakActor, bIsBeingSealed);
	DOREPLIFETIME(ACoolantLeakActor, SealProgress);
}

// ============================================================
// Interaction
// ============================================================

void ACoolantLeakActor::StartSealing()
{
	if (!CanBeSealed())
		return;

	bIsBeingSealed = true;
	UE_LOG(LogTemp, Log, TEXT("🔧 Starting to seal coolant leak"));
}

void ACoolantLeakActor::StopSealing()
{
	bIsBeingSealed = false;
	SealProgress = 0.0f;
	UE_LOG(LogTemp, Log, TEXT("🔧 Stopped sealing coolant leak"));
}

void ACoolantLeakActor::ApplySealant(float DeltaTime)
{
	if (!bIsBeingSealed)
	{
		StartSealing();
	}

	// Progress is handled in Tick()
}

void ACoolantLeakActor::SealImmediately()
{
	HandleSealed();
}

void ACoolantLeakActor::WorsenLeak()
{
	ECoolantLeakSeverity OldSeverity = CurrentSeverity;

	switch (CurrentSeverity)
	{
	case ECoolantLeakSeverity::Minor:
		CurrentSeverity = ECoolantLeakSeverity::Moderate;
		break;
	case ECoolantLeakSeverity::Moderate:
		CurrentSeverity = ECoolantLeakSeverity::Major;
		break;
	case ECoolantLeakSeverity::Major:
		CurrentSeverity = ECoolantLeakSeverity::Critical;
		break;
	case ECoolantLeakSeverity::Critical:
		// Already max
		return;
	}

	UpdateLeakVisuals();
	UpdateLeakAudio();
	UpdatePuddle();
	OnSeverityChanged.Broadcast(OldSeverity, CurrentSeverity);

	UE_LOG(LogTemp, Error, TEXT("💧 Coolant leak worsened to %s"), *UEnum::GetValueAsString(CurrentSeverity));
}

void ACoolantLeakActor::ImproveLeak()
{
	ECoolantLeakSeverity OldSeverity = CurrentSeverity;

	switch (CurrentSeverity)
	{
	case ECoolantLeakSeverity::Critical:
		CurrentSeverity = ECoolantLeakSeverity::Major;
		break;
	case ECoolantLeakSeverity::Major:
		CurrentSeverity = ECoolantLeakSeverity::Moderate;
		break;
	case ECoolantLeakSeverity::Moderate:
		CurrentSeverity = ECoolantLeakSeverity::Minor;
		break;
	case ECoolantLeakSeverity::Minor:
		// Will be sealed
		HandleSealed();
		return;
	}

	UpdateLeakVisuals();
	UpdateLeakAudio();
	UpdatePuddle();
	OnSeverityChanged.Broadcast(OldSeverity, CurrentSeverity);

	UE_LOG(LogTemp, Log, TEXT("🔧 Coolant leak improved to %s"), *UEnum::GetValueAsString(CurrentSeverity));
}

// ============================================================
// Query
// ============================================================

float ACoolantLeakActor::GetCurrentHeatIncrease() const
{
	return HeatIncreasePerSecond * GetSeverityMultiplier();
}

float ACoolantLeakActor::GetCurrentVaporDamage() const
{
	return VaporDamagePerSecond * GetSeverityMultiplier();
}

bool ACoolantLeakActor::CanBeSealed() const
{
	return !bIsBeingSealed;
}

float ACoolantLeakActor::GetSeverityMultiplier() const
{
	switch (CurrentSeverity)
	{
	case ECoolantLeakSeverity::Minor:
		return 1.0f;
	case ECoolantLeakSeverity::Moderate:
		return 2.0f;
	case ECoolantLeakSeverity::Major:
		return 3.0f;
	case ECoolantLeakSeverity::Critical:
		return 4.0f;
	default:
		return 1.0f;
	}
}

// ============================================================
// Internal Functions
// ============================================================

void ACoolantLeakActor::UpdateLeakVisuals()
{
	if (!VaporParticles)
		return;

	float Multiplier = GetSeverityMultiplier();

	// Scale particle system
	VaporParticles->SetRelativeScale3D(FVector(Multiplier, Multiplier, Multiplier * 0.5f));

	// Adjust spray direction based on severity
	// Higher severity = more pressurized spray
}

void ACoolantLeakActor::UpdateLeakAudio()
{
	if (!LeakAudio)
		return;

	float Multiplier = GetSeverityMultiplier();

	// Adjust volume based on severity
	LeakAudio->SetVolumeMultiplier(0.4f * Multiplier);

	// Adjust pitch - higher severity = higher pressure = higher pitch
	LeakAudio->SetPitchMultiplier(0.8f + (Multiplier * 0.2f));
}

void ACoolantLeakActor::UpdatePuddle()
{
	if (!PuddleDecal)
		return;

	float Multiplier = GetSeverityMultiplier();

	// Larger puddle for worse leaks
	float Size = PuddleSize * Multiplier;
	PuddleDecal->DecalSize = FVector(Size, Size, Size);
}

void ACoolantLeakActor::ApplyHeatToReactor(float DeltaTime)
{
	if (!CachedReactor)
		return;

	float HeatToAdd = GetCurrentHeatIncrease() * DeltaTime;
	CachedReactor->AddHeat(HeatToAdd);
}

void ACoolantLeakActor::ApplyVaporDamage(float DeltaTime)
{
	AccumulatedDamageTime += DeltaTime;

	// Apply damage every 1 second
	if (AccumulatedDamageTime >= 1.0f)
	{
		float DamageToApply = GetCurrentVaporDamage() * AccumulatedDamageTime;
		AccumulatedDamageTime = 0.0f;

		for (AActor* Actor : ActorsInVapor)
		{
			if (!Actor)
				continue;

			// Apply cold damage to characters
			ACharacter* Character = Cast<ACharacter>(Actor);
			if (Character)
			{
				UGameplayStatics::ApplyDamage(Character, DamageToApply, nullptr, this, UDamageType::StaticClass());
				OnCharacterInVapor.Broadcast(Character);
			}
		}
	}
}

void ACoolantLeakActor::CheckForWorsening(float DeltaTime)
{
	// Leak has been active for too long without repair
	WorsenLeak();
	bHasWorsened = true;
}

void ACoolantLeakActor::HandleSealed()
{
	OnLeakSealed.Broadcast(this);
	UE_LOG(LogTemp, Log, TEXT("✅ Coolant leak sealed at %s"), *GetActorLocation().ToString());

	// Destroy actor
	Destroy();
}

// ============================================================
// Overlap Events
// ============================================================

void ACoolantLeakActor::OnVaporSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                                                    bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
		return;

	// Only care about characters
	ACharacter* Character = Cast<ACharacter>(OtherActor);
	if (!Character)
		return;

	ActorsInVapor.AddUnique(OtherActor);
	UE_LOG(LogTemp, Log, TEXT("💧 %s entered coolant vapor cloud"), *OtherActor->GetName());
}

void ACoolantLeakActor::OnVaporSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                  UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor)
		return;

	ActorsInVapor.Remove(OtherActor);
	UE_LOG(LogTemp, Log, TEXT("💧 %s left coolant vapor cloud"), *OtherActor->GetName());
}
