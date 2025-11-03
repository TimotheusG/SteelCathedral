// Copyright Epic Games, Inc. All Rights Reserved.

#include "TerrainSpawner.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ATerrainSpawner::ATerrainSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	// Load default CityKit paths
	LoadDefaultCityKitAssets();
}

void ATerrainSpawner::BeginPlay()
{
	Super::BeginPlay();
}

void ATerrainSpawner::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (bAutoSpawn)
	{
		SpawnTerrain();
	}
}

void ATerrainSpawner::LoadDefaultCityKitAssets()
{
	// CityKit BR rubble meshes
	RubbleMeshPaths.Empty();

	RubbleMeshPaths.Add(FSoftObjectPath(TEXT("/Game/CityKitBR_V4/Meshes/Props_Construction/Nanite/SM_Concrete_Rumble_1_N.SM_Concrete_Rumble_1_N")));
	RubbleMeshPaths.Add(FSoftObjectPath(TEXT("/Game/CityKitBR_V4/Meshes/Props_Construction/Nanite/SM_Concrete_Rumble_2_N.SM_Concrete_Rumble_2_N")));
	RubbleMeshPaths.Add(FSoftObjectPath(TEXT("/Game/CityKitBR_V4/Meshes/Props_Construction/Nanite/SM_Concrete_Rumble_3_N.SM_Concrete_Rumble_3_N")));
	RubbleMeshPaths.Add(FSoftObjectPath(TEXT("/Game/CityKitBR_V4/Meshes/Props_Construction/Nanite/SM_Brick_Rumble_1_N.SM_Brick_Rumble_1_N")));
	RubbleMeshPaths.Add(FSoftObjectPath(TEXT("/Game/CityKitBR_V4/Meshes/Props_Construction/Nanite/SM_Brick_Rumble_2_N.SM_Brick_Rumble_2_N")));
	RubbleMeshPaths.Add(FSoftObjectPath(TEXT("/Game/CityKitBR_V4/Meshes/Props_Construction/Nanite/SM_CDumpster_Rumble_N.SM_CDumpster_Rumble_N")));
	RubbleMeshPaths.Add(FSoftObjectPath(TEXT("/Game/CityKitBR_V4/Meshes/Props_Construction/Nanite/SM_CementBrick_Pile_1_N.SM_CementBrick_Pile_1_N")));
	RubbleMeshPaths.Add(FSoftObjectPath(TEXT("/Game/CityKitBR_V4/Meshes/Props_Construction/Nanite/SM_BrickPile_1_N.SM_BrickPile_1_N")));

	UE_LOG(LogTemp, Warning, TEXT("Loaded %d CityKit rubble mesh paths"), RubbleMeshPaths.Num());
}

void ATerrainSpawner::SpawnTerrain()
{
	UE_LOG(LogTemp, Warning, TEXT("Spawning terrain..."));

	ClearTerrain();

	// Initialize random stream with seed
	FRandomStream RandomStream(RandomSeed);

	// Spawn ground plane
	SpawnGroundPlane();

	// Spawn rubble pieces
	SpawnRubblePieces();

	UE_LOG(LogTemp, Warning, TEXT("Terrain spawned: %d meshes"), SpawnedMeshes.Num());
}

void ATerrainSpawner::ClearTerrain()
{
	for (UStaticMeshComponent* MeshComp : SpawnedMeshes)
	{
		if (MeshComp)
		{
			MeshComp->DestroyComponent();
		}
	}
	SpawnedMeshes.Empty();
}

void ATerrainSpawner::SpawnGroundPlane()
{
	// Create a simple ground plane
	UStaticMeshComponent* GroundMesh = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("GroundPlane"));
	if (GroundMesh)
	{
		GroundMesh->RegisterComponent();
		GroundMesh->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		// Use engine plane mesh
		UStaticMesh* PlaneMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane"));
		if (PlaneMesh)
		{
			GroundMesh->SetStaticMesh(PlaneMesh);
			GroundMesh->SetRelativeScale3D(FVector(SpawnAreaSize / 100.0f, SpawnAreaSize / 100.0f, 1.0f));
			GroundMesh->SetRelativeLocation(FVector(0, 0, 0));

			// Set collision
			GroundMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			GroundMesh->SetCollisionResponseToAllChannels(ECR_Block);

			SpawnedMeshes.Add(GroundMesh);
			UE_LOG(LogTemp, Log, TEXT("Ground plane spawned"));
		}
	}
}

void ATerrainSpawner::SpawnRubblePieces()
{
	if (RubbleMeshPaths.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No rubble mesh paths configured!"));
		return;
	}

	FRandomStream RandomStream(RandomSeed);
	float HalfSize = SpawnAreaSize * 0.5f;

	for (int32 i = 0; i < RubbleCount; i++)
	{
		// Random position
		FVector SpawnLocation = FVector(
			RandomStream.FRandRange(-HalfSize, HalfSize),
			RandomStream.FRandRange(-HalfSize, HalfSize),
			50.0f // Slightly above ground
		);

		// Random rotation
		FRotator SpawnRotation = FRotator(
			0,
			RandomStream.FRandRange(0.0f, 360.0f),
			0
		);

		// Random scale
		float Scale = RandomStream.FRandRange(0.8f, 1.5f);

		// Pick random mesh from available paths
		int32 MeshIndex = RandomStream.RandRange(0, RubbleMeshPaths.Num() - 1);
		FSoftObjectPath MeshPath = RubbleMeshPaths[MeshIndex];

		// Try to load the mesh
		UStaticMesh* RubbleMesh = Cast<UStaticMesh>(MeshPath.TryLoad());
		if (RubbleMesh)
		{
			UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(
				this,
				UStaticMeshComponent::StaticClass(),
				*FString::Printf(TEXT("Rubble_%d"), i)
			);

			if (MeshComp)
			{
				MeshComp->RegisterComponent();
				MeshComp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
				MeshComp->SetStaticMesh(RubbleMesh);
				MeshComp->SetRelativeLocation(SpawnLocation);
				MeshComp->SetRelativeRotation(SpawnRotation);
				MeshComp->SetRelativeScale3D(FVector(Scale));

				// Set collision
				MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				MeshComp->SetCollisionResponseToAllChannels(ECR_Block);

				SpawnedMeshes.Add(MeshComp);
			}
		}
		else
		{
			// Fallback to engine cube if CityKit mesh fails to load
			UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube"));
			if (CubeMesh)
			{
				UStaticMeshComponent* MeshComp = NewObject<UStaticMeshComponent>(
					this,
					UStaticMeshComponent::StaticClass(),
					*FString::Printf(TEXT("Rubble_%d"), i)
				);

				if (MeshComp)
				{
					MeshComp->RegisterComponent();
					MeshComp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
					MeshComp->SetStaticMesh(CubeMesh);
					MeshComp->SetRelativeLocation(SpawnLocation);
					MeshComp->SetRelativeRotation(SpawnRotation);
					MeshComp->SetRelativeScale3D(FVector(Scale * 2.0f)); // Cubes are bigger

					MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					MeshComp->SetCollisionResponseToAllChannels(ECR_Block);

					SpawnedMeshes.Add(MeshComp);
				}
			}
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Spawned %d rubble pieces"), SpawnedMeshes.Num() - 1); // -1 for ground plane
}
