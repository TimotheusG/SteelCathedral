// Copyright Epic Games, Inc. All Rights Reserved.

#include "ProceduralInteriorGeometry.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "MechStation.h"
#include "FireHazardActor.h"
#include "Components/PointLightComponent.h"

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

	// Detailed components
	Consoles = NewObject<UProceduralMeshComponent>(this, TEXT("Consoles"));
	ConsoleScreens = NewObject<UProceduralMeshComponent>(this, TEXT("ConsoleScreens"));
	FloorGrating = NewObject<UProceduralMeshComponent>(this, TEXT("FloorGrating"));
	WallPanels = NewObject<UProceduralMeshComponent>(this, TEXT("WallPanels"));
	Reactor = NewObject<UProceduralMeshComponent>(this, TEXT("Reactor"));
	Catwalks = NewObject<UProceduralMeshComponent>(this, TEXT("Catwalks"));

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

	Consoles->RegisterComponent();
	Consoles->AttachToComponent(InteriorRoot, FAttachmentTransformRules::KeepRelativeTransform);

	ConsoleScreens->RegisterComponent();
	ConsoleScreens->AttachToComponent(InteriorRoot, FAttachmentTransformRules::KeepRelativeTransform);

	FloorGrating->RegisterComponent();
	FloorGrating->AttachToComponent(InteriorRoot, FAttachmentTransformRules::KeepRelativeTransform);

	WallPanels->RegisterComponent();
	WallPanels->AttachToComponent(InteriorRoot, FAttachmentTransformRules::KeepRelativeTransform);

	Reactor->RegisterComponent();
	Reactor->AttachToComponent(InteriorRoot, FAttachmentTransformRules::KeepRelativeTransform);

	Catwalks->RegisterComponent();
	Catwalks->AttachToComponent(InteriorRoot, FAttachmentTransformRules::KeepRelativeTransform);

	// Set collision
	CockpitFloor->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CockpitWalls->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CockpitCeiling->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CockpitWindow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ReactorRoom->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Corridors->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Consoles->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ConsoleScreens->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FloorGrating->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WallPanels->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Reactor->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Catwalks->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// Generate geometry
	CreateCockpitGeometry();
	CreateWindowGeometry();
	CreateReactorRoomGeometry();
	CreateCorridorGeometry();

	// Create detailed geometry
	CreateConsoleGeometry();
	CreateConsoleScreenGeometry();
	CreateFloorGratingGeometry();
	CreateWallPanelsGeometry();
	CreateReactorGeometry();
	CreateCatwalkGeometry();

	// Spawn stations in cockpit (only if we have a world)
	if (GetWorld())
	{
		SpawnStations();
		SpawnTestHazards();
		SpawnLights();
	}

	UE_LOG(LogTemp, Warning, TEXT("Interior generated successfully!"));
}

TArray<FVector> AProceduralInteriorGeometry::GetCrewSpawnPositions() const
{
	TArray<FVector> SpawnPositions;

	// Define station positions (these are the SAME as in SpawnStations)
	// These are RELATIVE to interior origin
	TArray<FVector> StationPositions = {
		FVector(-200, -150, 0) * InteriorScale, // Pilot
		FVector(-200, 150, 0) * InteriorScale,  // Gunner
		FVector(0, 0, 0) * InteriorScale        // Center (for 3rd player)
	};

	// Spawn crew BEHIND each station (so they face it)
	const float SpawnDistanceBehindStation = 80.0f * InteriorScale; // 80cm behind station
	const float SpawnHeight = 100.0f * InteriorScale; // Standing height

	// Player 1: Behind pilot station
	SpawnPositions.Add(StationPositions[0] + FVector(SpawnDistanceBehindStation, 0, SpawnHeight));

	// Player 2: Behind gunner station
	SpawnPositions.Add(StationPositions[1] + FVector(SpawnDistanceBehindStation, 0, SpawnHeight));

	// Player 3: Center of cockpit
	SpawnPositions.Add(StationPositions[2] + FVector(0, 0, SpawnHeight));

	return SpawnPositions;
}

