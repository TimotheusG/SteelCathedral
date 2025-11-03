// Copyright Epic Games, Inc. All Rights Reserved.

#include "ScreenshotCapture.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Mech.h"

AScreenshotCapture::AScreenshotCapture()
{
	PrimaryActorTick.bCanEverTick = true;

	// Create camera component
	CaptureCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CaptureCamera"));
	RootComponent = CaptureCamera;
}

void AScreenshotCapture::BeginPlay()
{
	Super::BeginPlay();

	// Find mech if not set
	if (!TargetActor)
	{
		TargetActor = Cast<AActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AMech::StaticClass()));
		if (TargetActor)
		{
			UE_LOG(LogTemp, Warning, TEXT("Screenshot: Found mech at %s"), *TargetActor->GetActorLocation().ToString());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Screenshot: No mech found in level!"));
		}
	}

	// Setup capture queue
	SetupCaptureQueue();

	UE_LOG(LogTemp, Warning, TEXT("📸 ScreenshotCapture ready! Call StartCaptureSequence() or press a bound key"));
}

void AScreenshotCapture::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bCapturing)
	{
		CaptureTimer += DeltaTime;

		if (CaptureTimer >= CaptureDelay)
		{
			ProcessNextCapture();
			CaptureTimer = 0.0f;
		}
	}
}

void AScreenshotCapture::SetupCaptureQueue()
{
	CaptureQueue.Empty();

	// Front view
	CaptureQueue.Add({
		FVector(CaptureDistance, 0, CaptureHeight),
		FRotator(0, 180, 0),
		TEXT("front_view"),
		TEXT("Front view of mech")
	});

	// Left side
	CaptureQueue.Add({
		FVector(0, -CaptureDistance, CaptureHeight),
		FRotator(0, 90, 0),
		TEXT("left_side"),
		TEXT("Left side view")
	});

	// Right side
	CaptureQueue.Add({
		FVector(0, CaptureDistance, CaptureHeight),
		FRotator(0, -90, 0),
		TEXT("right_side"),
		TEXT("Right side view")
	});

	// Top-down
	CaptureQueue.Add({
		FVector(0, 0, CaptureDistance * 2),
		FRotator(-90, 0, 0),
		TEXT("top_down"),
		TEXT("Top-down view")
	});

	// Diagonal overview
	CaptureQueue.Add({
		FVector(CaptureDistance * 1.5f, CaptureDistance * 1.5f, CaptureDistance),
		FRotator(-30, -135, 0),
		TEXT("diagonal_overview"),
		TEXT("Diagonal overview")
	});

	// Interior cockpit view
	CaptureQueue.Add({
		FVector(200, 0, 2600),
		FRotator(0, 0, 0),
		TEXT("interior_cockpit"),
		TEXT("Interior cockpit view")
	});

	// Close-up head/cockpit
	CaptureQueue.Add({
		FVector(500, 0, 3000),
		FRotator(0, 180, 0),
		TEXT("closeup_head"),
		TEXT("Close-up of head/cockpit")
	});
}

void AScreenshotCapture::StartCaptureSequence()
{
	if (!TargetActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Screenshot: Cannot capture - no target actor!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("📸 Starting screenshot capture sequence (%d shots)..."), CaptureQueue.Num());

	bCapturing = true;
	CurrentCaptureIndex = 0;
	CaptureTimer = 0.0f;

	// Start first capture immediately
	ProcessNextCapture();
}

void AScreenshotCapture::ProcessNextCapture()
{
	if (CurrentCaptureIndex >= CaptureQueue.Num())
	{
		// Done!
		bCapturing = false;
		UE_LOG(LogTemp, Warning, TEXT("✅ Screenshot capture complete! Saved %d images"), CaptureQueue.Num());
		UE_LOG(LogTemp, Warning, TEXT("   Location: [Project]/Saved/Screenshots/WindowsEditor/"));
		return;
	}

	const FCaptureInfo& Info = CaptureQueue[CurrentCaptureIndex];

	// Position camera relative to target
	FVector TargetLocation = TargetActor->GetActorLocation();
	FVector CameraWorldLocation = TargetLocation + Info.RelativePosition;

	SetActorLocation(CameraWorldLocation);
	SetActorRotation(Info.Rotation);

	UE_LOG(LogTemp, Warning, TEXT("📸 [%d/%d] %s - %s"),
		CurrentCaptureIndex + 1,
		CaptureQueue.Num(),
		*Info.Filename,
		*Info.Description);

	// Take screenshot
	TakeScreenshot(Info.Filename);

	CurrentCaptureIndex++;
}

void AScreenshotCapture::TakeScreenshot(const FString& Filename)
{
	// Use console command to take high-res screenshot
	if (GEngine && GEngine->GameViewport)
	{
		FString Command = FString::Printf(TEXT("HighResShot 1920x1080 filename=%s"), *Filename);
		GEngine->Exec(GetWorld(), *Command);
	}
}

void AScreenshotCapture::CaptureFromAngle(FVector RelativePosition, FRotator Rotation, FString Filename)
{
	if (!TargetActor)
	{
		UE_LOG(LogTemp, Error, TEXT("Screenshot: Cannot capture - no target actor!"));
		return;
	}

	FVector TargetLocation = TargetActor->GetActorLocation();
	FVector CameraWorldLocation = TargetLocation + RelativePosition;

	SetActorLocation(CameraWorldLocation);
	SetActorRotation(Rotation);

	TakeScreenshot(Filename);

	UE_LOG(LogTemp, Warning, TEXT("📸 Screenshot: %s"), *Filename);
}
