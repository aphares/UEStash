
#include "PCController.h"
#include "../../../RealtimeWorld/WorldBuilder.h"

// Called every frame
void APCController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APCController::traceAction(FString type)
{
	// eventually toggle thing
	FVector Location;
	FRotator Rotation;
	GetPlayerViewPoint(Location, Rotation);
	//	const AActor* Camera = PlayerController->GetViewTarget();
	//	Camera->GetActorEyesViewPoint(Location, Rotation);
	const FVector TraceStart = Location;
	const FVector TraceEnd = TraceStart + Rotation.Vector() * 1000.f;

	// AActor* Camera = PlayerController->GetViewTarget();
	const AActor *ActorToIgnore = nullptr;

	//	UE_LOG(QuickStart, Log, TEXT("ARayTracing: TraceStart [%f, %f, %f]"), TraceStart.X, TraceStart.Y, TraceStart.Z);
	//	UE_LOG(QuickStart, Log, TEXT("ARayTracing: TraceEnd [%f, %f, %f]"), TraceEnd.X, TraceEnd.Y, TraceEnd.Z);

	FCollisionQueryParams TraceParams(FName(TEXT("RayTracing")), true, ActorToIgnore);
	TraceParams.bReturnPhysicalMaterial = false;
	TraceParams.bTraceComplex = true;
	TraceParams.bReturnFaceIndex = true;

	FHitResult HitResult(ForceInit);

	const bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Camera, TraceParams);

	if (bHit && HitResult.FaceIndex != -1)
	{
		// switch (type) {
		// case "Dig":
		const AWorldBuilder *Hit_Actor = Cast<AWorldBuilder>(HitResult.GetActor());
		if (Hit_Actor && Hit_Actor->IsA(AWorldBuilder::StaticClass()))
		{
			UE_LOG(LogTemp, Log, TEXT("ARayTracing: LineTraceSingleByChannel bHit=%d Comp=%x faceIndex=%i"), bHit, HitResult.GetActor(), HitResult.FaceIndex);
		}
		// break;

		// HitResult.GetComponent()->AddLocalRotation(FRotator(1.0f, 0, 0));
		// UE_LOG(LogTemp, Log, TEXT("ARayTracing: LineTraceSingleByChannel bHit=%d Comp=%x faceIndex=%i"), bHit, HitResult.GetActor(), HitResult.FaceIndex);
	}
}

void APCController::Initialize_Implementation(AWorldGrid *map, AWorldBuilder *chunk, FVector offset)
{
	// get world_builder reference by name in getactorsbyclass (from world), cannot copy pointer between server -> client
	if (!chunk) {
		UE_LOG(LogTemp, Warning, TEXT("Chunk does not exist in the map"));
		return;
	}

	if (map->HeightMap.hMap.IsEmpty()) {
		map->HeightMap.Init(MAP_CHUNKS * (map->chunkSize / map->mapResolution));
	}

	chunk->mapRef = map;
	chunk->Initialize(offset);
	map->setMaterial(chunk->realMesh);
	//chunk->realMesh = chunk->GetRealtimeMeshComponent()->InitializeRealtimeMesh<URealtimeMeshSimple>();
	//AppendBoxMesh(chunk, FVector(5000.f), FTransform::Identity, offset);
	//FRealtimeMeshSectionConfig Properties = FRealtimeMeshSectionConfig(ERealtimeMeshSectionDrawType::Dynamic, 0);
	//Properties.bCastsShadow = true;
	//Properties.bIsVisible = true;
	//chunk->LODKeys.Add(chunk->realMesh->CreateMeshSection(FRealtimeMeshLODKey(0), Properties, chunk->meshData, true));

}

void APCController::ServerToClient_Implementation(AWorldGrid *map, AWorldBuilder *chunk, FVector offset)
{
 	if (!chunk) {
		UE_LOG(LogTemp, Warning, TEXT("Chunk does not exist in the map"));
		return;
	}

	if (map->HeightMap.hMap.IsEmpty()) {
		map->HeightMap.Init(MAP_CHUNKS * (map->chunkSize / map->mapResolution));
	}

	chunk->mapRef = map;
	chunk->Initialize(offset);
	Initialize(map, chunk, offset);
	map->setMaterial(chunk->realMesh);
}