void AProceduralInteriorGeometry::SpawnStations()
{
	// Clear existing stations
	for (AMechStation* Station : Stations)
	{
		if (Station)
		{
			Station->Destroy();
		}
	}
	Stations.Empty();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	// Pilot station - front left
	AMechStation* PilotStation = GetWorld()->SpawnActor<AMechStation>(
		AMechStation::StaticClass(),
		GetActorLocation() + FVector(-200, -150, 0) * InteriorScale,
		FRotator(0, 0, 0),
		SpawnParams
	);
	if (PilotStation)
	{
		PilotStation->StationType = EStationType::Pilot;
		PilotStation->StationName = "Pilot Console";
		PilotStation->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		Stations.Add(PilotStation);
		UE_LOG(LogTemp, Warning, TEXT("✅ Spawned Pilot Station"));
	}

	// Gunner station - front right
	AMechStation* GunnerStation = GetWorld()->SpawnActor<AMechStation>(
		AMechStation::StaticClass(),
		GetActorLocation() + FVector(-200, 150, 0) * InteriorScale,
		FRotator(0, 0, 0),
		SpawnParams
	);
	if (GunnerStation)
	{
		GunnerStation->StationType = EStationType::Gunner;
		GunnerStation->StationName = "Gunner Console";
		GunnerStation->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		Stations.Add(GunnerStation);
		UE_LOG(LogTemp, Warning, TEXT("✅ Spawned Gunner Station"));
	}

	// Technician station - back left
	AMechStation* TechStation = GetWorld()->SpawnActor<AMechStation>(
		AMechStation::StaticClass(),
		GetActorLocation() + FVector(150, -150, 0) * InteriorScale,
		FRotator(0, 180, 0),
		SpawnParams
	);
	if (TechStation)
	{
		TechStation->StationType = EStationType::Technician;
		TechStation->StationName = "Technician Console";
		TechStation->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		Stations.Add(TechStation);
		UE_LOG(LogTemp, Warning, TEXT("✅ Spawned Technician Station"));
	}

	// Navigation station - back right
	AMechStation* NavStation = GetWorld()->SpawnActor<AMechStation>(
		AMechStation::StaticClass(),
		GetActorLocation() + FVector(150, 150, 0) * InteriorScale,
		FRotator(0, 180, 0),
		SpawnParams
	);
	if (NavStation)
	{
		NavStation->StationType = EStationType::Navigation;
		NavStation->StationName = "Navigation Console";
		NavStation->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		Stations.Add(NavStation);
		UE_LOG(LogTemp, Warning, TEXT("✅ Spawned Navigation Station"));
	}

	UE_LOG(LogTemp, Warning, TEXT("🎮 Spawned %d stations in cockpit"), Stations.Num());
}

void AProceduralInteriorGeometry::SpawnTestHazards()
{
	// Clear existing hazards
	for (AFireHazardActor* Hazard : TestHazards)
	{
		if (Hazard)
		{
			Hazard->Destroy();
		}
	}
	TestHazards.Empty();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	// Spawn a small test fire in the corner
	AFireHazardActor* TestFire = GetWorld()->SpawnActor<AFireHazardActor>(
		AFireHazardActor::StaticClass(),
		GetActorLocation() + FVector(200, 200, 50) * InteriorScale,
		FRotator::ZeroRotator,
		SpawnParams
	);
	if (TestFire)
	{
		TestFire->AttachToActor(this, FAttachmentTransformRules::KeepRelativeTransform);
		TestHazards.Add(TestFire);
		UE_LOG(LogTemp, Warning, TEXT("🔥 Spawned test fire hazard"));
	}
}

