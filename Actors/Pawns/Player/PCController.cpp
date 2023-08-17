
#include "PCController.h"
#include "../../../RealtimeWorld/WorldBuilder_PMC.h"

namespace FMathPerlinHelpers
{
	const int16 MAP_CHUNKS = 16;
	// const uint16 MAP_CHUNKS = 65535 / 2;

	// random permutation of 256 numbers, repeated 2x
	static const int32 Permutation[512] = {
		63, 9, 212, 205, 31, 128, 72, 59, 137, 203, 195, 170, 181, 115, 165, 40, 116, 139, 175, 225, 132, 99, 222, 2, 41, 15, 197, 93, 169, 90, 228, 43, 221, 38, 206, 204, 73, 17, 97, 10, 96, 47, 32, 138, 136, 30, 219,
		78, 224, 13, 193, 88, 134, 211, 7, 112, 176, 19, 106, 83, 75, 217, 85, 0, 98, 140, 229, 80, 118, 151, 117, 251, 103, 242, 81, 238, 172, 82, 110, 4, 227, 77, 243, 46, 12, 189, 34, 188, 200, 161, 68, 76, 171, 194,
		57, 48, 247, 233, 51, 105, 5, 23, 42, 50, 216, 45, 239, 148, 249, 84, 70, 125, 108, 241, 62, 66, 64, 240, 173, 185, 250, 49, 6, 37, 26, 21, 244, 60, 223, 255, 16, 145, 27, 109, 58, 102, 142, 253, 120, 149, 160,
		124, 156, 79, 186, 135, 127, 14, 121, 22, 65, 54, 153, 91, 213, 174, 24, 252, 131, 192, 190, 202, 208, 35, 94, 231, 56, 95, 183, 163, 111, 147, 25, 67, 36, 92, 236, 71, 166, 1, 187, 100, 130, 143, 237, 178, 158,
		104, 184, 159, 177, 52, 214, 230, 119, 87, 114, 201, 179, 198, 3, 248, 182, 39, 11, 152, 196, 113, 20, 232, 69, 141, 207, 234, 53, 86, 180, 226, 74, 150, 218, 29, 133, 8, 44, 123, 28, 146, 89, 101, 154, 220, 126,
		155, 122, 210, 168, 254, 162, 129, 33, 18, 209, 61, 191, 199, 157, 245, 55, 164, 167, 215, 246, 144, 107, 235,

		63, 9, 212, 205, 31, 128, 72, 59, 137, 203, 195, 170, 181, 115, 165, 40, 116, 139, 175, 225, 132, 99, 222, 2, 41, 15, 197, 93, 169, 90, 228, 43, 221, 38, 206, 204, 73, 17, 97, 10, 96, 47, 32, 138, 136, 30, 219,
		78, 224, 13, 193, 88, 134, 211, 7, 112, 176, 19, 106, 83, 75, 217, 85, 0, 98, 140, 229, 80, 118, 151, 117, 251, 103, 242, 81, 238, 172, 82, 110, 4, 227, 77, 243, 46, 12, 189, 34, 188, 200, 161, 68, 76, 171, 194,
		57, 48, 247, 233, 51, 105, 5, 23, 42, 50, 216, 45, 239, 148, 249, 84, 70, 125, 108, 241, 62, 66, 64, 240, 173, 185, 250, 49, 6, 37, 26, 21, 244, 60, 223, 255, 16, 145, 27, 109, 58, 102, 142, 253, 120, 149, 160,
		124, 156, 79, 186, 135, 127, 14, 121, 22, 65, 54, 153, 91, 213, 174, 24, 252, 131, 192, 190, 202, 208, 35, 94, 231, 56, 95, 183, 163, 111, 147, 25, 67, 36, 92, 236, 71, 166, 1, 187, 100, 130, 143, 237, 178, 158,
		104, 184, 159, 177, 52, 214, 230, 119, 87, 114, 201, 179, 198, 3, 248, 182, 39, 11, 152, 196, 113, 20, 232, 69, 141, 207, 234, 53, 86, 180, 226, 74, 150, 218, 29, 133, 8, 44, 123, 28, 146, 89, 101, 154, 220, 126,
		155, 122, 210, 168, 254, 162, 129, 33, 18, 209, 61, 191, 199, 157, 245, 55, 164, 167, 215, 246, 144, 107, 235 };

