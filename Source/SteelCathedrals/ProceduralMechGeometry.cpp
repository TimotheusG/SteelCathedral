// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProceduralMechGeometry.h"
#include "ProceduralMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"

UProceduralMechGeometry::UProceduralMechGeometry()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UProceduralMechGeometry::OnComponentCreated()
{
	Super::OnComponentCreated();
}

void UProceduralMechGeometry::BeginPlay()
{
	Super::BeginPlay();

	// Load downloaded materials first
	LoadDownloadedMaterials();

	// Auto-generate on start if using procedural
	if (bUseProceduralGeometry)
	{
		GenerateMechGeometry();
	}
}

void UProceduralMechGeometry::GenerateMechGeometry()
{
	if (!GetOwner())
	{
		UE_LOG(LogTemp, Error, TEXT("ProceduralMechGeometry: No owner actor!"));
		return;
	}

	ClearGeometry();

	UE_LOG(LogTemp, Warning, TEXT("Generating procedural mech geometry..."));

	// Create procedural mesh components
	TorsoMesh = NewObject<UProceduralMeshComponent>(GetOwner(), TEXT("ProceduralTorso"));
	HeadMesh = NewObject<UProceduralMeshComponent>(GetOwner(), TEXT("ProceduralHead"));
	LegLeftMesh = NewObject<UProceduralMeshComponent>(GetOwner(), TEXT("ProceduralLegLeft"));
	LegRightMesh = NewObject<UProceduralMeshComponent>(GetOwner(), TEXT("ProceduralLegRight"));
	ArmLeftMesh = NewObject<UProceduralMeshComponent>(GetOwner(), TEXT("ProceduralArmLeft"));
	ArmRightMesh = NewObject<UProceduralMeshComponent>(GetOwner(), TEXT("ProceduralArmRight"));

	// Register and attach components
	TorsoMesh->RegisterComponent();
	TorsoMesh->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	HeadMesh->RegisterComponent();
	HeadMesh->AttachToComponent(TorsoMesh, FAttachmentTransformRules::KeepRelativeTransform);

	LegLeftMesh->RegisterComponent();
	LegLeftMesh->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	LegRightMesh->RegisterComponent();
	LegRightMesh->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

	ArmLeftMesh->RegisterComponent();
	ArmLeftMesh->AttachToComponent(TorsoMesh, FAttachmentTransformRules::KeepRelativeTransform);

	ArmRightMesh->RegisterComponent();
	ArmRightMesh->AttachToComponent(TorsoMesh, FAttachmentTransformRules::KeepRelativeTransform);

	// Set collision
	TorsoMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LegLeftMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	LegRightMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ArmLeftMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ArmRightMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Generate geometry based on mech scale
	float Scale = MechScale * 100.0f; // Convert to cm

	// TORSO: 800cm x 600cm x 1000cm (scaled)
	FVector TorsoSize = FVector(800, 600, 1000) * MechScale;
	TorsoMesh->SetRelativeLocation(FVector(0, 0, 2000 * MechScale));
	CreateBeveledBox(TorsoMesh, TorsoSize, 20.0f * MechScale);

	// HEAD: 300cm x 400cm x 200cm, angled forward
	FVector HeadSize = FVector(300, 400, 200) * MechScale;
	HeadMesh->SetRelativeLocation(FVector(200 * MechScale, 0, 600 * MechScale));
	CreateCockpitHead(HeadMesh, HeadSize);

	// LEGS: 200cm x 200cm x 1600cm each
	FVector LegSize = FVector(200, 200, 1600) * MechScale;
	LegLeftMesh->SetRelativeLocation(FVector(-200 * MechScale, -200 * MechScale, 800 * MechScale));
	CreateLegSegment(LegLeftMesh, LegSize);

	LegRightMesh->SetRelativeLocation(FVector(-200 * MechScale, 200 * MechScale, 800 * MechScale));
	CreateLegSegment(LegRightMesh, LegSize);

	// ARMS: 150cm x 150cm x 600cm each
	FVector ArmSize = FVector(150, 150, 600) * MechScale;
	ArmLeftMesh->SetRelativeLocation(FVector(0, -400 * MechScale, 200 * MechScale));
	CreateArmSegment(ArmLeftMesh, ArmSize);

	ArmRightMesh->SetRelativeLocation(FVector(0, 400 * MechScale, 200 * MechScale));
	CreateArmSegment(ArmRightMesh, ArmSize);

	// Apply materials
	ApplyProceduralMaterial(TorsoMesh);
	ApplyProceduralMaterial(HeadMesh);
	ApplyProceduralMaterial(LegLeftMesh);
	ApplyProceduralMaterial(LegRightMesh);
	ApplyProceduralMaterial(ArmLeftMesh);
	ApplyProceduralMaterial(ArmRightMesh);

	UE_LOG(LogTemp, Warning, TEXT("Procedural mech geometry generated successfully!"));
}