void APCController::Interact_Implementation(AWorldBuilder *chunk, int32 faceIndex, float strength, bool isAdd)
{
	if (!chunk->interactLock)
	{
		if (chunk->meshData.Positions.Num() == 0)
		{
			return;
		}

		chunk->interactLock = true;
		const int32 thisTri = (int32)faceIndex * 3;

		const int32 addOrSubtract = isAdd ? 1 : -1;
		const float baseChange = strength * addOrSubtract;

		chunk->meshData.Positions[chunk->meshData.Triangles[thisTri]].Z += baseChange + chunk->mapRef->randGen.RandRange(-strength, strength);
		chunk->meshData.Positions[chunk->meshData.Triangles[thisTri] + 1].Z += baseChange + chunk->mapRef->randGen.RandRange(-strength, strength);
		chunk->meshData.Positions[chunk->meshData.Triangles[thisTri] + 2].Z += baseChange + chunk->mapRef->randGen.RandRange(-strength, strength);

		int MaxLOD = 0; // 2;
		for (int LODIndex = 0; LODIndex <= MaxLOD; LODIndex++)
		{
			chunk->realMesh->UpdateSectionMesh(chunk->LODKeys[LODIndex], chunk->meshData);
			// Save Landscape Changes to object / file for later loading
			// Broadcast these changes to all clients
		}
		// OnGenerateMesh(this);
		// InteractClient_Implementation();
		chunk->interactLock = false;
	}
}

void ConvertQuadToTrianglesPMC(TArray<int32> &Triangles, int32 Vert0, int32 Vert1, int32 Vert2, int32 Vert3)
{
	Triangles.Add(Vert0);
	Triangles.Add(Vert1);
	Triangles.Add(Vert3);

	Triangles.Add(Vert1);
	Triangles.Add(Vert2);
	Triangles.Add(Vert3);
}