void AProceduralInteriorGeometry::ClearInterior()
{
	if (CockpitFloor) { CockpitFloor->DestroyComponent(); CockpitFloor = nullptr; }
	if (CockpitWalls) { CockpitWalls->DestroyComponent(); CockpitWalls = nullptr; }
	if (CockpitCeiling) { CockpitCeiling->DestroyComponent(); CockpitCeiling = nullptr; }
	if (CockpitWindow) { CockpitWindow->DestroyComponent(); CockpitWindow = nullptr; }
	if (ReactorRoom) { ReactorRoom->DestroyComponent(); ReactorRoom = nullptr; }
	if (Corridors) { Corridors->DestroyComponent(); Corridors = nullptr; }
	if (Consoles) { Consoles->DestroyComponent(); Consoles = nullptr; }
	if (ConsoleScreens) { ConsoleScreens->DestroyComponent(); ConsoleScreens = nullptr; }
	if (FloorGrating) { FloorGrating->DestroyComponent(); FloorGrating = nullptr; }
	if (WallPanels) { WallPanels->DestroyComponent(); WallPanels = nullptr; }
	if (Reactor) { Reactor->DestroyComponent(); Reactor = nullptr; }
	if (Catwalks) { Catwalks->DestroyComponent(); Catwalks = nullptr; }
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

void AProceduralInteriorGeometry::CreateBox(TArray<FVector>& Vertices, TArray<int32>& Triangles,
                                            TArray<FVector>& Normals, TArray<FVector2D>& UVs,
                                            FVector Center, FVector Size)
{
	FVector HalfSize = Size * 0.5f;

	// Front face (+X)
	AddQuad(Vertices, Triangles, Normals, UVs,
		Center + FVector(HalfSize.X, -HalfSize.Y, -HalfSize.Z),
		Center + FVector(HalfSize.X, HalfSize.Y, -HalfSize.Z),
		Center + FVector(HalfSize.X, HalfSize.Y, HalfSize.Z),
		Center + FVector(HalfSize.X, -HalfSize.Y, HalfSize.Z),
		FVector(1, 0, 0));

	// Back face (-X)
	AddQuad(Vertices, Triangles, Normals, UVs,
		Center + FVector(-HalfSize.X, HalfSize.Y, -HalfSize.Z),
		Center + FVector(-HalfSize.X, -HalfSize.Y, -HalfSize.Z),
		Center + FVector(-HalfSize.X, -HalfSize.Y, HalfSize.Z),
		Center + FVector(-HalfSize.X, HalfSize.Y, HalfSize.Z),
		FVector(-1, 0, 0));

	// Right face (+Y)
	AddQuad(Vertices, Triangles, Normals, UVs,
		Center + FVector(HalfSize.X, HalfSize.Y, -HalfSize.Z),
		Center + FVector(-HalfSize.X, HalfSize.Y, -HalfSize.Z),
		Center + FVector(-HalfSize.X, HalfSize.Y, HalfSize.Z),
		Center + FVector(HalfSize.X, HalfSize.Y, HalfSize.Z),
		FVector(0, 1, 0));

	// Left face (-Y)
	AddQuad(Vertices, Triangles, Normals, UVs,
		Center + FVector(-HalfSize.X, -HalfSize.Y, -HalfSize.Z),
		Center + FVector(HalfSize.X, -HalfSize.Y, -HalfSize.Z),
		Center + FVector(HalfSize.X, -HalfSize.Y, HalfSize.Z),
		Center + FVector(-HalfSize.X, -HalfSize.Y, HalfSize.Z),
		FVector(0, -1, 0));

	// Top face (+Z)
	AddQuad(Vertices, Triangles, Normals, UVs,
		Center + FVector(-HalfSize.X, -HalfSize.Y, HalfSize.Z),
		Center + FVector(HalfSize.X, -HalfSize.Y, HalfSize.Z),
		Center + FVector(HalfSize.X, HalfSize.Y, HalfSize.Z),
		Center + FVector(-HalfSize.X, HalfSize.Y, HalfSize.Z),
		FVector(0, 0, 1));

	// Bottom face (-Z)
	AddQuad(Vertices, Triangles, Normals, UVs,
		Center + FVector(-HalfSize.X, HalfSize.Y, -HalfSize.Z),
		Center + FVector(HalfSize.X, HalfSize.Y, -HalfSize.Z),
		Center + FVector(HalfSize.X, -HalfSize.Y, -HalfSize.Z),
		Center + FVector(-HalfSize.X, -HalfSize.Y, -HalfSize.Z),
		FVector(0, 0, -1));
}

void AProceduralInteriorGeometry::CreateConsoleGeometry()
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	// Create console boxes at each station location
	FVector ConsoleSize = FVector(80, 120, 100) * InteriorScale;

	// Pilot console - front left
	CreateBox(Vertices, Triangles, Normals, UVs,
		FVector(-200, -150, 50) * InteriorScale, ConsoleSize);

	// Gunner console - front right
	CreateBox(Vertices, Triangles, Normals, UVs,
		FVector(-200, 150, 50) * InteriorScale, ConsoleSize);

	// Technician console - back left
	CreateBox(Vertices, Triangles, Normals, UVs,
		FVector(150, -150, 50) * InteriorScale, ConsoleSize);

	// Navigation console - back right
	CreateBox(Vertices, Triangles, Normals, UVs,
		FVector(150, 150, 50) * InteriorScale, ConsoleSize);

	Consoles->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);

	// Apply material - brighter for visibility
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* ConsoleMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		ConsoleMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.4f, 0.45f, 0.5f, 1.0f)); // Lighter grey
		Consoles->SetMaterial(0, ConsoleMat);
	}
}