void UProceduralMechGeometry::ClearGeometry()
{
	// Destroy procedural meshes
	if (TorsoMesh) { TorsoMesh->DestroyComponent(); TorsoMesh = nullptr; }
	if (HeadMesh) { HeadMesh->DestroyComponent(); HeadMesh = nullptr; }
	if (LegLeftMesh) { LegLeftMesh->DestroyComponent(); LegLeftMesh = nullptr; }
	if (LegRightMesh) { LegRightMesh->DestroyComponent(); LegRightMesh = nullptr; }
	if (ArmLeftMesh) { ArmLeftMesh->DestroyComponent(); ArmLeftMesh = nullptr; }
	if (ArmRightMesh) { ArmRightMesh->DestroyComponent(); ArmRightMesh = nullptr; }

	// Destroy static meshes
	if (TorsoStaticMesh) { TorsoStaticMesh->DestroyComponent(); TorsoStaticMesh = nullptr; }
	if (HeadStaticMesh) { HeadStaticMesh->DestroyComponent(); HeadStaticMesh = nullptr; }
	if (LegLeftStaticMesh) { LegLeftStaticMesh->DestroyComponent(); LegLeftStaticMesh = nullptr; }
	if (LegRightStaticMesh) { LegRightStaticMesh->DestroyComponent(); LegRightStaticMesh = nullptr; }
	if (ArmLeftStaticMesh) { ArmLeftStaticMesh->DestroyComponent(); ArmLeftStaticMesh = nullptr; }
	if (ArmRightStaticMesh) { ArmRightStaticMesh->DestroyComponent(); ArmRightStaticMesh = nullptr; }
}

void UProceduralMechGeometry::SwapToStaticMeshes()
{
	// TODO: Implement static mesh swapping when assets are available
	UE_LOG(LogTemp, Warning, TEXT("Static mesh swapping not yet implemented"));
}

void UProceduralMechGeometry::SwapToProceduralGeometry()
{
	bUseProceduralGeometry = true;
	GenerateMechGeometry();
}

void UProceduralMechGeometry::CreateBeveledBox(UProceduralMeshComponent* MeshComp, FVector Size, float BevelAmount)
{
	if (!MeshComp) return;

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	FVector HalfSize = Size * 0.5f;
	float Bevel = FMath::Min(BevelAmount, FMath::Min3(Size.X, Size.Y, Size.Z) * 0.1f);

	// Create 6 faces with slight beveling (simplified box for performance)
	// Front face
	AddQuad(Vertices, Triangles, Normals, UVs,
		FVector(HalfSize.X, -HalfSize.Y, -HalfSize.Z),
		FVector(HalfSize.X, HalfSize.Y, -HalfSize.Z),
		FVector(HalfSize.X, HalfSize.Y, HalfSize.Z),
		FVector(HalfSize.X, -HalfSize.Y, HalfSize.Z),
		FVector(1, 0, 0));

	// Back face
	AddQuad(Vertices, Triangles, Normals, UVs,
		FVector(-HalfSize.X, HalfSize.Y, -HalfSize.Z),
		FVector(-HalfSize.X, -HalfSize.Y, -HalfSize.Z),
		FVector(-HalfSize.X, -HalfSize.Y, HalfSize.Z),
		FVector(-HalfSize.X, HalfSize.Y, HalfSize.Z),
		FVector(-1, 0, 0));

	// Right face
	AddQuad(Vertices, Triangles, Normals, UVs,
		FVector(-HalfSize.X, HalfSize.Y, -HalfSize.Z),
		FVector(HalfSize.X, HalfSize.Y, -HalfSize.Z),
		FVector(HalfSize.X, HalfSize.Y, HalfSize.Z),
		FVector(-HalfSize.X, HalfSize.Y, HalfSize.Z),
		FVector(0, 1, 0));

	// Left face
	AddQuad(Vertices, Triangles, Normals, UVs,
		FVector(HalfSize.X, -HalfSize.Y, -HalfSize.Z),
		FVector(-HalfSize.X, -HalfSize.Y, -HalfSize.Z),
		FVector(-HalfSize.X, -HalfSize.Y, HalfSize.Z),
		FVector(HalfSize.X, -HalfSize.Y, HalfSize.Z),
		FVector(0, -1, 0));

	// Top face
	AddQuad(Vertices, Triangles, Normals, UVs,
		FVector(-HalfSize.X, -HalfSize.Y, HalfSize.Z),
		FVector(-HalfSize.X, HalfSize.Y, HalfSize.Z),
		FVector(HalfSize.X, HalfSize.Y, HalfSize.Z),
		FVector(HalfSize.X, -HalfSize.Y, HalfSize.Z),
		FVector(0, 0, 1));

	// Bottom face
	AddQuad(Vertices, Triangles, Normals, UVs,
		FVector(-HalfSize.X, HalfSize.Y, -HalfSize.Z),
		FVector(-HalfSize.X, -HalfSize.Y, -HalfSize.Z),
		FVector(HalfSize.X, -HalfSize.Y, -HalfSize.Z),
		FVector(HalfSize.X, HalfSize.Y, -HalfSize.Z),
		FVector(0, 0, -1));

	MeshComp->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);
}

