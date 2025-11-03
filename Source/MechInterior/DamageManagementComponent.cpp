// Copyright Epic Games, Inc. All Rights Reserved.

#include "DamageManagementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

UDamageManagementComponent::UDamageManagementComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	// Default interior hazard spawn points (relative to mech)
	InteriorHazardSpawnPoints.Add(FVector(200, 0, 2500));     // Cockpit
	InteriorHazardSpawnPoints.Add(FVector(0, -200, 2000));    // Left corridor
	InteriorHazardSpawnPoints.Add(FVector(0, 200, 2000));     // Right corridor
	InteriorHazardSpawnPoints.Add(FVector(-100, 0, 1500));    // Reactor room
	InteriorHazardSpawnPoints.Add(FVector(100, -150, 2300));  // Left equipment bay
	InteriorHazardSpawnPoints.Add(FVector(100, 150, 2300));   // Right equipment bay
}

void UDamageManagementComponent::BeginPlay()
{
	Super::BeginPlay();

	InitializeSections();
	InitializeSubsystems();

	UE_LOG(LogTemp, Warning, TEXT("DamageManagementComponent initialized - Total integrity: %.0f"), TotalMaxIntegrity);
}

void UDamageManagementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Check for critical state transitions
	float TotalPercent = GetTotalIntegrityPercent();

	if (TotalPercent < 0.25f && !bWasCritical)
	{
		bWasCritical = true;
		OnMechCritical.Broadcast(TotalPercent);
		UE_LOG(LogTemp, Error, TEXT("🚨 MECH CRITICAL - Hull integrity: %.1f%%"), TotalPercent * 100.0f);
	}
	else if (TotalPercent >= 0.25f && bWasCritical)
	{
		bWasCritical = false;
	}

	if (TotalPercent <= 0.0f)
	{
		OnMechDestroyed.Broadcast();
		UE_LOG(LogTemp, Error, TEXT("💀 MECH DESTROYED"));
	}
}

void UDamageManagementComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UDamageManagementComponent, Sections);
	DOREPLIFETIME(UDamageManagementComponent, Subsystems);
}

// ============================================================
// Initialization
// ============================================================

void UDamageManagementComponent::InitializeSections()
{
	// Head - 500 integrity
	FMechSectionData Head;
	Head.Section = EMechSection::Head;
	Head.MaxIntegrity = 500.0f;
	Head.CurrentIntegrity = 500.0f;
	Head.AffectedSubsystems.Add(ESubsystemType::Sensors);
	Head.AffectedSubsystems.Add(ESubsystemType::Communications);
	Sections.Add(Head);

	// Torso - 2000 integrity (main body)
	FMechSectionData Torso;
	Torso.Section = EMechSection::Torso;
	Torso.MaxIntegrity = 2000.0f;
	Torso.CurrentIntegrity = 2000.0f;
	Torso.AffectedSubsystems.Add(ESubsystemType::LifeSupport);
	Torso.AffectedSubsystems.Add(ESubsystemType::CoolantPump);
	Torso.AffectedSubsystems.Add(ESubsystemType::PowerDistribution);
	Sections.Add(Torso);

	// Left Arm - 800 integrity
	FMechSectionData LeftArm;
	LeftArm.Section = EMechSection::LeftArm;
	LeftArm.MaxIntegrity = 800.0f;
	LeftArm.CurrentIntegrity = 800.0f;
	LeftArm.AffectedSubsystems.Add(ESubsystemType::Actuators);
	Sections.Add(LeftArm);

	// Right Arm - 800 integrity
	FMechSectionData RightArm;
	RightArm.Section = EMechSection::RightArm;
	RightArm.MaxIntegrity = 800.0f;
	RightArm.CurrentIntegrity = 800.0f;
	RightArm.AffectedSubsystems.Add(ESubsystemType::Actuators);
	Sections.Add(RightArm);

	// Left Leg - 900 integrity
	FMechSectionData LeftLeg;
	LeftLeg.Section = EMechSection::LeftLeg;
	LeftLeg.MaxIntegrity = 900.0f;
	LeftLeg.CurrentIntegrity = 900.0f;
	LeftLeg.AffectedSubsystems.Add(ESubsystemType::Stabilizers);
	Sections.Add(LeftLeg);

	// Right Leg - 900 integrity
	FMechSectionData RightLeg;
	RightLeg.Section = EMechSection::RightLeg;
	RightLeg.MaxIntegrity = 900.0f;
	RightLeg.CurrentIntegrity = 900.0f;
	RightLeg.AffectedSubsystems.Add(ESubsystemType::Stabilizers);
	Sections.Add(RightLeg);

	UE_LOG(LogTemp, Log, TEXT("Initialized %d mech sections"), Sections.Num());
}

