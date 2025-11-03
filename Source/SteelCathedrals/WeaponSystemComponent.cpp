// Copyright Epic Games, Inc. All Rights Reserved.

#include "WeaponSystemComponent.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

UWeaponSystemComponent::UWeaponSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UWeaponSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialize default loadout if none configured
	if (EquippedWeapons.Num() == 0)
	{
		InitializeDefaultLoadout();
	}

	// Initialize reload timers
	ReloadTimers.SetNum(EquippedWeapons.Num());
	for (int32 i = 0; i < ReloadTimers.Num(); i++)
	{
		ReloadTimers[i] = 0.0f;
	}

	UE_LOG(LogTemp, Warning, TEXT("WeaponSystemComponent initialized with %d weapons"), EquippedWeapons.Num());
}

void UWeaponSystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Update reload timers
	UpdateReloadTimers(DeltaTime);

	// Handle continuous fire
	if (bIsFiring)
	{
		FireAccumulator += DeltaTime;

		if (CanFireCurrentWeapon())
		{
			FWeaponData& CurrentWeapon = EquippedWeapons[CurrentWeaponIndex];

			if (FireAccumulator >= CurrentWeapon.FireRate)
			{
				ExecuteWeaponFire(CurrentWeaponIndex);
				FireAccumulator = 0.0f;
			}
		}
	}
}

void UWeaponSystemComponent::InitializeDefaultLoadout()
{
	UE_LOG(LogTemp, Warning, TEXT("Initializing default weapon loadout..."));

	// Right Arm: Autocannon (Ballistic)
	FWeaponData Autocannon;
	Autocannon.WeaponName = "30mm Autocannon";
	Autocannon.WeaponType = EWeaponType::Ballistic;
	Autocannon.MountLocation = EWeaponMount::RightArm;
	Autocannon.Damage = 50.0f;
	Autocannon.FireRate = 0.2f;
	Autocannon.Range = 200000.0f; // 2km
	Autocannon.HeatPerShot = 3.0f;
	Autocannon.MaxAmmo = 300;
	Autocannon.CurrentAmmo = 300;
	Autocannon.ReloadTime = 4.0f;

	// Left Shoulder: Missile Launcher
	FWeaponData Missiles;
	Missiles.WeaponName = "SRM-6 Launcher";
	Missiles.WeaponType = EWeaponType::Missile;
	Missiles.MountLocation = EWeaponMount::LeftShoulder;
	Missiles.Damage = 150.0f;
	Missiles.FireRate = 1.0f;
	Missiles.Range = 300000.0f; // 3km
	Missiles.HeatPerShot = 15.0f;
	Missiles.MaxAmmo = 24;
	Missiles.CurrentAmmo = 24;
	Missiles.ReloadTime = 6.0f;

	// Torso: Laser (Energy)
	FWeaponData Laser;
	Laser.WeaponName = "Medium Laser";
	Laser.WeaponType = EWeaponType::Energy;
	Laser.MountLocation = EWeaponMount::Torso;
	Laser.Damage = 80.0f;
	Laser.FireRate = 0.5f;
	Laser.Range = 250000.0f; // 2.5km
	Laser.HeatPerShot = 20.0f;
	Laser.MaxAmmo = -1; // Infinite ammo
	Laser.CurrentAmmo = -1;
	Laser.ReloadTime = 0.0f;

	EquippedWeapons.Add(Autocannon);
	EquippedWeapons.Add(Missiles);
	EquippedWeapons.Add(Laser);

	UE_LOG(LogTemp, Warning, TEXT("✅ Default loadout: 3 weapons equipped"));
}

void UWeaponSystemComponent::FireWeapon()
{
	bIsFiring = true;
	FireAccumulator = 999.0f; // Fire immediately
}