void UProceduralMechGeometry::CreateCockpitHead(UProceduralMeshComponent* MeshComp, FVector Size)
{
	if (!MeshComp) return;

	// Angled cockpit - front is taller than back
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	FVector HalfSize = Size * 0.5f;
	float AngleOffset = HalfSize.Z * 0.3f; // Tilt forward

	// Create angled box (simplified)
	// Front face (angled forward)
	AddQuad(Vertices, Triangles, Normals, UVs,
		FVector(HalfSize.X + AngleOffset, -HalfSize.Y, -HalfSize.Z),
		FVector(HalfSize.X + AngleOffset, HalfSize.Y, -HalfSize.Z),
		FVector(HalfSize.X, HalfSize.Y, HalfSize.Z),
		FVector(HalfSize.X, -HalfSize.Y, HalfSize.Z),
		FVector(1, 0, 0).GetSafeNormal());

	// Other faces similar to beveled box
	CreateBeveledBox(MeshComp, Size * 0.9f, 10.0f);
}

void UProceduralMechGeometry::CreateLegSegment(UProceduralMeshComponent* MeshComp, FVector Size)
{
	// Legs are just elongated boxes for now
	CreateBeveledBox(MeshComp, Size, 15.0f);
}

void UProceduralMechGeometry::CreateArmSegment(UProceduralMeshComponent* MeshComp, FVector Size)
{
	// Arms are elongated boxes
	CreateBeveledBox(MeshComp, Size, 10.0f);
}

void UProceduralMechGeometry::ApplyProceduralMaterial(UProceduralMeshComponent* MeshComp)
{
	if (!MeshComp) return;

	// Use downloaded rusty metal material if available
	if (RustyMetalMaterial)
	{
		MeshComp->SetMaterial(0, RustyMetalMaterial);
		UE_LOG(LogTemp, Log, TEXT("Applied downloaded rusty metal material"));
	}
	else
	{
		// Fallback to basic material
		UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
		if (BaseMaterial)
		{
			UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, MeshComp);
			if (DynMaterial)
			{
				DynMaterial->SetVectorParameterValue(TEXT("Color"), MechColor);
				MeshComp->SetMaterial(0, DynMaterial);
			}
		}
		UE_LOG(LogTemp, Warning, TEXT("Downloaded materials not available, using fallback"));
	}
}