	// Gradient functions for 1D, 2D and 3D Perlin noise

	FORCEINLINE float Grad1(int32 Hash, float X)
	{
		// Slicing Perlin's 3D improved noise would give us only scales of -1, 0 and 1; this looks pretty bad so let's use a different sampling
		static const float Grad1Scales[16] = { -8 / 8, -7 / 8., -6 / 8., -5 / 8., -4 / 8., -3 / 8., -2 / 8., -1 / 8., 1 / 8., 2 / 8., 3 / 8., 4 / 8., 5 / 8., 6 / 8., 7 / 8., 8 / 8 };
		return Grad1Scales[Hash & 15] * X;
	}

	// Note: If you change the Grad2 or Grad3 functions, check that you don't change the range of the resulting noise as well; it should be (within floating point error) in the range of (-1, 1)
	FORCEINLINE float Grad2(int32 Hash, float X, float Y)
	{
		// corners and major axes (similar to the z=0 projection of the cube-edge-midpoint sampling from improved Perlin noise)
		switch (Hash & 7)
		{
		case 0:
			return X;
		case 1:
			return X + Y;
		case 2:
			return Y;
		case 3:
			return -X + Y;
		case 4:
			return -X;
		case 5:
			return -X - Y;
		case 6:
			return -Y;
		case 7:
			return X - Y;
			// can't happen
		default:
			return 0;
		}
	}

	FORCEINLINE float Grad3(int32 Hash, float X, float Y, float Z)
	{
		switch (Hash & 15)
		{
			// 12 cube midpoints
		case 0:
			return X + Z;
		case 1:
			return X + Y;
		case 2:
			return Y + Z;
		case 3:
			return -X + Y;
		case 4:
			return -X + Z;
		case 5:
			return -X - Y;
		case 6:
			return -Y + Z;
		case 7:
			return X - Y;
		case 8:
			return X - Z;
		case 9:
			return Y - Z;
		case 10:
			return -X - Z;
		case 11:
			return -Y - Z;
			// 4 vertices of regular tetrahedron
		case 12:
			return X + Y;
		case 13:
			return -X + Y;
		case 14:
			return -Y + Z;
		case 15:
			return -Y - Z;
			// can't happen
		default:
			return 0;
		}
	}

	// Curve w/ second derivative vanishing at 0 and 1, from Perlin's improved noise paper
	FORCEINLINE float SmoothCurve(float X)
	{
		return X * X * X * (X * (X * 6.0f - 15.0f) + 10.0f);
	}

	FORCEINLINE int32 szudzikPair(int32 x, int32 y)
	{
		x += MAP_CHUNKS;
		y += MAP_CHUNKS;
		return (x >= y ? (x * x) + x + y : (y * y) + x);
	}

	FORCEINLINE FVector2D szudzikPair(int32 sectionId)
	{
		int32 sqrtzy = FMath::Floor(FMath::Sqrt(float(sectionId)));
		int32 sqz = FMath::Square(sqrtzy);

		return ((sectionId - sqz) >= sqrtzy) ? FVector2D(sqrtzy - MAP_CHUNKS, (sectionId - sqz - sqrtzy) - MAP_CHUNKS) : FVector2D((sectionId - sqz) - MAP_CHUNKS, sqrtzy - MAP_CHUNKS);
	}
};

using namespace FMathPerlinHelpers;