void UDamageManagementComponent::InitializeSubsystems()
{
	// Sensors
	FSubsystemData Sensors;
	Sensors.Type = ESubsystemType::Sensors;
	Sensors.SubsystemName = "Sensor Array";
	Sensors.LinkedSection = EMechSection::Head;
	Subsystems.Add(Sensors);

	// Life Support
	FSubsystemData LifeSupport;
	LifeSupport.Type = ESubsystemType::LifeSupport;
	LifeSupport.SubsystemName = "Life Support";
	LifeSupport.LinkedSection = EMechSection::Torso;
	Subsystems.Add(LifeSupport);

	// Communications
	FSubsystemData Comms;
	Comms.Type = ESubsystemType::Communications;
	Comms.SubsystemName = "Communications";
	Comms.LinkedSection = EMechSection::Head;
	Subsystems.Add(Comms);

	// Stabilizers
	FSubsystemData Stabilizers;
	Stabilizers.Type = ESubsystemType::Stabilizers;
	Stabilizers.SubsystemName = "Gyro Stabilizers";
	Stabilizers.LinkedSection = EMechSection::Torso;
	Subsystems.Add(Stabilizers);

	// Actuators
	FSubsystemData Actuators;
	Actuators.Type = ESubsystemType::Actuators;
	Actuators.SubsystemName = "Limb Actuators";
	Actuators.LinkedSection = EMechSection::Torso;
	Subsystems.Add(Actuators);

	// Coolant Pump
	FSubsystemData CoolantPump;
	CoolantPump.Type = ESubsystemType::CoolantPump;
	CoolantPump.SubsystemName = "Coolant Pump";
	CoolantPump.LinkedSection = EMechSection::Torso;
	Subsystems.Add(CoolantPump);

	// Power Distribution
	FSubsystemData PowerDist;
	PowerDist.Type = ESubsystemType::PowerDistribution;
	PowerDist.SubsystemName = "Power Distribution";
	PowerDist.LinkedSection = EMechSection::Torso;
	Subsystems.Add(PowerDist);

	UE_LOG(LogTemp, Log, TEXT("Initialized %d subsystems"), Subsystems.Num());
}

// ============================================================
// Damage Application
// ============================================================

void UDamageManagementComponent::ApplyDamage(EMechSection Section, float DamageAmount, EDamageType DamageType, FVector HitLocation)
{
	if (DamageAmount <= 0.0f)
		return;

	FMechSectionData* SectionData = Sections.FindByPredicate([Section](const FMechSectionData& Data) { return Data.Section == Section; });
	if (!SectionData)
		return;

	// Already destroyed, no further damage
	if (SectionData->bIsDestroyed)
		return;

	// Apply damage
	float OldIntegrity = SectionData->CurrentIntegrity;
	SectionData->CurrentIntegrity = FMath::Max(0.0f, SectionData->CurrentIntegrity - DamageAmount);
	float NewPercent = SectionData->GetIntegrityPercent();

	UE_LOG(LogTemp, Warning, TEXT("💥 %s section damaged: %.0f damage, %.1f%% integrity remaining"),
	       *UEnum::GetValueAsString(Section), DamageAmount, NewPercent * 100.0f);

	// Broadcast damage event
	OnSectionDamaged.Broadcast(Section, DamageAmount, NewPercent);

	// Check for critical state
	if (SectionData->IsCritical() && OldIntegrity >= SectionData->MaxIntegrity * 0.25f)
	{
		OnSectionCritical.Broadcast(Section);
		UE_LOG(LogTemp, Error, TEXT("⚠️ %s section CRITICAL"), *UEnum::GetValueAsString(Section));
	}

	// Check for destruction
	if (SectionData->CurrentIntegrity <= 0.0f)
	{
		HandleSectionDestruction(Section);
	}

	// Possibly spawn interior hazard
	if (ShouldSpawnHazard(DamageAmount, Section))
	{
		FVector InteriorLocation = GetInteriorSpawnPointForSection(Section);
		SpawnRandomHazard(InteriorLocation, DamageType);
	}

	// Damage to linked subsystems
	for (ESubsystemType SubsystemType : SectionData->AffectedSubsystems)
	{
		if (NewPercent < 0.5f) // Section below 50% damages subsystems
		{
			HandleSubsystemFailure(SubsystemType, Section);
		}
	}
}

