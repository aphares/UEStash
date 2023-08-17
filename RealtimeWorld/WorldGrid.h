// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RealtimeMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Chunker_PMC.h"
#include "WorldGrid.generated.h"

class AWorldBuilder_PMC;
class APC;

USTRUCT(Category = Map, BlueprintType, meta = (AllowPrivateAccess = "true"))
struct FHeightDataStruct {
	GENERATED_USTRUCT_BODY();

public:
	float height;

	/*ALandscape* operator[] (uint32 i) {
		return Chunks[i].Chunk;
	}

	void Add(ALandscape* chunk) {
		Chunks.Add(FChunkStruct(chunk, false));
	}

	void Set(uint32 i, FChunkStruct chunk) {
		Chunks[i] = chunk;
	}*/
};

USTRUCT(Category = Map, BlueprintType, meta = (AllowPrivateAccess = "true"))
struct FHeightMapStruct {
	GENERATED_USTRUCT_BODY();

	public:	
		TArray<TArray<float>> hMap;
		int size;
		float SEA_LEVEL = -1000.0f;
		float ORIGIN = 10000.0f - SEA_LEVEL;

		void Init(int givenSize) {
			size = givenSize;
			int totalSize = size * 2 + 1;
			float sizeScal = size;
			hMap.SetNum(totalSize);
			for (int y = -size; y <= size; y++) {
				TArray<float> row;
				row.SetNum(totalSize);
				for (int x = -size; x <= size; x++) row[x + size] = (1 - FMath::Max(FMath::Abs(x), FMath::Abs(y)) / sizeScal) * ORIGIN + SEA_LEVEL;
				hMap[y + size] = row;
			}
		}

		float Get(int x, int y) {
			return hMap[x + size][y + size];
		}

		/*ALandscape* operator[] (uint32 i) {
			return Chunks[i].Chunk;
		}

		void Add(ALandscape* chunk) {
			Chunks.Add(FChunkStruct(chunk, false));
		}

		void Set(uint32 i, FChunkStruct chunk) {
			Chunks[i] = chunk;
		}*/

		//void SetNum(uint32 num) {
		//	Chunks.SetNum(num);
		//}
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TERRSHALOM_API AWorldGrid : public AActor
{
	GENERATED_BODY()

public:
	AWorldGrid();
protected:
	virtual void BeginPlay() override;
public:
	UPROPERTY(EditAnywhere)
		int chunkSize;

	UPROPERTY(EditAnywhere)
		int chunkEdge;

	UPROPERTY(EditAnywhere)
		int mapResolution;
	
	UPROPERTY(EditAnywhere)
		int seed;

	UPROPERTY(EditAnywhere)
		FRandomStream randGen;

	UPROPERTY(EditAnywhere)
		int MAP_CHUNKS;

	UPROPERTY(Category = Map, EditAnywhere, BlueprintReadWrite)
		FHeightMapStruct HeightMap;

	UPROPERTY(Category = Map, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		TSubclassOf<AWorldBuilder_PMC> WORLD_BUILDER;
	
	UPROPERTY(Category = Map, EditAnywhere, BlueprintReadWrite)
		TMap<int32, AWorldBuilder_PMC*> WorldMap;

	UPROPERTY(Category = Materials, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		int32 materialIndex;

	UPROPERTY(Category = Materials, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UMaterial* DisplayMaterial;

	UPROPERTY(Category = Materials, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UMaterialInterface* DisplayMaterialInterface;
	
	UPROPERTY(Category = Materials, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UMaterialInstance* DisplayMaterialInstance;

	UPROPERTY(Category = Materials, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
		UMaterialInstanceDynamic* DisplayMaterialInstanceDynamic;

	virtual void Tick(float DeltaTime) override;

	//UFUNCTION()
	//	void OverlapBeginT(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	//UFUNCTION()
	//	void OverlapEndT(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	//UFUNCTION()
	//	void OverlapEndH(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	//UFUNCTION()
	//	void OverlapEndV(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Dig") // Server, Reliable, WithValidation
	void InitMap();

	UFUNCTION(BlueprintCallable, Category = "Sandbox|Dig") // Server, Reliable, WithValidation
	AWorldBuilder_PMC* LoadCell(int32 xPos, int32 yPos, APC* pRef);

	void DestroyCell(int32 xPos, int32 yPos, APC* pRef);

	TArray<AWorldBuilder_PMC*> spawnGrid(FVector collLoc, APC* pRef);

	void despawnGrid(FVector collLoc, APC* pRef, int32 useX, int32 useY);

	int32 szudzikPair(int32 x, int32 y);

	FVector2D szudzikPair(int32 sectionId);

	void setMaterial(UChunker_PMC* setMesh);

	//virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};