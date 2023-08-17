// Fill out your copyright notice in the Description page of Project Settings.
#include "WorldGrid.h"
#include "../Actors/Pawns/Player/PC.h"
#include "Net/UnrealNetwork.h"
#include "WorldBuilder.h"

// Sets default values for this component's properties
AWorldGrid::AWorldGrid()
{
	//USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	//SetRootComponent(SceneComponent);
	//chunkSize = 10000;
	//chunkEdge = chunkSize * 0.01;
	//mapResolution = 200; // must be divisble
	seed = FMath::RandRange(TNumericLimits<int32>::Min(), TNumericLimits<int32>::Max());
	randGen = FRandomStream(seed);

	UE_LOG(LogStreaming, Log, TEXT("Loading TERRSHALOMGAME using seed: %d"), seed);
	//UE_LOG(LogStreaming, Log, TEXT("Deleting cells -> %d, %d from %d, %d"), xDiff_D, yDiff_D, chunkX, chunkY);
}

void AWorldGrid::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AWorldGrid::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWorldGrid::InitMap() {
	HeightMap.Init(MAP_CHUNKS * (chunkSize / mapResolution));
	TArray<int32> chunkIds;
	int32 tempLoadChunks = 1;
	for (int y = -tempLoadChunks; y < tempLoadChunks; y++) for (int x = -tempLoadChunks; x < tempLoadChunks; x++) LoadCell(x, y, NULL);
}

AWorldBuilder* AWorldGrid::LoadCell(int32 xPos, int32 yPos, APC* pRef) {
	if (FMath::Abs(xPos) > MAP_CHUNKS || FMath::Abs(yPos) > MAP_CHUNKS) return nullptr;
	int32 sectionId = szudzikPair(xPos, yPos);
	AWorldBuilder** gridElem = WorldMap.Find(sectionId);
	if (gridElem) {
		AWorldBuilder* ref = *gridElem;
		ref->Players.Emplace(pRef);
		return *gridElem;
	}
	else {
		FActorSpawnParameters SpawnInfo;
		SpawnInfo.Owner = this;
		// SpawnInfo.Name = FName(FString::Printf(TEXT("w|B%d_%d"), xPos, yPos)); CRASHES when going over a previously left area
		AWorldBuilder* Spawned = GetWorld()->SpawnActor<AWorldBuilder>(WORLD_BUILDER, FVector(xPos * chunkSize, yPos * chunkSize, 0.0f), FRotator(0.0f), SpawnInfo);
		//Spawned->SetReplicates(true);
		//Spawned->bNetLoadOnClient = true;
		//Spawned->SetReplicateMovement(true);
		//Spawned->Chunks->
		Spawned->initWorldBuilder(this);
		Spawned->Players.Emplace(pRef);
		WorldMap.Emplace(sectionId, Spawned);
		return Spawned;
	}
}

void AWorldGrid::DestroyCell(int32 xPos, int32 yPos, APC* pRef) {
	int32 sectionId = szudzikPair(xPos, yPos);
	AWorldBuilder** gridElem = WorldMap.Find(sectionId);
	if (gridElem)
	{
		AWorldBuilder* ref = *gridElem;
		//if (collLoc.X > ref->GetActorLocation().X && collLoc.X < ref->GetActorLocation().X + chunkSize && collLoc.Y > ref->GetActorLocation().Y && collLoc.Y < ref->GetActorLocation().Y + chunkSize) return;
		ref->Players.Remove(pRef);
		if (ref->Players.Num() == 0) {
			//ref->RealtimeMesh->DestroyComponent();
			//ref->Chunks->BeginDestroy();
			ref->Destroy();
			WorldMap.Remove(sectionId);
		}
	}
}

TArray<AWorldBuilder*> AWorldGrid::spawnGrid(FVector collLoc, APC* pRef)
{
	TArray<AWorldBuilder*> newChunks;
	int32 chunkX = FMath::Floor(collLoc.X / chunkSize);
	int32 chunkY = FMath::Floor(collLoc.Y / chunkSize);
	int32 useX = 0;
	int32 useY = 0;
	FVector width;
	pRef->GetActorBounds(true, collLoc, width);
	if (collLoc.X < chunkSize * (chunkX + 0.01) + width.X) {
		useX = -1;
	}
	else if (collLoc.X > chunkSize * (chunkX + 0.99) - width.X) {
		useX = 1;
	}

	if (collLoc.Y < chunkSize * (chunkY + 0.01) + width.Y) {
		useY = -1;
	}
	else if (collLoc.Y > chunkSize * (chunkY + 0.99) - width.Y) {
		useY = 1;
	}
	int8 distX = collLoc.X < chunkSize* (chunkX + chunkEdge) ? -1 : 1;
	int8 distY = collLoc.Y < chunkSize* (chunkY + chunkEdge) ? -1 : 1;

	if (useX) newChunks.Add(LoadCell(chunkX + useX, chunkY, pRef));
	if (useY) newChunks.Add(LoadCell(chunkX, chunkY + useY, pRef));
	if (useX && useY) newChunks.Add(LoadCell(chunkX + useX, chunkY + useY, pRef));

	return newChunks;
}

void AWorldGrid::despawnGrid(FVector collLoc, APC* pRef, int32 useX, int32 useY)
{
	int32 chunkX = FMath::Floor(collLoc.X / chunkSize);
	int32 chunkY = FMath::Floor(collLoc.Y / chunkSize);
	if (useX) {
		DestroyCell(chunkX + useX, chunkY + 1, pRef);
		DestroyCell(chunkX + useX, chunkY, pRef);
		DestroyCell(chunkX + useX, chunkY - 1, pRef);
	}

	if (useY) {
		DestroyCell(chunkX + 1, chunkY + useY, pRef);
		DestroyCell(chunkX, chunkY + useY, pRef);
		DestroyCell(chunkX - 1, chunkY + useY, pRef);
	}
}

int32 AWorldGrid::szudzikPair(int32 x, int32 y)
{
	x += MAP_CHUNKS;
	y += MAP_CHUNKS;
	return (x >= y ? (x * x) + x + y : (y * y) + x);
}

FVector2D AWorldGrid::szudzikPair(int32 sectionId)
{
	int32 sqrtzy = FMath::Floor(FMath::Sqrt(float(sectionId)));
	int32 sqz = FMath::Square(sqrtzy);

	return ((sectionId - sqz) >= sqrtzy) ? FVector2D(sqrtzy - MAP_CHUNKS, (sectionId - sqz - sqrtzy) - MAP_CHUNKS) : FVector2D((sectionId - sqz) - MAP_CHUNKS, sqrtzy - MAP_CHUNKS);
}

void AWorldGrid::setMaterial(URealtimeMeshSimple* setMesh) {
	switch (materialIndex)
	{
	case 2:
	case 12:
	case 22:
		setMesh->SetupMaterialSlot(0, FName("Material Interface"), DisplayMaterialInterface);
		break;
	case 3:
	case 13:
	case 23:
		setMesh->SetupMaterialSlot(0, FName("Material Instance"), DisplayMaterialInstance);
		break;
	case 4:
	case 14:
	case 24:
		setMesh->SetupMaterialSlot(0, FName("Material Instance Dynamic"), DisplayMaterialInstanceDynamic);
		break;
	case 1:
	case 11:
	case 21:
	default:
		setMesh->SetupMaterialSlot(0, FName("Material"), DisplayMaterial);
		break;
	}
}

//void AWorldGrid::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
//{
//	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//	DOREPLIFETIME(AWorldGrid, HeightMap);
//		//, Triangles, Normals, VertexColors, Tangents, UV0);
//}