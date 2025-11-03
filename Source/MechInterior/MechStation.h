// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MechStation.generated.h"

class APlayerMechPawn;
class UBoxComponent;

/**
 * Station types in the mech
 */
UENUM(BlueprintType)
enum class EStationType : uint8
{
	Pilot UMETA(DisplayName = "Pilot Station"),
	Gunner UMETA(DisplayName = "Gunner Station"),
	Technician UMETA(DisplayName = "Technician Station"),
	Navigation UMETA(DisplayName = "Navigation Station")
};

/**
 * Interactable station inside the mech
 * Player walks up and presses E to use
 */
UCLASS()
class MECHINTERIOR_API AMechStation : public AActor
{
	GENERATED_BODY()

public:
	AMechStation();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// ============================================================
	// Station Configuration
	// ============================================================

	/** What type of station this is */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	EStationType StationType = EStationType::Pilot;

	/** Display name shown to player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	FString StationName = "Pilot Console";

	/** Interaction prompt text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	FString InteractionPrompt = "Press E to use Pilot Console";

	/** Camera position when seated at station (relative to actor) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	FVector SeatedCameraLocation = FVector(0, 0, 100);

	/** Camera rotation when seated */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	FRotator SeatedCameraRotation = FRotator(0, 0, 0);

	/** Reference to the mech this station controls */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Station")
	APlayerMechPawn* OwningMech = nullptr;

	// ============================================================
	// Station State
	// ============================================================

	/** Is someone currently using this station */
	UPROPERTY(BlueprintReadOnly, Category = "Station")
	bool bIsOccupied = false;

	/** Current player using this station */
	UPROPERTY(BlueprintReadOnly, Category = "Station")
	APawn* CurrentUser = nullptr;

	// ============================================================
	// Interaction
	// ============================================================

	/** Can this station be used right now */
	UFUNCTION(BlueprintPure, Category = "Station")
	bool CanUseStation(APawn* User) const;

	/** Use the station (sit down, take control) */
	UFUNCTION(BlueprintCallable, Category = "Station")
	void UseStation(APawn* User);

	/** Leave the station (stand up, release control) */
	UFUNCTION(BlueprintCallable, Category = "Station")
	void LeaveStation();

	/** Check if player is in range to interact */
	UFUNCTION(BlueprintPure, Category = "Station")
	bool IsPlayerInRange(APawn* Player) const;

	// ============================================================
	// Events
	// ============================================================

	/** Called when player enters interaction range */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerEnterRange, APawn*, Player);
	UPROPERTY(BlueprintAssignable, Category = "Station")
	FOnPlayerEnterRange OnPlayerEnterRange;

	/** Called when player leaves interaction range */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerLeaveRange, APawn*, Player);
	UPROPERTY(BlueprintAssignable, Category = "Station")
	FOnPlayerLeaveRange OnPlayerLeaveRange;

	/** Called when station is used */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStationUsed, APawn*, User, EStationType, Type);
	UPROPERTY(BlueprintAssignable, Category = "Station")
	FOnStationUsed OnStationUsed;

	/** Called when station is exited */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStationExited, APawn*, User, EStationType, Type);
	UPROPERTY(BlueprintAssignable, Category = "Station")
	FOnStationExited OnStationExited;

private:
	// ============================================================
	// Components
	// ============================================================

	/** Interaction trigger volume */
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* InteractionVolume;

	/** Visual mesh for the station */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* StationMesh;

	// ============================================================
	// Internal Functions
	// ============================================================

	/** Handle overlap events */
	UFUNCTION()
	void OnInteractionVolumeBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                                      bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionVolumeEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	/** Store original player camera settings */
	FVector OriginalCameraLocation;
	FRotator OriginalCameraRotation;
	bool bOriginalCameraWasAttached;

	/** Store the original pawn we possessed before using station */
	UPROPERTY()
	APawn* OriginalPawn;

	/** Store the player controller */
	UPROPERTY()
	APlayerController* StationController;
};