void UDamageManagementComponent::ApplyDamageAtLocation(float DamageAmount, EDamageType DamageType, FVector WorldHitLocation)
{
	EMechSection ClosestSection = GetSectionAtLocation(WorldHitLocation);
	ApplyDamage(ClosestSection, DamageAmount, DamageType, WorldHitLocation);
}

void UDamageManagementComponent::RepairSection(EMechSection Section, float RepairAmount)
{
	FMechSectionData* SectionData = Sections.FindByPredicate([Section](const FMechSectionData& Data) { return Data.Section == Section; });
	if (!SectionData)
		return;

	if (SectionData->bIsDestroyed)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot repair destroyed section: %s"), *UEnum::GetValueAsString(Section));
		return;
	}

	float OldIntegrity = SectionData->CurrentIntegrity;
	SectionData->CurrentIntegrity = FMath::Min(SectionData->MaxIntegrity, SectionData->CurrentIntegrity + RepairAmount);

	UE_LOG(LogTemp, Log, TEXT("🔧 Repaired %s: +%.0f integrity (%.1f%%)"),
	       *UEnum::GetValueAsString(Section), RepairAmount, SectionData->GetIntegrityPercent() * 100.0f);
}

void UDamageManagementComponent::FullRepair()
{
	for (FMechSectionData& Section : Sections)
	{
		Section.CurrentIntegrity = Section.MaxIntegrity;
		Section.bIsDestroyed = false;
	}

	for (FSubsystemData& Subsystem : Subsystems)
	{
		Subsystem.bIsOperational = true;
		Subsystem.EfficiencyPercent = 100.0f;
	}

	UE_LOG(LogTemp, Warning, TEXT("✅ Full repair complete - all systems nominal"));
}

// ============================================================
// Subsystem Management
// ============================================================

void UDamageManagementComponent::DisableSubsystem(ESubsystemType Subsystem, float EfficiencyLoss)
{
	FSubsystemData* SubsystemData = Subsystems.FindByPredicate([Subsystem](const FSubsystemData& Data) { return Data.Type == Subsystem; });
	if (!SubsystemData)
		return;

	SubsystemData->EfficiencyPercent = FMath::Max(0.0f, SubsystemData->EfficiencyPercent - EfficiencyLoss);

	if (SubsystemData->EfficiencyPercent <= 0.0f)
	{
		SubsystemData->bIsOperational = false;
		UE_LOG(LogTemp, Error, TEXT("❌ SUBSYSTEM FAILURE: %s"), *SubsystemData->SubsystemName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("⚠️ %s efficiency reduced to %.1f%%"),
		       *SubsystemData->SubsystemName, SubsystemData->EfficiencyPercent);
	}

	OnSubsystemFailed.Broadcast(Subsystem, SubsystemData->EfficiencyPercent);
}

void UDamageManagementComponent::RepairSubsystem(ESubsystemType Subsystem, float EfficiencyGain)
{
	FSubsystemData* SubsystemData = Subsystems.FindByPredicate([Subsystem](const FSubsystemData& Data) { return Data.Type == Subsystem; });
	if (!SubsystemData)
		return;

	SubsystemData->EfficiencyPercent = FMath::Min(100.0f, SubsystemData->EfficiencyPercent + EfficiencyGain);

	if (SubsystemData->EfficiencyPercent > 0.0f)
	{
		SubsystemData->bIsOperational = true;
	}

	UE_LOG(LogTemp, Log, TEXT("🔧 %s repaired to %.1f%% efficiency"),
	       *SubsystemData->SubsystemName, SubsystemData->EfficiencyPercent);

	OnSubsystemRepaired.Broadcast(Subsystem, SubsystemData->EfficiencyPercent);
}