void UWeaponSystemComponent::FireWeaponByIndex(int32 WeaponIndex)
{
	if (!EquippedWeapons.IsValidIndex(WeaponIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid weapon index: %d"), WeaponIndex);
		return;
	}

	if (CanFireWeapon(WeaponIndex))
	{
		ExecuteWeaponFire(WeaponIndex);
	}
}

void UWeaponSystemComponent::FireAllWeapons()
{
	UE_LOG(LogTemp, Warning, TEXT("ALPHA STRIKE! Firing all weapons!"));

	for (int32 i = 0; i < EquippedWeapons.Num(); i++)
	{
		if (CanFireWeapon(i))
		{
			ExecuteWeaponFire(i);
		}
	}
}

void UWeaponSystemComponent::StopFiring()
{
	bIsFiring = false;
	FireAccumulator = 0.0f;
}

void UWeaponSystemComponent::ExecuteWeaponFire(int32 WeaponIndex)
{
	if (!EquippedWeapons.IsValidIndex(WeaponIndex))
		return;

	FWeaponData& Weapon = EquippedWeapons[WeaponIndex];

	// Check ammo (unless infinite)
	if (Weapon.CurrentAmmo == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s out of ammo!"), *Weapon.WeaponName);
		OnWeaponEmpty.Broadcast(WeaponIndex);
		return;
	}

	// Perform weapon trace
	FHitResult Hit;
	bool bHitSomething = PerformWeaponTrace(Weapon, Hit);

	// Consume ammo (unless infinite)
	if (Weapon.MaxAmmo > 0)
	{
		Weapon.CurrentAmmo = FMath::Max(0, Weapon.CurrentAmmo - 1);
	}

	// Update last fire time
	Weapon.LastFireTime = GetWorld()->GetTimeSeconds();

	// Apply effects
	ApplyWeaponEffects(WeaponIndex, Hit);

	// Broadcast event
	OnWeaponFired.Broadcast(WeaponIndex, Weapon);

	// Generate heat (will be handled by reactor system)
	// ReactorSystem->AddHeat(Weapon.HeatPerShot);

	UE_LOG(LogTemp, Log, TEXT("Fired %s | Ammo: %d/%d | Hit: %s"),
	       *Weapon.WeaponName,
	       Weapon.CurrentAmmo,
	       Weapon.MaxAmmo,
	       bHitSomething ? *Hit.GetActor()->GetName() : TEXT("Miss"));
}

bool UWeaponSystemComponent::PerformWeaponTrace(const FWeaponData& Weapon, FHitResult& OutHit)
{
	if (!GetOwner())
		return false;

	// Get muzzle location and aim direction
	FVector MuzzleLocation = GetWeaponMuzzleLocation(Weapon.MountLocation);
	FVector AimDirection = GetWeaponAimDirection();
	FVector EndLocation = MuzzleLocation + (AimDirection * Weapon.Range);

	// Perform line trace
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());
	QueryParams.bTraceComplex = false;

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		OutHit,
		MuzzleLocation,
		EndLocation,
		ECC_Visibility,
		QueryParams
	);

	// Debug visualization
	DrawDebugLine(
		GetWorld(),
		MuzzleLocation,
		bHit ? OutHit.Location : EndLocation,
		bHit ? FColor::Red : FColor::Green,
		false,
		0.5f,
		0,
		2.0f
	);

	return bHit;
}

FVector UWeaponSystemComponent::GetWeaponMuzzleLocation(EWeaponMount Mount) const
{
	if (!GetOwner())
		return FVector::ZeroVector;

	FVector OwnerLocation = GetOwner()->GetActorLocation();
	FVector Offset = FVector::ZeroVector;

	// Calculate offset based on mount location (mech-specific)
	switch (Mount)
	{
	case EWeaponMount::RightArm:
		Offset = FVector(500.0f, 300.0f, 2000.0f); // Right side, mid-height
		break;
	case EWeaponMount::LeftArm:
		Offset = FVector(500.0f, -300.0f, 2000.0f); // Left side, mid-height
		break;
	case EWeaponMount::RightShoulder:
		Offset = FVector(0.0f, 400.0f, 3000.0f); // Right shoulder, high
		break;
	case EWeaponMount::LeftShoulder:
		Offset = FVector(0.0f, -400.0f, 3000.0f); // Left shoulder, high
		break;
	case EWeaponMount::Torso:
		Offset = FVector(500.0f, 0.0f, 2500.0f); // Center torso
		break;
	}

	return OwnerLocation + Offset;
}

FVector UWeaponSystemComponent::GetWeaponAimDirection() const
{
	if (!GetOwner())
		return FVector::ForwardVector;

	// For now, just use actor forward direction
	// TODO: Use camera/crosshair direction when pilot station implemented
	return GetOwner()->GetActorForwardVector();
}

void UWeaponSystemComponent::ApplyWeaponEffects(int32 WeaponIndex, const FHitResult& Hit)
{
	if (!EquippedWeapons.IsValidIndex(WeaponIndex))
		return;

	const FWeaponData& Weapon = EquippedWeapons[WeaponIndex];

	// Apply damage if hit something
	if (Hit.GetActor())
	{
		// Apply damage through Unreal's damage system
		UGameplayStatics::ApplyDamage(Hit.GetActor(), Weapon.Damage, GetOwner()->GetInstigatorController(), GetOwner(), UDamageType::StaticClass());

		OnWeaponHit.Broadcast(WeaponIndex, Hit.GetActor(), Weapon.Damage);

		UE_LOG(LogTemp, Log, TEXT("Hit %s for %.1f damage"), *Hit.GetActor()->GetName(), Weapon.Damage);
	}

	// TODO: Spawn VFX at muzzle and hit location
	// TODO: Play weapon sound
	// TODO: Add screen shake for pilot
}

bool UWeaponSystemComponent::CanFireCurrentWeapon() const
{
	return CanFireWeapon(CurrentWeaponIndex);
}

