// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProceduralInteriorGeometry.h"
#include "Materials/MaterialInstanceDynamic.h"

AProceduralInteriorGeometry::AProceduralInteriorGeometry()
{
	PrimaryActorTick.bCanEverTick = false;

	InteriorRoot = CreateDefaultSubobject<USceneComponent>(TEXT("InteriorRoot"));
	RootComponent = InteriorRoot;
}

void AProceduralInteriorGeometry::BeginPlay()
{
	Super::BeginPlay();
}

void AProceduralInteriorGeometry::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (bAutoGenerate)
	{
		GenerateInterior();
	}
}

void AProceduralInteriorGeometry::GenerateInterior()
{
	UE_LOG(LogTemp, Warning, TEXT("Generating procedural interior..."));

	ClearInterior();

	if (!bUseProceduralGeometry)
	{
		UE_LOG(LogTemp, Warning, TEXT("Static mesh mode not yet implemented"));
		return;
	}

	// Create procedural mesh components
	CockpitFloor = NewObject<UProceduralMeshComponent>(this, TEXT("CockpitFloor"));
	CockpitWalls = NewObject<UProceduralMeshComponent>(this, TEXT("CockpitWalls"));
	CockpitCeiling = NewObject<UProceduralMeshComponent>(this, TEXT("CockpitCeiling"));
	CockpitWindow = NewObject<UProceduralMeshComponent>(this, TEXT("CockpitWindow"));
	ReactorRoom = NewObject<UProceduralMeshComponent>(this, TEXT("ReactorRoom"));
	Corridors = NewObject<UProceduralMeshComponent>(this, TEXT("Corridors"));

	// Register all components
	CockpitFloor->RegisterComponent();
	CockpitFloor->AttachToComponent(InteriorRoot, FAttachmentTransformRules::KeepRelativeTransform);

	CockpitWalls->RegisterComponent();
	CockpitWalls->AttachToComponent(InteriorRoot, FAttachmentTransformRules::KeepRelativeTransform);

	CockpitCeiling->RegisterComponent();
	CockpitCeiling->AttachToComponent(InteriorRoot, FAttachmentTransformRules::KeepRelativeTransform);

	CockpitWindow->RegisterComponent();
	CockpitWindow->AttachToComponent(InteriorRoot, FAttachmentTransformRules::KeepRelativeTransform);

	ReactorRoom->RegisterComponent();
	ReactorRoom->AttachToComponent(InteriorRoot, FAttachmentTransformRules::KeepRelativeTransform);

	Corridors->RegisterComponent();
	Corridors->AttachToComponent(InteriorRoot, FAttachmentTransformRules::KeepRelativeTransform);

	// Set collision
	CockpitFloor->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CockpitWalls->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CockpitCeiling->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CockpitWindow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ReactorRoom->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Corridors->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// Generate geometry
	CreateCockpitGeometry();
	CreateWindowGeometry();
	CreateReactorRoomGeometry();
	CreateCorridorGeometry();

	UE_LOG(LogTemp, Warning, TEXT("Interior generated successfully!"));
}

void AProceduralInteriorGeometry::ClearInterior()
{
	if (CockpitFloor) { CockpitFloor->DestroyComponent(); CockpitFloor = nullptr; }
	if (CockpitWalls) { CockpitWalls->DestroyComponent(); CockpitWalls = nullptr; }
	if (CockpitCeiling) { CockpitCeiling->DestroyComponent(); CockpitCeiling = nullptr; }
	if (CockpitWindow) { CockpitWindow->DestroyComponent(); CockpitWindow = nullptr; }
	if (ReactorRoom) { ReactorRoom->DestroyComponent(); ReactorRoom = nullptr; }
	if (Corridors) { Corridors->DestroyComponent(); Corridors = nullptr; }
}

void AProceduralInteriorGeometry::CreateCockpitGeometry()
{
	float Scale = InteriorScale * 100.0f;

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	// FLOOR: 600cm x 500cm
	FVector FloorSize = FVector(600, 500, 10) * InteriorScale;
	AddQuad(Vertices, Triangles, Normals, UVs,
		FVector(-300, -250, 0) * InteriorScale,
		FVector(300, -250, 0) * InteriorScale,
		FVector(300, 250, 0) * InteriorScale,
		FVector(-300, 250, 0) * InteriorScale,
		FVector(0, 0, 1));

	CockpitFloor->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);

	// WALLS
	Vertices.Empty();
	Triangles.Empty();
	Normals.Empty();
	UVs.Empty();

	float Height = 250.0f * InteriorScale;

	// Left wall
	AddQuad(Vertices, Triangles, Normals, UVs,
		FVector(-300, -250, 0) * InteriorScale,
		FVector(-300, -250, Height),
		FVector(-300, 250, Height),
		FVector(-300, 250, 0) * InteriorScale,
		FVector(1, 0, 0));

	// Right wall
	AddQuad(Vertices, Triangles, Normals, UVs,
		FVector(300, 250, 0) * InteriorScale,
		FVector(300, 250, Height),
		FVector(300, -250, Height),
		FVector(300, -250, 0) * InteriorScale,
		FVector(-1, 0, 0));

	// Back wall
	AddQuad(Vertices, Triangles, Normals, UVs,
		FVector(-300, -250, 0) * InteriorScale,
		FVector(-300, -250, Height),
		FVector(300, -250, Height),
		FVector(300, -250, 0) * InteriorScale,
		FVector(0, 1, 0));

	CockpitWalls->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);

	// CEILING
	Vertices.Empty();
	Triangles.Empty();
	Normals.Empty();
	UVs.Empty();

	AddQuad(Vertices, Triangles, Normals, UVs,
		FVector(-300, 250, Height),
		FVector(300, 250, Height),
		FVector(300, -250, Height),
		FVector(-300, -250, Height),
		FVector(0, 0, -1));

	CockpitCeiling->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);

	// Apply materials
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* FloorMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		FloorMat->SetVectorParameterValue(TEXT("Color"), FloorColor);
		CockpitFloor->SetMaterial(0, FloorMat);

		UMaterialInstanceDynamic* WallMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		WallMat->SetVectorParameterValue(TEXT("Color"), WallColor);
		CockpitWalls->SetMaterial(0, WallMat);
		CockpitCeiling->SetMaterial(0, WallMat);
	}
}