// Called every frame
void APCController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APCController::traceAction(FString type) {
	// eventually toggle thing
	FVector Location;
	FRotator Rotation;
	GetPlayerViewPoint(Location, Rotation);
	//	const AActor* Camera = PlayerController->GetViewTarget();
	//	Camera->GetActorEyesViewPoint(Location, Rotation);
	const FVector TraceStart = Location;
	const FVector TraceEnd = TraceStart + Rotation.Vector() * 1000.f;

	// AActor* Camera = PlayerController->GetViewTarget();
	const AActor* ActorToIgnore = nullptr;

	//	UE_LOG(QuickStart, Log, TEXT("ARayTracing: TraceStart [%f, %f, %f]"), TraceStart.X, TraceStart.Y, TraceStart.Z);
	//	UE_LOG(QuickStart, Log, TEXT("ARayTracing: TraceEnd [%f, %f, %f]"), TraceEnd.X, TraceEnd.Y, TraceEnd.Z);

	FCollisionQueryParams TraceParams(FName(TEXT("RayTracing")), true, ActorToIgnore);
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnFaceIndex = true;

	FHitResult HitResult(ForceInit);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Camera, TraceParams);

	if (bHit && HitResult.FaceIndex != -1) {
		//switch (type) {
		//case "Dig":
			const AWorldBuilder_PMC* Hit_Actor = Cast<AWorldBuilder_PMC>(HitResult.GetActor());
			if (Hit_Actor && Hit_Actor->IsA(AWorldBuilder_PMC::StaticClass())) {
				UE_LOG(LogTemp, Log, TEXT("ARayTracing: LineTraceSingleByChannel bHit=%d Comp=%x faceIndex=%i"), bHit, HitResult.GetActor(), HitResult.FaceIndex);
			}
			//break;
		
		//HitResult.GetComponent()->AddLocalRotation(FRotator(1.0f, 0, 0));
		//UE_LOG(LogTemp, Log, TEXT("ARayTracing: LineTraceSingleByChannel bHit=%d Comp=%x faceIndex=%i"), bHit, HitResult.GetActor(), HitResult.FaceIndex);
	}
}

void APCController::Initialize_Implementation(AWorldGrid* map, UChunker_PMC* chunk, FVector offset)
{
	// get world_builder reference by name in getactorsbyclass (from world), cannot copy pointer between server -> client
	if (!chunk) {
		UE_LOG(LogTemp, Warning, TEXT("Chunk does not exist in the map"));
		return;
	}
	map->setMaterial(chunk);

	if (map->HeightMap.hMap.IsEmpty()) {
		map->HeightMap.Init(MAP_CHUNKS * (map->chunkSize / map->mapResolution));
	}

	FVector spawnLoc = offset / map->mapResolution;
	chunk->mapRef = map;
	chunk->NoiseSamplesPerLine = map->chunkSize / map->mapResolution + 1;
	chunk->VerticesArraySize = chunk->NoiseSamplesPerLine * chunk->NoiseSamplesPerLine;
	chunk->sectionOffset = FVector2D(spawnLoc.X, spawnLoc.Y);

	//AppendBoxMesh(chunk, FVector(100.f), FTransform::Identity);
	//chunk->SetRelativeLocation(FVector(0.0f, 0.0f, 14000.0f));
	////SetRelativeScale3D(FVector(100.f));
	//return;
	int MaxLOD = 0; // 2; This is not specific to LOD, but of the sections ON this current procedural mesh
	chunk->MinX = chunk->MinY = MAP_CHUNKS;
	chunk->MaxX = chunk->MaxY = MAP_CHUNKS;
	chunk->MinValue = -1;
	chunk->MaxValue = 1;
	chunk->PointsSX = 128;
	chunk->PointsSY = chunk->PointsSX;
	chunk->SizeX = map->chunkSize;
	chunk->SizeY = map->chunkSize;
	chunk->SizeZ = map->chunkSize / 5;
	//chunk->parentOffset = offset;

	//LODKeys.Reserve(MaxLOD);
	chunk->initMeshData();

	//chunk->AddRelativeLocation(-offset);

	for (int LODIndex = 0; LODIndex <= MaxLOD; LODIndex++)
	{
		FRealtimeMeshSectionConfig Properties = FRealtimeMeshSectionConfig(ERealtimeMeshSectionDrawType::Dynamic, 0);
		Properties.bCastsShadow = true;
		Properties.bIsVisible = true;
		chunk->CreateMeshSection(LODIndex, chunk->Vertices, chunk->Triangles, chunk->Normals, chunk->UV0, chunk->VertexColors, chunk->Tangents, true);
	}
	chunk->CalculateBounds();
}

