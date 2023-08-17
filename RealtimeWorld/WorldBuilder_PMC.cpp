// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldBuilder_PMC.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "../Actors/Pawns/Player/PC.h"
#include "../Actors/Pawns/Player/PCController.h"
#include "WorldGrid.h"

// Sets default values for this component's properties
AWorldBuilder_PMC::AWorldBuilder_PMC()
{	
	// Set this component to be initialized 
	// when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryActorTick.bCanEverTick = true;
	bRebuild = bReplicates = bAlwaysRelevant = bNetLoadOnClient = true;
	chunkSize = 10000;
	chunkEdge = chunkSize * 0.01;

	USceneComponent* SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	Chunks = CreateDefaultSubobject<UChunker_PMC>(TEXT("ProceduralMeshComponent"));
	Chunks->SetIsReplicated(true); // Enable replication by default
	Chunks->SetupAttachment(SceneComponent);

	baseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));
	baseMesh->SetupAttachment(SceneComponent);

	//static ConstructorHelpers::FObjectFinder<UStaticMesh>MeshAsset(TEXT("/Game/BasicShapes/Cube.Cube"));
	//UStaticMesh* Asset = MeshAsset.Object;
	//baseMesh->SetStaticMesh(Asset);

	baseMesh->SetWorldLocation(FVector(0.0, 0.0, 10000.f));
	baseMesh->SetRelativeScale3D(FVector(10));

	Walls = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Walls"));
	Walls->SetupAttachment(SceneComponent);

	FVector boxSize = FVector(chunkEdge * 0.5, chunkSize * 0.5, 2048481.0f);

	leftBds = CreateDefaultSubobject<UBoxComponent>(TEXT("leftBds"));
	leftBds->SetupAttachment(SceneComponent);
	leftBds->InitBoxExtent(boxSize);
	leftBds->SetRelativeTransform(FTransform(FRotator(0.0f), FVector(chunkSize - boxSize.X, chunkSize * 0.5, 0.0f), FVector(1.0f)));
	topBds = CreateDefaultSubobject<UBoxComponent>(TEXT("topBds"));
	topBds->SetupAttachment(SceneComponent);
	topBds->InitBoxExtent(boxSize);
	topBds->SetRelativeTransform(FTransform(FRotator(0.0f, -90.0f, 0.0f), FVector(chunkSize * 0.5, chunkSize - boxSize.X, 0.0f), FVector(1.0f)));
	rightBds = CreateDefaultSubobject<UBoxComponent>(TEXT("rightBds"));
	rightBds->SetupAttachment(SceneComponent);
	rightBds->InitBoxExtent(boxSize);
	rightBds->SetRelativeTransform(FTransform(FRotator(-180.0f), FVector(boxSize.X, chunkSize * 0.5, 0.0f), FVector(1.0f)));
	botBds = CreateDefaultSubobject<UBoxComponent>(TEXT("botBds"));
	botBds->SetupAttachment(SceneComponent);
	botBds->InitBoxExtent(boxSize);
	botBds->SetRelativeTransform(FTransform(FRotator(0.0f, -270.0f, 0.0f), FVector(chunkSize * 0.5, boxSize.X, 0.0f), FVector(1.0f)));

	leftBds->OnComponentBeginOverlap.AddDynamic(this, &AWorldBuilder_PMC::OverlapBegin);
	topBds->OnComponentBeginOverlap.AddDynamic(this, &AWorldBuilder_PMC::OverlapBegin);
	rightBds->OnComponentBeginOverlap.AddDynamic(this, &AWorldBuilder_PMC::OverlapBegin);
	botBds->OnComponentBeginOverlap.AddDynamic(this, &AWorldBuilder_PMC::OverlapBegin);
	leftBds->OnComponentEndOverlap.AddDynamic(this, &AWorldBuilder_PMC::OverlapEnd);
	topBds->OnComponentEndOverlap.AddDynamic(this, &AWorldBuilder_PMC::OverlapEnd);
	rightBds->OnComponentEndOverlap.AddDynamic(this, &AWorldBuilder_PMC::OverlapEnd);
	botBds->OnComponentEndOverlap.AddDynamic(this, &AWorldBuilder_PMC::OverlapEnd);

	leftBds->bHiddenInGame = false;
	topBds->bHiddenInGame = false;
	rightBds->bHiddenInGame = false;
	botBds->bHiddenInGame = false;

	leftBds->SetEnableGravity(false);
	topBds->SetEnableGravity(false);
	rightBds->SetEnableGravity(false);
	botBds->SetEnableGravity(false);
}

void AWorldBuilder_PMC::OverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	// load mesh above
	APC* collider = Cast<APC>(OtherActor);
	if (mapRef && collider) {
		FVector collLoc = collider->GetActorLocation();
		if (collLoc.X > GetActorLocation().X && collLoc.X < GetActorLocation().X + chunkSize && collLoc.Y > GetActorLocation().Y && collLoc.Y < GetActorLocation().Y + chunkSize && collider->GetLocalRole() >= ROLE_AutonomousProxy)
		{
			TArray<AWorldBuilder_PMC*> builders = mapRef->spawnGrid(collLoc, collider);
			APCController* pcRef = Cast<APCController>(collider->GetController());

			// Plz spawn 
			for (AWorldBuilder_PMC* builder : builders) if(builder) pcRef->ServerToClient(mapRef, builder->Chunks, builder->GetActorLocation());
		}
	}
}

void AWorldBuilder_PMC::OverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	// load mesh above
	APC* collider = Cast<APC>(OtherActor);
	if (mapRef && collider) {
		FVector collLoc = collider->GetActorLocation();
		float actX = GetActorLocation().X;
		float actY = GetActorLocation().Y;
		bool isL = collLoc.X < actX + chunkSize;
		bool isT = collLoc.Y < actY + chunkSize;
		bool isR = collLoc.X > actX;
		bool isB = collLoc.Y > actY;
		float xHalf = actX + chunkSize * 0.5;
		float yHalf = actY + chunkSize * 0.5;
		if (isR && isL && isB && isT) {
			int32 useX = 0;
			int32 useY = 0;
			if (isR && collLoc.X < xHalf && collLoc.X > actX + chunkEdge){
				useX = -1;
			}
			else if (isL && collLoc.X > xHalf && collLoc.X < actX + chunkSize - chunkEdge) {
				useX = 1;
			}

			if (isB && collLoc.Y < yHalf && collLoc.Y > actY + chunkEdge) {
				useY = -1;
			}
			else if (isT && collLoc.Y > yHalf && collLoc.Y < actY + chunkSize - chunkEdge) {
				useY = 1;
			}

			if ((useX || useY) && collider->GetLocalRole() >= ROLE_AutonomousProxy) mapRef->despawnGrid(collLoc, collider, useX, useY);
		}
	}
}

void AWorldBuilder_PMC::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AWorldBuilder_PMC::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWorldBuilder_PMC::initWorldBuilder(AWorldGrid* map) {
	mapRef = map;
	//mapRef->setMaterial(Chunks);
	//Chunks->SetBounds(map, GetActorLocation() / mapRef->mapResolution);
	//Chunks->Initialize(GetActorLocation());
	//OnRealtimeMeshUpdate();
}