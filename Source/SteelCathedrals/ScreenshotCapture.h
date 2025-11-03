// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Camera/CameraComponent.h"
#include "ScreenshotCapture.generated.h"

/**
 * Actor that automatically captures screenshots from multiple angles
 * Press a key to trigger capture sequence
 */
UCLASS()
class STEELCATHEDRALS_API AScreenshotCapture : public AActor
{
	GENERATED_BODY()

public:
	AScreenshotCapture();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	/** Camera used for capturing screenshots */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Screenshot")
	UCameraComponent* CaptureCamera;

	/** Target actor to capture (usually the mech) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screenshot")
	AActor* TargetActor;

	/** Distance from target for screenshots */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screenshot")
	float CaptureDistance = 3000.0f;

	/** Height offset for side views */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Screenshot")
	float CaptureHeight = 2000.0f;

	/** Start capture sequence */
	UFUNCTION(BlueprintCallable, Category = "Screenshot")
	void StartCaptureSequence();

	/** Capture screenshot from specific angle */
	UFUNCTION(BlueprintCallable, Category = "Screenshot")
	void CaptureFromAngle(FVector RelativePosition, FRotator Rotation, FString Filename);

private:
	bool bCapturing = false;
	int32 CurrentCaptureIndex = 0;
	float CaptureTimer = 0.0f;
	float CaptureDelay = 0.5f;  // Delay between captures

	struct FCaptureInfo
	{
		FVector RelativePosition;
		FRotator Rotation;
		FString Filename;
		FString Description;
	};

	TArray<FCaptureInfo> CaptureQueue;

	void SetupCaptureQueue();
	void ProcessNextCapture();
	void TakeScreenshot(const FString& Filename);
};