void AProceduralInteriorGeometry::CreateConsoleScreenGeometry()
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	FVector ScreenSize = FVector(1, 50, 40) * InteriorScale; // Thin vertical screens

	// Pilot screen - cyan
	AddQuad(Vertices, Triangles, Normals, UVs,
		FVector(-200, -150, 100) * InteriorScale + FVector(-ScreenSize.X/2, -ScreenSize.Y/2, 0),
		FVector(-200, -150, 100) * InteriorScale + FVector(-ScreenSize.X/2, ScreenSize.Y/2, 0),
		FVector(-200, -150, 100) * InteriorScale + FVector(-ScreenSize.X/2, ScreenSize.Y/2, ScreenSize.Z),
		FVector(-200, -150, 100) * InteriorScale + FVector(-ScreenSize.X/2, -ScreenSize.Y/2, ScreenSize.Z),
		FVector(-1, 0, 0));

	// Gunner screen - cyan
	AddQuad(Vertices, Triangles, Normals, UVs,
		FVector(-200, 150, 100) * InteriorScale + FVector(-ScreenSize.X/2, -ScreenSize.Y/2, 0),
		FVector(-200, 150, 100) * InteriorScale + FVector(-ScreenSize.X/2, ScreenSize.Y/2, 0),
		FVector(-200, 150, 100) * InteriorScale + FVector(-ScreenSize.X/2, ScreenSize.Y/2, ScreenSize.Z),
		FVector(-200, 150, 100) * InteriorScale + FVector(-ScreenSize.X/2, -ScreenSize.Y/2, ScreenSize.Z),
		FVector(-1, 0, 0));

	// Tech screen - orange
	AddQuad(Vertices, Triangles, Normals, UVs,
		FVector(150, -150, 100) * InteriorScale + FVector(ScreenSize.X/2, -ScreenSize.Y/2, 0),
		FVector(150, -150, 100) * InteriorScale + FVector(ScreenSize.X/2, ScreenSize.Y/2, 0),
		FVector(150, -150, 100) * InteriorScale + FVector(ScreenSize.X/2, ScreenSize.Y/2, ScreenSize.Z),
		FVector(150, -150, 100) * InteriorScale + FVector(ScreenSize.X/2, -ScreenSize.Y/2, ScreenSize.Z),
		FVector(1, 0, 0));

	// Nav screen - orange
	AddQuad(Vertices, Triangles, Normals, UVs,
		FVector(150, 150, 100) * InteriorScale + FVector(ScreenSize.X/2, -ScreenSize.Y/2, 0),
		FVector(150, 150, 100) * InteriorScale + FVector(ScreenSize.X/2, ScreenSize.Y/2, 0),
		FVector(150, 150, 100) * InteriorScale + FVector(ScreenSize.X/2, ScreenSize.Y/2, ScreenSize.Z),
		FVector(150, 150, 100) * InteriorScale + FVector(ScreenSize.X/2, -ScreenSize.Y/2, ScreenSize.Z),
		FVector(1, 0, 0));

	ConsoleScreens->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);

	// Glowing material
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* ScreenMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		ScreenMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.5f, 1.5f, 2.0f, 1.0f)); // Bright cyan glow
		ConsoleScreens->SetMaterial(0, ScreenMat);
	}
}

