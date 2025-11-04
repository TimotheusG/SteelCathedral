// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "CrewMember.generated.h"

class UCameraComponent;
class UInputComponent;

/**
 * CrewMember - A crew member character that walks around the INTERIOR of the mech
 * - First-person view
 * - Standard WASD movement
 * - Can interact with MechStations to control the HULL (pilot console, weapon console, etc.)
 * - Supports 3-player multiplayer
 */
UCLASS()
class STEELCATHEDRALS_API ACrewMember : public ACharacter
{
	GENERATED_BODY()

public:
	ACrewMember();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// ============================================================
	// Components
	// ============================================================

	/** First-person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UCameraComponent* FirstPersonCamera;

	// ============================================================
	// Input Handlers
	// ============================================================

	/** Handle forward/backward movement */
	void MoveForward(float Value);

	/** Handle right/left movement */
	void MoveRight(float Value);

	/** Handle mouse look horizontal */
	void Turn(float Value);

	/** Handle mouse look vertical */
	void LookUp(float Value);

	// ============================================================
	// Station Interaction (for future implementation)
	// ============================================================

	/** The station this crew member is currently using (nullptr if not at a station) */
	UPROPERTY(BlueprintReadOnly, Category = "Station")
	class AMechStation* CurrentStation;

	/** Try to use/interact with a nearby station */
	UFUNCTION(BlueprintCallable, Category = "Station")
	void InteractWithStation();

	/** Leave current station */
	UFUNCTION(BlueprintCallable, Category = "Station")
	void LeaveStation();

	/** Primary station action pressed (e.g., fire weapons) */
	void HandlePrimaryActionPressed();

	/** Primary station action released */
	void HandlePrimaryActionReleased();
};