void AProceduralInteriorGeometry::CreateWindowGeometry()
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	// HUGE WINDOW: 1500cm wide x 800cm tall
	float Width = 1500.0f * InteriorScale;
	float Height = 800.0f * InteriorScale;
	float WallHeight = 250.0f * InteriorScale;

	// Position at front of cockpit
	FVector WindowCenter = FVector(310, 0, WallHeight * 0.5f) * InteriorScale;

	AddQuad(Vertices, Triangles, Normals, UVs,
		WindowCenter + FVector(0, -Width * 0.5f, -Height * 0.5f),
		WindowCenter + FVector(0, Width * 0.5f, -Height * 0.5f),
		WindowCenter + FVector(0, Width * 0.5f, Height * 0.5f),
		WindowCenter + FVector(0, -Width * 0.5f, Height * 0.5f),
		FVector(-1, 0, 0));

	CockpitWindow->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);

	// Apply transparent glass material
	UMaterialInterface* GlassMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (GlassMaterial)
	{
		UMaterialInstanceDynamic* GlassMat = UMaterialInstanceDynamic::Create(GlassMaterial, this);
		GlassMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.7f, 0.85f, 1.0f, 0.2f));
		CockpitWindow->SetMaterial(0, GlassMat);
	}
}

void AProceduralInteriorGeometry::CreateReactorRoomGeometry()
{
	// Simple reactor room at rear
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	FVector RoomSize = FVector(400, 400, 300) * InteriorScale;
	FVector RoomCenter = FVector(-800, 0, 0) * InteriorScale;

	// Floor
	AddQuad(Vertices, Triangles, Normals, UVs,
		RoomCenter + FVector(-200, -200, 0) * InteriorScale,
		RoomCenter + FVector(200, -200, 0) * InteriorScale,
		RoomCenter + FVector(200, 200, 0) * InteriorScale,
		RoomCenter + FVector(-200, 200, 0) * InteriorScale,
		FVector(0, 0, 1));

	ReactorRoom->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);

	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		Mat->SetVectorParameterValue(TEXT("Color"), FloorColor);
		ReactorRoom->SetMaterial(0, Mat);
	}
}

void AProceduralInteriorGeometry::CreateCorridorGeometry()
{
	// Simple corridor connecting cockpit to reactor
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	float CorridorWidth = 150.0f * InteriorScale;
	float CorridorLength = 600.0f * InteriorScale;

	// Floor
	AddQuad(Vertices, Triangles, Normals, UVs,
		FVector(-300, -CorridorWidth, 0) * InteriorScale,
		FVector(-300 - CorridorLength, -CorridorWidth, 0),
		FVector(-300 - CorridorLength, CorridorWidth, 0),
		FVector(-300, CorridorWidth, 0) * InteriorScale,
		FVector(0, 0, 1));

	Corridors->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);

	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* Mat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		Mat->SetVectorParameterValue(TEXT("Color"), FloorColor);
		Corridors->SetMaterial(0, Mat);
	}
}

void AProceduralInteriorGeometry::AddQuad(TArray<FVector>& Vertices, TArray<int32>& Triangles,
                                          TArray<FVector>& Normals, TArray<FVector2D>& UVs,
                                          FVector V0, FVector V1, FVector V2, FVector V3, FVector Normal)
{
	int32 StartIndex = Vertices.Num();

	Vertices.Add(V0);
	Vertices.Add(V1);
	Vertices.Add(V2);
	Vertices.Add(V3);

	Triangles.Add(StartIndex + 0);
	Triangles.Add(StartIndex + 1);
	Triangles.Add(StartIndex + 2);

	Triangles.Add(StartIndex + 0);
	Triangles.Add(StartIndex + 2);
	Triangles.Add(StartIndex + 3);

	Normals.Add(Normal);
	Normals.Add(Normal);
	Normals.Add(Normal);
	Normals.Add(Normal);

	UVs.Add(FVector2D(0, 0));
	UVs.Add(FVector2D(1, 0));
	UVs.Add(FVector2D(1, 1));
	UVs.Add(FVector2D(0, 1));
}