void AProceduralInteriorGeometry::CreateFloorGratingGeometry()
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	// Create industrial grating pattern - grid of raised bars
	float BarWidth = 8.0f * InteriorScale;
	float BarHeight = 3.0f * InteriorScale;
	float GridSpacing = 50.0f * InteriorScale;

	// Floor bounds
	float FloorMinX = -290.0f * InteriorScale;
	float FloorMaxX = 290.0f * InteriorScale;
	float FloorMinY = -240.0f * InteriorScale;
	float FloorMaxY = 240.0f * InteriorScale;
	float FloorZ = 1.0f * InteriorScale;

	// X-direction bars
	for (float Y = FloorMinY; Y <= FloorMaxY; Y += GridSpacing)
	{
		CreateBox(Vertices, Triangles, Normals, UVs,
			FVector((FloorMinX + FloorMaxX) * 0.5f, Y, FloorZ + BarHeight * 0.5f),
			FVector(FloorMaxX - FloorMinX, BarWidth, BarHeight));
	}

	// Y-direction bars
	for (float X = FloorMinX; X <= FloorMaxX; X += GridSpacing)
	{
		CreateBox(Vertices, Triangles, Normals, UVs,
			FVector(X, (FloorMinY + FloorMaxY) * 0.5f, FloorZ + BarHeight * 0.5f),
			FVector(BarWidth, FloorMaxY - FloorMinY, BarHeight));
	}

	FloorGrating->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);

	// Apply material
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* GratingMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		GratingMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.3f, 0.3f, 0.32f, 1.0f));
		FloorGrating->SetMaterial(0, GratingMat);
	}
}

void AProceduralInteriorGeometry::CreateWallPanelsGeometry()
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	float PanelDepth = 5.0f * InteriorScale;
	float PanelInset = 10.0f * InteriorScale;
	float Height = 250.0f * InteriorScale;

	// Left wall panels
	for (int i = 0; i < 4; i++)
	{
		float Y = (-200.0f + i * 120.0f) * InteriorScale;
		CreateBox(Vertices, Triangles, Normals, UVs,
			FVector(-300.0f * InteriorScale + PanelDepth * 0.5f, Y, Height * 0.5f),
			FVector(PanelDepth, 80.0f * InteriorScale, 180.0f * InteriorScale));
	}

	// Right wall panels
	for (int i = 0; i < 4; i++)
	{
		float Y = (-200.0f + i * 120.0f) * InteriorScale;
		CreateBox(Vertices, Triangles, Normals, UVs,
			FVector(300.0f * InteriorScale - PanelDepth * 0.5f, Y, Height * 0.5f),
			FVector(PanelDepth, 80.0f * InteriorScale, 180.0f * InteriorScale));
	}

	// Back wall panels
	for (int i = 0; i < 4; i++)
	{
		float X = (-200.0f + i * 120.0f) * InteriorScale;
		CreateBox(Vertices, Triangles, Normals, UVs,
			FVector(X, -250.0f * InteriorScale + PanelDepth * 0.5f, Height * 0.5f),
			FVector(80.0f * InteriorScale, PanelDepth, 180.0f * InteriorScale));
	}

	WallPanels->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);

	// Apply material
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* PanelMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		PanelMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.2f, 0.22f, 0.25f, 1.0f));
		WallPanels->SetMaterial(0, PanelMat);
	}
}

