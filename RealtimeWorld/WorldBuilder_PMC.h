// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
// #include "RealtimeMeshCollision.h"
#include "Components/BoxComponent.h"
#include "Chunker_PMC.h"
#include "WorldBuilder_PMC.generated.h"

class AWorldGrid;
class APC;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TERRSHALOM_API AWorldBuilder_PMC : public AActor
{
	GENERATED_BODY()
public:
	AWorldBuilder_PMC();
private:
	/** Whether we need to rebuild or not. */
	int32 bRebuild : 1;

	UFUNCTION()
		void OverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
protected:
	//UPROPERTY(ReplicatedUsing = OnRep_RealtimeMesh, VisibleAnywhere, BlueprintReadOnly, Meta = (ExposeFunctionCategories = "Mesh,Rendering,Physics,Components|RealtimeMesh", AllowPrivateAccess = "true"))

	/** RepNotify for changes made to current health.*/
	//UFUNCTION(Server, Reliable, WithValidation)
	//	void OnRep_RealtimeMesh();
	virtual void BeginPlay() override;
public:
		AWorldGrid* mapRef;

	UPROPERTY(Category = Map, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UChunker_PMC* Chunks;

	UPROPERTY(Category = Map, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		class UStaticMeshComponent* baseMesh;

	UPROPERTY(EditAnywhere)
		class UInstancedStaticMeshComponent* Walls;

	UPROPERTY(Category = Bounds, VisibleAnywhere, BlueprintReadOnly)
		UBoxComponent* leftBds;

	UPROPERTY(Category = Bounds, VisibleAnywhere, BlueprintReadOnly)
		UBoxComponent* topBds;

	UPROPERTY(Category = Bounds, VisibleAnywhere, BlueprintReadOnly)
		UBoxComponent* rightBds;

	UPROPERTY(Category = Bounds, VisibleAnywhere, BlueprintReadOnly)
		UBoxComponent* botBds;

	UPROPERTY(EditAnywhere)
		int32 chunkSize;
		float chunkEdge;

	UPROPERTY()
		TSet<APC*> Players;

	virtual void Tick(float DeltaTime);

	void initWorldBuilder(AWorldGrid* map);
};