bool UDamageManagementComponent::IsSubsystemOperational(ESubsystemType Subsystem) const
{
	const FSubsystemData* SubsystemData = Subsystems.FindByPredicate([Subsystem](const FSubsystemData& Data) { return Data.Type == Subsystem; });
	if (!SubsystemData)
		return false;

	return SubsystemData->bIsOperational;
}

float UDamageManagementComponent::GetSubsystemEfficiency(ESubsystemType Subsystem) const
{
	const FSubsystemData* SubsystemData = Subsystems.FindByPredicate([Subsystem](const FSubsystemData& Data) { return Data.Type == Subsystem; });
	if (!SubsystemData)
		return 0.0f;

	return SubsystemData->EfficiencyPercent;
}

// ============================================================
// Query Functions
// ============================================================

float UDamageManagementComponent::GetSectionIntegrity(EMechSection Section) const
{
	const FMechSectionData* SectionData = Sections.FindByPredicate([Section](const FMechSectionData& Data) { return Data.Section == Section; });
	if (!SectionData)
		return 0.0f;

	return SectionData->GetIntegrityPercent();
}

float UDamageManagementComponent::GetTotalIntegrityPercent() const
{
	float TotalCurrent = 0.0f;
	float TotalMax = 0.0f;

	for (const FMechSectionData& Section : Sections)
	{
		TotalCurrent += Section.CurrentIntegrity;
		TotalMax += Section.MaxIntegrity;
	}

	if (TotalMax <= 0.0f)
		return 0.0f;

	return TotalCurrent / TotalMax;
}

bool UDamageManagementComponent::IsSectionDestroyed(EMechSection Section) const
{
	const FMechSectionData* SectionData = Sections.FindByPredicate([Section](const FMechSectionData& Data) { return Data.Section == Section; });
	if (!SectionData)
		return true;

	return SectionData->bIsDestroyed;
}

bool UDamageManagementComponent::IsSectionCritical(EMechSection Section) const
{
	const FMechSectionData* SectionData = Sections.FindByPredicate([Section](const FMechSectionData& Data) { return Data.Section == Section; });
	if (!SectionData)
		return false;

	return SectionData->IsCritical();
}

EMechSection UDamageManagementComponent::GetSectionAtLocation(FVector WorldLocation) const
{
	AActor* Owner = GetOwner();
	if (!Owner)
		return EMechSection::Torso;

	FVector LocalLocation = Owner->GetActorTransform().InverseTransformPosition(WorldLocation);

	// Simple spatial mapping based on Z height and Y position
	float Z = LocalLocation.Z;
	float Y = LocalLocation.Y;

	// Head: Top of mech (Z > 3000)
	if (Z > 3000.0f)
		return EMechSection::Head;

	// Torso: Middle section
	if (Z > 1000.0f && Z <= 3000.0f)
		return EMechSection::Torso;

	// Arms: Upper body, left/right based on Y
	if (Z > 1500.0f)
	{
		if (Y < -300.0f)
			return EMechSection::LeftArm;
		if (Y > 300.0f)
			return EMechSection::RightArm;
	}

	// Legs: Lower body, left/right based on Y
	if (Y < 0.0f)
		return EMechSection::LeftLeg;
	else
		return EMechSection::RightLeg;
}

// ============================================================
// Hazard Spawning
// ============================================================

void UDamageManagementComponent::SpawnFireHazard(FVector InteriorLocation)
{
	// TODO: Spawn AFireHazardActor when implemented
	UE_LOG(LogTemp, Warning, TEXT("🔥 Fire hazard spawned at: %s"), *InteriorLocation.ToString());
	OnHazardSpawned.Broadcast("Fire", InteriorLocation);
}

void UDamageManagementComponent::SpawnCoolantLeak(FVector InteriorLocation)
{
	// TODO: Spawn ACoolantLeakActor when implemented
	UE_LOG(LogTemp, Warning, TEXT("💧 Coolant leak spawned at: %s"), *InteriorLocation.ToString());
	OnHazardSpawned.Broadcast("CoolantLeak", InteriorLocation);
}