void AProceduralInteriorGeometry::CreateReactorGeometry()
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	// Create cylindrical reactor core (approximated with octagon)
	FVector ReactorCenter = FVector(-800, 0, 150) * InteriorScale;
	float ReactorRadius = 80.0f * InteriorScale;
	float ReactorHeight = 200.0f * InteriorScale;
	int32 NumSides = 8;

	// Create octagonal cylinder
	for (int i = 0; i < NumSides; i++)
	{
		float Angle1 = (i / (float)NumSides) * PI * 2.0f;
		float Angle2 = ((i + 1) / (float)NumSides) * PI * 2.0f;

		FVector P1 = ReactorCenter + FVector(0, FMath::Cos(Angle1) * ReactorRadius, -ReactorHeight * 0.5f + FMath::Sin(Angle1) * ReactorRadius);
		FVector P2 = ReactorCenter + FVector(0, FMath::Cos(Angle2) * ReactorRadius, -ReactorHeight * 0.5f + FMath::Sin(Angle2) * ReactorRadius);
		FVector P3 = ReactorCenter + FVector(0, FMath::Cos(Angle2) * ReactorRadius, ReactorHeight * 0.5f + FMath::Sin(Angle2) * ReactorRadius);
		FVector P4 = ReactorCenter + FVector(0, FMath::Cos(Angle1) * ReactorRadius, ReactorHeight * 0.5f + FMath::Sin(Angle1) * ReactorRadius);

		FVector Normal = FVector(0, FMath::Cos((Angle1 + Angle2) * 0.5f), FMath::Sin((Angle1 + Angle2) * 0.5f));
		AddQuad(Vertices, Triangles, Normals, UVs, P1, P2, P3, P4, Normal);
	}

	// Reactor core housing (box around it)
	CreateBox(Vertices, Triangles, Normals, UVs,
		ReactorCenter, FVector(120, 200, 240) * InteriorScale);

	Reactor->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);

	// Apply glowing material
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* ReactorMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		ReactorMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.2f, 0.5f, 1.0f, 1.0f)); // Cyan glow
		Reactor->SetMaterial(0, ReactorMat);
	}
}

void AProceduralInteriorGeometry::CreateCatwalkGeometry()
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> VertexColors;
	TArray<FProcMeshTangent> Tangents;

	// Create catwalk in corridor
	float CatwalkWidth = 100.0f * InteriorScale;
	float CatwalkThickness = 5.0f * InteriorScale;
	float CatwalkLength = 500.0f * InteriorScale;
	FVector CatwalkCenter = FVector(-600, 0, 120) * InteriorScale;

	// Catwalk platform
	CreateBox(Vertices, Triangles, Normals, UVs,
		CatwalkCenter,
		FVector(CatwalkLength, CatwalkWidth, CatwalkThickness));

	// Support beams (4 corners)
	float BeamSize = 8.0f * InteriorScale;
	float BeamHeight = 120.0f * InteriorScale;

	CreateBox(Vertices, Triangles, Normals, UVs,
		FVector(-600, -CatwalkWidth * 0.4f, BeamHeight * 0.5f) * InteriorScale,
		FVector(BeamSize, BeamSize, BeamHeight));

	CreateBox(Vertices, Triangles, Normals, UVs,
		FVector(-600, CatwalkWidth * 0.4f, BeamHeight * 0.5f) * InteriorScale,
		FVector(BeamSize, BeamSize, BeamHeight));

	// Railings
	float RailingHeight = 100.0f * InteriorScale;
	float RailingThickness = 3.0f * InteriorScale;

	// Left railing
	CreateBox(Vertices, Triangles, Normals, UVs,
		CatwalkCenter + FVector(0, -CatwalkWidth * 0.5f, RailingHeight * 0.5f),
		FVector(CatwalkLength, RailingThickness, RailingHeight));

	// Right railing
	CreateBox(Vertices, Triangles, Normals, UVs,
		CatwalkCenter + FVector(0, CatwalkWidth * 0.5f, RailingHeight * 0.5f),
		FVector(CatwalkLength, RailingThickness, RailingHeight));

	Catwalks->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, Tangents, true);

	// Apply material
	UMaterialInterface* BaseMaterial = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial"));
	if (BaseMaterial)
	{
		UMaterialInstanceDynamic* CatwalkMat = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		CatwalkMat->SetVectorParameterValue(TEXT("Color"), FLinearColor(0.35f, 0.35f, 0.37f, 1.0f));
		Catwalks->SetMaterial(0, CatwalkMat);
	}
}