void APCController::ServerToClient_Implementation(AWorldGrid* map, UChunker_PMC* chunk, FVector offset)
{
	if (!chunk) {
		UE_LOG(LogTemp, Warning, TEXT("Chunk does not exist in the map"));
		return;
	}
	map->setMaterial(chunk);

	if (map->HeightMap.hMap.IsEmpty()) {
		map->HeightMap.Init(MAP_CHUNKS * (map->chunkSize / map->mapResolution));
	}

	FVector spawnLoc = offset / map->mapResolution;
	chunk->mapRef = map;
	chunk->NoiseSamplesPerLine = map->chunkSize / map->mapResolution + 1;
	chunk->VerticesArraySize = chunk->NoiseSamplesPerLine * chunk->NoiseSamplesPerLine;
	chunk->sectionOffset = FVector2D(spawnLoc.X, spawnLoc.Y);

	//AppendBoxMesh(chunk, FVector(100.f), FTransform::Identity);
	//chunk->SetRelativeLocation(FVector(0.0f, 0.0f, 14000.0f));
	////SetRelativeScale3D(FVector(100.f));
	//return;
	int MaxLOD = 0; // 2; This is not specific to LOD, but of the sections ON this current procedural mesh
	chunk->MinX = chunk->MinY = MAP_CHUNKS;
	chunk->MaxX = chunk->MaxY = MAP_CHUNKS;
	chunk->MinValue = -1;
	chunk->MaxValue = 1;
	chunk->PointsSX = 128;
	chunk->PointsSY = chunk->PointsSX;
	chunk->SizeX = map->chunkSize;
	chunk->SizeY = map->chunkSize;
	chunk->SizeZ = map->chunkSize / 5;
	//chunk->parentOffset = offset;

	//LODKeys.Reserve(MaxLOD);
	chunk->initMeshData();

	//chunk->AddRelativeLocation(-offset);

	for (int LODIndex = 0; LODIndex <= MaxLOD; LODIndex++)
	{
		FRealtimeMeshSectionConfig Properties = FRealtimeMeshSectionConfig(ERealtimeMeshSectionDrawType::Dynamic, 0);
		Properties.bCastsShadow = true;
		Properties.bIsVisible = true;
		chunk->CreateMeshSection(LODIndex, chunk->Vertices, chunk->Triangles, chunk->Normals, chunk->UV0, chunk->VertexColors, chunk->Tangents, true);
	}
	chunk->CalculateBounds();
	Initialize(map, chunk, offset);
}


void APCController::Interact_Implementation(UChunker_PMC* chunk, int32 faceIndex, float strength, bool isAdd)
{
	if (!chunk->interactLock) {
		if (chunk->Vertices.Num() == 0) {
			return;
		}

		chunk->interactLock = true;
		const int32 thisTri = (int32)faceIndex * 3;

		const int32 addOrSubtract = isAdd ? 1 : -1;
		const float baseChange = strength * addOrSubtract;

		chunk->Vertices[chunk->Triangles[thisTri]].Z += baseChange + chunk->mapRef->randGen.RandRange(-strength, strength);
		chunk->Vertices[chunk->Triangles[thisTri] + 1].Z += baseChange + chunk->mapRef->randGen.RandRange(-strength, strength);
		chunk->Vertices[chunk->Triangles[thisTri] + 2].Z += baseChange + chunk->mapRef->randGen.RandRange(-strength, strength);

		int MaxLOD = 0; // 2;
		for (int LODIndex = 0; LODIndex <= MaxLOD; LODIndex++)
		{
			chunk->UpdateMeshSection(LODIndex, chunk->Vertices, chunk->Normals, chunk->UV0, chunk->VertexColors, chunk->Tangents);
			// Save Landscape Changes to object / file for later loading
			// Broadcast these changes to all clients
		}
		// OnGenerateMesh(this);
		//InteractClient_Implementation();
		chunk->interactLock = false;
	}
}