void WriteQuadPositions(TArray<FVector> &Vertices, const FVector &VertA, const FVector &VertB, const FVector &VertC, const FVector &VertD)
{
	Vertices.Add(VertA);
	Vertices.Add(VertB);
	Vertices.Add(VertC);
	Vertices.Add(VertD);
}

 void APCController::AppendBoxMesh(AWorldBuilder *chunk, FVector BoxRadius, FTransform BoxTransform, FVector offset) {
// Generate verts
FVector BoxVerts[8];
BoxVerts[0] = BoxTransform.TransformPosition(FVector(-BoxRadius.X, BoxRadius.Y, BoxRadius.Z)) + offset;
BoxVerts[1] = BoxTransform.TransformPosition(FVector(BoxRadius.X, BoxRadius.Y, BoxRadius.Z)) + offset;
BoxVerts[2] = BoxTransform.TransformPosition(FVector(BoxRadius.X, -BoxRadius.Y, BoxRadius.Z)) + offset;
BoxVerts[3] = BoxTransform.TransformPosition(FVector(-BoxRadius.X, -BoxRadius.Y, BoxRadius.Z)) + offset;

BoxVerts[4] = BoxTransform.TransformPosition(FVector(-BoxRadius.X, BoxRadius.Y, -BoxRadius.Z)) + offset;
BoxVerts[5] = BoxTransform.TransformPosition(FVector(BoxRadius.X, BoxRadius.Y, -BoxRadius.Z)) + offset;
BoxVerts[6] = BoxTransform.TransformPosition(FVector(BoxRadius.X, -BoxRadius.Y, -BoxRadius.Z)) + offset;
BoxVerts[7] = BoxTransform.TransformPosition(FVector(-BoxRadius.X, -BoxRadius.Y, -BoxRadius.Z)) + offset;

// Generate triangles (from quads)
const int32 StartVertex = chunk->meshData.Positions.Num();
const int32 NumVerts = 24; // 6 faces x 4 verts per face
const int32 NumIndices = 36;

// Make sure the secondary arrays are the same length, zeroing them if necessary
chunk->meshData.Normals.SetNumZeroed(StartVertex);
chunk->meshData.Tangents.SetNumZeroed(StartVertex);
chunk->meshData.UV0.SetNumZeroed(StartVertex);

chunk->meshData.Positions.Reserve(StartVertex + NumVerts);
chunk->meshData.Normals.Reserve(StartVertex + NumVerts);
chunk->meshData.Tangents.Reserve(StartVertex + NumVerts);
chunk->meshData.UV0.Reserve(StartVertex + NumVerts);
chunk->meshData.Triangles.Reserve(chunk->meshData.Triangles.Num() + NumIndices);

const auto WriteToNextFour = [](TArray<FVector>& Array, const FVector& Value)
{
	Array.Add(Value);
	Array.Add(Value);
	Array.Add(Value);
	Array.Add(Value);
};

WriteQuadPositions(chunk->meshData.Positions, BoxVerts[0], BoxVerts[1], BoxVerts[2], BoxVerts[3]);
WriteToNextFour(chunk->meshData.Normals, BoxTransform.TransformVectorNoScale(FVector(0.0f, 0.0f, 1.0f)));
WriteToNextFour(chunk->meshData.Tangents, BoxTransform.TransformVectorNoScale(FVector(0.0f, -1.0f, 0.0f)));
ConvertQuadToTrianglesPMC(chunk->meshData.Triangles, StartVertex + 0, StartVertex + 1, StartVertex + 2, StartVertex + 3);

WriteQuadPositions(chunk->meshData.Positions, BoxVerts[4], BoxVerts[0], BoxVerts[3], BoxVerts[7]);
WriteToNextFour(chunk->meshData.Normals, BoxTransform.TransformVectorNoScale(FVector(-1.0, 0.0, 0.0)));
WriteToNextFour(chunk->meshData.Tangents, BoxTransform.TransformVectorNoScale(FVector(0.0f, -1.0f, 0.0f)));
ConvertQuadToTrianglesPMC(chunk->meshData.Triangles, StartVertex + 4, StartVertex + 5, StartVertex + 6, StartVertex + 7);

WriteQuadPositions(chunk->meshData.Positions, BoxVerts[5], BoxVerts[1], BoxVerts[0], BoxVerts[4]);
WriteToNextFour(chunk->meshData.Normals, BoxTransform.TransformVectorNoScale(FVector(0.0, 1.0, 0.0)));
WriteToNextFour(chunk->meshData.Tangents, BoxTransform.TransformVectorNoScale(FVector(-1.0f, 0.0f, 0.0f)));
ConvertQuadToTrianglesPMC(chunk->meshData.Triangles, StartVertex + 8, StartVertex + 9, StartVertex + 10, StartVertex + 11);

WriteQuadPositions(chunk->meshData.Positions, BoxVerts[6], BoxVerts[2], BoxVerts[1], BoxVerts[5]);
WriteToNextFour(chunk->meshData.Normals, BoxTransform.TransformVectorNoScale(FVector(1.0, 0.0, 0.0)));
WriteToNextFour(chunk->meshData.Tangents, BoxTransform.TransformVectorNoScale(FVector(0.0f, 1.0f, 0.0f)));
ConvertQuadToTrianglesPMC(chunk->meshData.Triangles, StartVertex + 12, StartVertex + 13, StartVertex + 14, StartVertex + 15);

WriteQuadPositions(chunk->meshData.Positions, BoxVerts[7], BoxVerts[3], BoxVerts[2], BoxVerts[6]);
WriteToNextFour(chunk->meshData.Normals, BoxTransform.TransformVectorNoScale(FVector(0.0, -1.0, 0.0)));
WriteToNextFour(chunk->meshData.Tangents, BoxTransform.TransformVectorNoScale(FVector(1.0f, 0.0f, 0.0f)));
ConvertQuadToTrianglesPMC(chunk->meshData.Triangles, StartVertex + 16, StartVertex + 17, StartVertex + 18, StartVertex + 19);

WriteQuadPositions(chunk->meshData.Positions, BoxVerts[7], BoxVerts[6], BoxVerts[5], BoxVerts[4]);
WriteToNextFour(chunk->meshData.Normals, BoxTransform.TransformVectorNoScale(FVector(0.0, 0.0, -1.0)));
WriteToNextFour(chunk->meshData.Tangents, BoxTransform.TransformVectorNoScale(FVector(0.0f, 1.0f, 0.0f)));
ConvertQuadToTrianglesPMC(chunk->meshData.Triangles, StartVertex + 20, StartVertex + 21, StartVertex + 22, StartVertex + 23);

// UVs
for (int32 Index = 0; Index < 6; Index++)
{
	chunk->meshData.UV0.Add(FVector2D(0.0f, 0.0f));
	chunk->meshData.UV0.Add(FVector2D(0.0f, 1.0f));
	chunk->meshData.UV0.Add(FVector2D(1.0f, 1.0f));
	chunk->meshData.UV0.Add(FVector2D(1.0f, 0.0f));
}
 }