void AProceduralInteriorGeometry::SpawnLights()
{
	// Clear existing lights
	for (UPointLightComponent* Light : InteriorLights)
	{
		if (Light)
		{
			Light->DestroyComponent();
		}
	}
	InteriorLights.Empty();

	auto CreateLight = [this](FVector Position, FLinearColor Color, float Intensity, float Radius) -> UPointLightComponent*
	{
		UPointLightComponent* Light = NewObject<UPointLightComponent>(this);
		Light->RegisterComponent();
		Light->AttachToComponent(InteriorRoot, FAttachmentTransformRules::KeepRelativeTransform);
		Light->SetRelativeLocation(Position);
		Light->SetLightColor(Color);
		Light->SetIntensity(Intensity);
		Light->SetAttenuationRadius(Radius);
		Light->SetCastShadows(false); // Performance
		InteriorLights.Add(Light);
		return Light;
	};

	// Overhead cockpit lights (white/cool) - BRIGHTER
	CreateLight(FVector(0, 0, 220) * InteriorScale, FLinearColor(0.9f, 0.95f, 1.0f), 8000.0f, 800.0f * InteriorScale);
	CreateLight(FVector(-150, -150, 200) * InteriorScale, FLinearColor(0.85f, 0.9f, 1.0f), 5000.0f, 600.0f * InteriorScale);
	CreateLight(FVector(-150, 150, 200) * InteriorScale, FLinearColor(0.85f, 0.9f, 1.0f), 5000.0f, 600.0f * InteriorScale);

	// Console accent lights (cyan + orange) - STRONGER
	CreateLight(FVector(-200, -150, 90) * InteriorScale, FLinearColor(0.3f, 0.9f, 1.0f), 3000.0f, 400.0f * InteriorScale);
	CreateLight(FVector(-200, 150, 90) * InteriorScale, FLinearColor(0.3f, 0.9f, 1.0f), 3000.0f, 400.0f * InteriorScale);
	CreateLight(FVector(150, -150, 90) * InteriorScale, FLinearColor(1.0f, 0.7f, 0.3f), 3000.0f, 400.0f * InteriorScale);
	CreateLight(FVector(150, 150, 90) * InteriorScale, FLinearColor(1.0f, 0.7f, 0.3f), 3000.0f, 400.0f * InteriorScale);

	// Reactor glow (bright blue)
	CreateLight(FVector(-800, 0, 150) * InteriorScale, FLinearColor(0.3f, 0.7f, 1.0f), 8000.0f, 1000.0f * InteriorScale);

	// Emergency strip lights (dim red)
	CreateLight(FVector(-300, 0, 10) * InteriorScale, FLinearColor(1.0f, 0.1f, 0.1f), 500.0f, 400.0f * InteriorScale);
	CreateLight(FVector(300, 0, 10) * InteriorScale, FLinearColor(1.0f, 0.1f, 0.1f), 500.0f, 400.0f * InteriorScale);

	UE_LOG(LogTemp, Warning, TEXT("💡 Spawned %d interior lights"), InteriorLights.Num());
}