void ConvertQuadToTrianglesPMC(TArray<int32>& Triangles, int32 Vert0, int32 Vert1, int32 Vert2, int32 Vert3)
{
	Triangles.Add(Vert0);
	Triangles.Add(Vert1);
	Triangles.Add(Vert3);

	Triangles.Add(Vert1);
	Triangles.Add(Vert2);
	Triangles.Add(Vert3);
}

void WriteQuadPositions(TArray<FVector>& Vertices, const FVector& VertA, const FVector& VertB, const FVector& VertC, const FVector& VertD) {
	Vertices.Add(VertA);
	Vertices.Add(VertB);
	Vertices.Add(VertC);
	Vertices.Add(VertD);
}

void APCController::AppendBoxMesh(UChunker_PMC* chunk, FVector BoxRadius, FTransform BoxTransform)
{
	// Generate verts
	FVector BoxVerts[8];
	BoxVerts[0] = BoxTransform.TransformPosition(FVector(-BoxRadius.X, BoxRadius.Y, BoxRadius.Z));
	BoxVerts[1] = BoxTransform.TransformPosition(FVector(BoxRadius.X, BoxRadius.Y, BoxRadius.Z));
	BoxVerts[2] = BoxTransform.TransformPosition(FVector(BoxRadius.X, -BoxRadius.Y, BoxRadius.Z));
	BoxVerts[3] = BoxTransform.TransformPosition(FVector(-BoxRadius.X, -BoxRadius.Y, BoxRadius.Z));

	BoxVerts[4] = BoxTransform.TransformPosition(FVector(-BoxRadius.X, BoxRadius.Y, -BoxRadius.Z));
	BoxVerts[5] = BoxTransform.TransformPosition(FVector(BoxRadius.X, BoxRadius.Y, -BoxRadius.Z));
	BoxVerts[6] = BoxTransform.TransformPosition(FVector(BoxRadius.X, -BoxRadius.Y, -BoxRadius.Z));
	BoxVerts[7] = BoxTransform.TransformPosition(FVector(-BoxRadius.X, -BoxRadius.Y, -BoxRadius.Z));

	// Generate triangles (from quads)
	const int32 StartVertex = chunk->Vertices.Num();
	const int32 NumVerts = 24; // 6 faces x 4 verts per face
	const int32 NumIndices = 36;

	// Make sure the secondary arrays are the same length, zeroing them if necessary
	chunk->Normals.SetNumZeroed(StartVertex);
	chunk->Tangents.SetNumZeroed(StartVertex);
	chunk->UV0.SetNumZeroed(StartVertex);

	chunk->Vertices.Reserve(StartVertex + NumVerts);
	chunk->Normals.Reserve(StartVertex + NumVerts);
	chunk->Tangents.Reserve(StartVertex + NumVerts);
	chunk->UV0.Reserve(StartVertex + NumVerts);
	chunk->Triangles.Reserve(chunk->Triangles.Num() + NumIndices);

	const auto WriteToNextFour = [](TArray<FVector>& Array, const FVector& Value)
	{
		Array.Add(Value);
		Array.Add(Value);
		Array.Add(Value);
		Array.Add(Value);
	};

	const auto WriteToNextFourTangent = [](TArray<FProcMeshTangent>& Array, const FProcMeshTangent& Value)
	{
		Array.Add(Value);
		Array.Add(Value);
		Array.Add(Value);
		Array.Add(Value);
	};

	WriteQuadPositions(chunk->Vertices, BoxVerts[0], BoxVerts[1], BoxVerts[2], BoxVerts[3]);
	WriteToNextFour(chunk->Normals, BoxTransform.TransformVectorNoScale(FVector(0.0f, 0.0f, 1.0f)));
	WriteToNextFourTangent(chunk->Tangents, FProcMeshTangent(BoxTransform.TransformVectorNoScale(FVector(0.0f, -1.0f, 0.0f)), false));
	ConvertQuadToTrianglesPMC(chunk->Triangles, StartVertex + 0, StartVertex + 1, StartVertex + 2, StartVertex + 3);

	WriteQuadPositions(chunk->Vertices, BoxVerts[4], BoxVerts[0], BoxVerts[3], BoxVerts[7]);
	WriteToNextFour(chunk->Normals, BoxTransform.TransformVectorNoScale(FVector(-1.0, 0.0, 0.0)));
	WriteToNextFourTangent(chunk->Tangents, FProcMeshTangent(BoxTransform.TransformVectorNoScale(FVector(0.0f, -1.0f, 0.0f)), false));
	ConvertQuadToTrianglesPMC(chunk->Triangles, StartVertex + 4, StartVertex + 5, StartVertex + 6, StartVertex + 7);

	WriteQuadPositions(chunk->Vertices, BoxVerts[5], BoxVerts[1], BoxVerts[0], BoxVerts[4]);
	WriteToNextFour(chunk->Normals, BoxTransform.TransformVectorNoScale(FVector(0.0, 1.0, 0.0)));
	WriteToNextFourTangent(chunk->Tangents, FProcMeshTangent(BoxTransform.TransformVectorNoScale(FVector(-1.0f, 0.0f, 0.0f)), false));
	ConvertQuadToTrianglesPMC(chunk->Triangles, StartVertex + 8, StartVertex + 9, StartVertex + 10, StartVertex + 11);

	WriteQuadPositions(chunk->Vertices, BoxVerts[6], BoxVerts[2], BoxVerts[1], BoxVerts[5]);
	WriteToNextFour(chunk->Normals, BoxTransform.TransformVectorNoScale(FVector(1.0, 0.0, 0.0)));
	WriteToNextFourTangent(chunk->Tangents, FProcMeshTangent(BoxTransform.TransformVectorNoScale(FVector(0.0f, 1.0f, 0.0f)), false));
	ConvertQuadToTrianglesPMC(chunk->Triangles, StartVertex + 12, StartVertex + 13, StartVertex + 14, StartVertex + 15);

	WriteQuadPositions(chunk->Vertices, BoxVerts[7], BoxVerts[3], BoxVerts[2], BoxVerts[6]);
	WriteToNextFour(chunk->Normals, BoxTransform.TransformVectorNoScale(FVector(0.0, -1.0, 0.0)));
	WriteToNextFourTangent(chunk->Tangents, FProcMeshTangent(BoxTransform.TransformVectorNoScale(FVector(1.0f, 0.0f, 0.0f)), false));
	ConvertQuadToTrianglesPMC(chunk->Triangles, StartVertex + 16, StartVertex + 17, StartVertex + 18, StartVertex + 19);

	WriteQuadPositions(chunk->Vertices, BoxVerts[7], BoxVerts[6], BoxVerts[5], BoxVerts[4]);
	WriteToNextFour(chunk->Normals, BoxTransform.TransformVectorNoScale(FVector(0.0, 0.0, -1.0)));
	WriteToNextFourTangent(chunk->Tangents, FProcMeshTangent(BoxTransform.TransformVectorNoScale(FVector(0.0f, 1.0f, 0.0f)), false));
	ConvertQuadToTrianglesPMC(chunk->Triangles, StartVertex + 20, StartVertex + 21, StartVertex + 22, StartVertex + 23);

	// UVs
	for (int32 Index = 0; Index < 6; Index++)
	{
		chunk->UV0.Add(FVector2D(0.0f, 0.0f));
		chunk->UV0.Add(FVector2D(0.0f, 1.0f));
		chunk->UV0.Add(FVector2D(1.0f, 1.0f));
		chunk->UV0.Add(FVector2D(1.0f, 0.0f));
	}

	chunk->CreateMeshSection(0, chunk->Vertices, chunk->Triangles, chunk->Normals, chunk->UV0, chunk->VertexColors, chunk->Tangents, true);
}