void UDamageManagementComponent::SpawnElectricalArc(FVector InteriorLocation)
{
	// TODO: Spawn AElectricalArcActor when implemented
	UE_LOG(LogTemp, Warning, TEXT("⚡ Electrical arc spawned at: %s"), *InteriorLocation.ToString());
	OnHazardSpawned.Broadcast("ElectricalArc", InteriorLocation);
}

// ============================================================
// Internal Functions
// ============================================================

void UDamageManagementComponent::HandleSectionDestruction(EMechSection Section)
{
	FMechSectionData* SectionData = Sections.FindByPredicate([Section](const FMechSectionData& Data) { return Data.Section == Section; });
	if (!SectionData)
		return;

	SectionData->bIsDestroyed = true;
	SectionData->CurrentIntegrity = 0.0f;

	UE_LOG(LogTemp, Error, TEXT("💀 %s DESTROYED"), *UEnum::GetValueAsString(Section));
	OnSectionDestroyed.Broadcast(Section);

	// Destroy all linked subsystems
	for (ESubsystemType SubsystemType : SectionData->AffectedSubsystems)
	{
		DisableSubsystem(SubsystemType, 100.0f); // Complete failure
	}
}

void UDamageManagementComponent::HandleSubsystemFailure(ESubsystemType Subsystem, EMechSection DamagedSection)
{
	FSubsystemData* SubsystemData = Subsystems.FindByPredicate([Subsystem](const FSubsystemData& Data) { return Data.Type == Subsystem; });
	if (!SubsystemData)
		return;

	// Only damage if linked to this section
	if (SubsystemData->LinkedSection != DamagedSection)
		return;

	// Random efficiency loss (10-30%)
	float EfficiencyLoss = FMath::RandRange(10.0f, 30.0f);
	DisableSubsystem(Subsystem, EfficiencyLoss);
}

bool UDamageManagementComponent::ShouldSpawnHazard(float DamageAmount, EMechSection Section) const
{
	if (DamageAmount < MinDamageForHazard)
		return false;

	// Roll for hazard spawn
	float Roll = FMath::FRand();
	return Roll < HazardSpawnChance;
}

FVector UDamageManagementComponent::GetInteriorSpawnPointForSection(EMechSection Section) const
{
	if (InteriorHazardSpawnPoints.Num() == 0)
		return FVector::ZeroVector;

	// Map sections to preferred spawn point indices
	int32 PreferredIndex = 0;
	switch (Section)
	{
	case EMechSection::Head:
		PreferredIndex = 0; // Cockpit
		break;
	case EMechSection::Torso:
		PreferredIndex = 3; // Reactor room
		break;
	case EMechSection::LeftArm:
		PreferredIndex = 4; // Left equipment bay
		break;
	case EMechSection::RightArm:
		PreferredIndex = 5; // Right equipment bay
		break;
	case EMechSection::LeftLeg:
		PreferredIndex = 1; // Left corridor
		break;
	case EMechSection::RightLeg:
		PreferredIndex = 2; // Right corridor
		break;
	}

	// Clamp to valid range
	PreferredIndex = FMath::Clamp(PreferredIndex, 0, InteriorHazardSpawnPoints.Num() - 1);

	// Convert relative position to world position
	AActor* Owner = GetOwner();
	if (!Owner)
		return InteriorHazardSpawnPoints[PreferredIndex];

	return Owner->GetActorTransform().TransformPosition(InteriorHazardSpawnPoints[PreferredIndex]);
}

void UDamageManagementComponent::SpawnRandomHazard(FVector Location, EDamageType DamageType)
{
	// Select hazard based on damage type
	switch (DamageType)
	{
	case EDamageType::Energy:
		SpawnFireHazard(Location);
		break;

	case EDamageType::Ballistic:
		// 50/50 between fire and electrical
		if (FMath::RandBool())
			SpawnFireHazard(Location);
		else
			SpawnElectricalArc(Location);
		break;

	case EDamageType::Explosive:
		// All three types possible
		{
			int32 Random = FMath::RandRange(0, 2);
			if (Random == 0)
				SpawnFireHazard(Location);
			else if (Random == 1)
				SpawnCoolantLeak(Location);
			else
				SpawnElectricalArc(Location);
		}
		break;

	case EDamageType::Melee:
		SpawnCoolantLeak(Location); // Puncture coolant lines
		break;

	default:
		SpawnFireHazard(Location);
		break;
	}
}