bool UWeaponSystemComponent::CanFireWeapon(int32 WeaponIndex) const
{
	if (!EquippedWeapons.IsValidIndex(WeaponIndex))
		return false;

	const FWeaponData& Weapon = EquippedWeapons[WeaponIndex];

	// Check if reloading
	if (Weapon.bIsReloading)
		return false;

	// Check ammo (unless infinite)
	if (Weapon.MaxAmmo > 0 && Weapon.CurrentAmmo <= 0)
		return false;

	// Check fire rate cooldown
	float TimeSinceLastFire = GetWorld()->GetTimeSeconds() - Weapon.LastFireTime;
	if (TimeSinceLastFire < Weapon.FireRate)
		return false;

	return true;
}

FWeaponData UWeaponSystemComponent::GetCurrentWeapon() const
{
	if (EquippedWeapons.IsValidIndex(CurrentWeaponIndex))
	{
		return EquippedWeapons[CurrentWeaponIndex];
	}

	return FWeaponData();
}

void UWeaponSystemComponent::SelectWeapon(int32 WeaponIndex)
{
	if (!EquippedWeapons.IsValidIndex(WeaponIndex))
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid weapon index: %d"), WeaponIndex);
		return;
	}

	int32 OldIndex = CurrentWeaponIndex;
	CurrentWeaponIndex = WeaponIndex;

	OnWeaponChanged.Broadcast(OldIndex, CurrentWeaponIndex);

	UE_LOG(LogTemp, Log, TEXT("Weapon changed: %s"), *EquippedWeapons[CurrentWeaponIndex].WeaponName);
}

void UWeaponSystemComponent::NextWeapon()
{
	int32 NewIndex = (CurrentWeaponIndex + 1) % EquippedWeapons.Num();
	SelectWeapon(NewIndex);
}

void UWeaponSystemComponent::PreviousWeapon()
{
	int32 NewIndex = (CurrentWeaponIndex - 1 + EquippedWeapons.Num()) % EquippedWeapons.Num();
	SelectWeapon(NewIndex);
}

void UWeaponSystemComponent::ReloadWeapon()
{
	ReloadWeaponByIndex(CurrentWeaponIndex);
}

void UWeaponSystemComponent::ReloadWeaponByIndex(int32 WeaponIndex)
{
	if (!EquippedWeapons.IsValidIndex(WeaponIndex))
		return;

	FWeaponData& Weapon = EquippedWeapons[WeaponIndex];

	// Don't reload if already full or infinite ammo
	if (Weapon.MaxAmmo < 0 || Weapon.CurrentAmmo >= Weapon.MaxAmmo)
	{
		UE_LOG(LogTemp, Log, TEXT("%s doesn't need reloading"), *Weapon.WeaponName);
		return;
	}

	// Don't reload if already reloading
	if (Weapon.bIsReloading)
	{
		UE_LOG(LogTemp, Log, TEXT("%s is already reloading"), *Weapon.WeaponName);
		return;
	}

	// Start reload
	Weapon.bIsReloading = true;
	ReloadTimers[WeaponIndex] = Weapon.ReloadTime;

	OnWeaponReloadStart.Broadcast(WeaponIndex, Weapon.ReloadTime);

	UE_LOG(LogTemp, Warning, TEXT("Reloading %s (%.1fs)..."), *Weapon.WeaponName, Weapon.ReloadTime);
}

void UWeaponSystemComponent::UpdateReloadTimers(float DeltaTime)
{
	for (int32 i = 0; i < EquippedWeapons.Num(); i++)
	{
		if (EquippedWeapons[i].bIsReloading)
		{
			ReloadTimers[i] -= DeltaTime;

			if (ReloadTimers[i] <= 0.0f)
			{
				// Reload complete
				EquippedWeapons[i].bIsReloading = false;
				EquippedWeapons[i].CurrentAmmo = EquippedWeapons[i].MaxAmmo;
				ReloadTimers[i] = 0.0f;

				OnWeaponReloadComplete.Broadcast(i);

				UE_LOG(LogTemp, Warning, TEXT("✅ %s reload complete!"), *EquippedWeapons[i].WeaponName);
			}
		}
	}
}

float UWeaponSystemComponent::GetTotalAmmoPercentage() const
{
	if (EquippedWeapons.Num() == 0)
		return 0.0f;

	float TotalAmmo = 0.0f;
	float TotalMaxAmmo = 0.0f;

	for (const FWeaponData& Weapon : EquippedWeapons)
	{
		// Skip infinite ammo weapons
		if (Weapon.MaxAmmo < 0)
			continue;

		TotalAmmo += Weapon.CurrentAmmo;
		TotalMaxAmmo += Weapon.MaxAmmo;
	}

	if (TotalMaxAmmo == 0.0f)
		return 1.0f; // All weapons have infinite ammo

	return TotalAmmo / TotalMaxAmmo;
}