void UProceduralMechGeometry::AddQuad(TArray<FVector>& Vertices, TArray<int32>& Triangles,
                                      TArray<FVector>& Normals, TArray<FVector2D>& UVs,
                                      FVector V0, FVector V1, FVector V2, FVector V3, FVector Normal)
{
	int32 StartIndex = Vertices.Num();

	Vertices.Add(V0);
	Vertices.Add(V1);
	Vertices.Add(V2);
	Vertices.Add(V3);

	// First triangle
	Triangles.Add(StartIndex + 0);
	Triangles.Add(StartIndex + 1);
	Triangles.Add(StartIndex + 2);

	// Second triangle
	Triangles.Add(StartIndex + 0);
	Triangles.Add(StartIndex + 2);
	Triangles.Add(StartIndex + 3);

	// Normals
	Normals.Add(Normal);
	Normals.Add(Normal);
	Normals.Add(Normal);
	Normals.Add(Normal);

	// UVs
	UVs.Add(FVector2D(0, 0));
	UVs.Add(FVector2D(1, 0));
	UVs.Add(FVector2D(1, 1));
	UVs.Add(FVector2D(0, 1));
}

void UProceduralMechGeometry::GeneratePlaneUVs(TArray<FVector2D>& UVs, int32 Count)
{
	for (int32 i = 0; i < Count; i++)
	{
		UVs.Add(FVector2D(0, 0));
	}
}

void UProceduralMechGeometry::LoadDownloadedMaterials()
{
	UE_LOG(LogTemp, Warning, TEXT("Loading downloaded materials from Poly Haven..."));

	// Try to load rusty metal textures
	UTexture2D* RustyDiffuse = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Materials/Downloaded/PolyHaven/rusty_metal_02_diff_2k"));
	UTexture2D* RustyNormal = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Materials/Downloaded/PolyHaven/rusty_metal_02_nor_gl_2k"));
	UTexture2D* RustyRoughness = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Materials/Downloaded/PolyHaven/rusty_metal_02_rough_2k"));

	// Try to load metal plate textures
	UTexture2D* PlateDiffuse = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Materials/Downloaded/PolyHaven/metal_plate_diff_2k"));
	UTexture2D* PlateNormal = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Materials/Downloaded/PolyHaven/metal_plate_nor_gl_2k"));
	UTexture2D* PlateRoughness = LoadObject<UTexture2D>(nullptr, TEXT("/Game/Materials/Downloaded/PolyHaven/metal_plate_rough_2k"));

	// Create rusty metal material if textures loaded
	if (RustyDiffuse && RustyNormal && RustyRoughness)
	{
		RustyMetalMaterial = CreateMaterialWithTextures(RustyDiffuse, RustyNormal, RustyRoughness);
		UE_LOG(LogTemp, Warning, TEXT("✅ Rusty metal material created successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("❌ Could not load rusty metal textures (they may not be imported yet)"));
	}

	// Create metal plate material if textures loaded
	if (PlateDiffuse && PlateNormal && PlateRoughness)
	{
		MetalPlateMaterial = CreateMaterialWithTextures(PlateDiffuse, PlateNormal, PlateRoughness);
		UE_LOG(LogTemp, Warning, TEXT("✅ Metal plate material created successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("❌ Could not load metal plate textures (they may not be imported yet)"));
	}
}

UMaterialInstanceDynamic* UProceduralMechGeometry::CreateMaterialWithTextures(UTexture2D* Diffuse, UTexture2D* Normal, UTexture2D* Roughness)
{
	if (!Diffuse || !Normal || !Roughness)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateMaterialWithTextures: Missing textures!"));
		return nullptr;
	}

	// Load base material (we'll use the engine's default lit material as base)
	UMaterial* BaseMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Engine/EngineMaterials/WorldGridMaterial"));
	if (!BaseMaterial)
	{
		// Try another base material
		BaseMaterial = LoadObject<UMaterial>(nullptr, TEXT("/Engine/EngineMaterials/DefaultMaterial"));
	}

	if (!BaseMaterial)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not load base material!"));
		return nullptr;
	}

	// Create dynamic material instance
	UMaterialInstanceDynamic* DynMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);

	if (DynMaterial)
	{
		// Try to set texture parameters (if the base material supports them)
		DynMaterial->SetTextureParameterValue(TEXT("Texture"), Diffuse);
		DynMaterial->SetTextureParameterValue(TEXT("Diffuse"), Diffuse);
		DynMaterial->SetTextureParameterValue(TEXT("BaseColor"), Diffuse);
		DynMaterial->SetTextureParameterValue(TEXT("Normal"), Normal);
		DynMaterial->SetTextureParameterValue(TEXT("Roughness"), Roughness);

		UE_LOG(LogTemp, Log, TEXT("Created material instance with textures"));
	}

	return DynMaterial;
}
