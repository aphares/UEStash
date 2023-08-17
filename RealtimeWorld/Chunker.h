// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Chunk.h"
#include "Chunker.generated.h"

class AWorldGrid;

UCLASS(Blueprintable, Abstract, ClassGroup = Rendering, HideCategories = (Object, Activation, Cooking))
class TERRSHALOM_API UChunker : public UChunk
{
	GENERATED_BODY()
private:
	mutable FCriticalSection PropertySyncRoot;
	FBoxSphereBounds LocalBounds;

	UPROPERTY()
	AWorldGrid *mapRef;

	//UPROPERTY()
	//FRealtimeMeshCollisionSettings CollisionSettings;

	UPROPERTY(VisibleAnywhere, BlueprintGetter = GetDisplayMaterial, BlueprintSetter = SetDisplayMaterial)
	UMaterialInterface *DisplayMaterial;

	UPROPERTY()
	int32 LODForMeshCollision;

	//UPROPERTY()
	//TMap<int32, FRealtimeMeshRenderableCollisionData> RenderableCollisionData;

	UPROPERTY()
	TMap<int32, int32> PlayerMap;

	UPROPERTY(VisibleAnywhere, BlueprintGetter = GetTime, BlueprintSetter = SetTime)
	float Time;

	FCriticalSection SyncRoot;

	float MinValue, MaxValue;
	int MinX, MaxX, MinY, MaxY;
	int32 PointsSX, PointsSY, screenX, screenY;
	float SizeX, SizeY, SizeZ;
	FVector parentOffset;

	//TArray<FVector> Positions;
	//TArray<int> Triangles;
	int NoiseSamplesPerLine;
	int VerticesArraySize;
	FVector2D sectionOffset;
	float NoiseInputScale = 0.05 * 0.8; // Making this smaller will "stretch" the perlin noise terrain
	float NoiseOutputScale = 2000;		// Making this bigger will scale the terrain's height
	bool interactLock = false;


	FChunkMeshData meshData;
	TArray<FRealtimeMeshSectionKey> LODKeys; // assuming 1 mesh 
	// Other things needed to generate the mesh
	//TArray<FVector> Normals;
	//TArray<FVector> Tangents;
	//TArray<FVector2D> TexCoords;
	//TArray<FColor> Colors;

public:
	UChunker();

	//virtual RealtimeMesh::FRealtimeMeshRef GetMesh() const
	//{
	//	// We should not ever bee here
	//	check(false);
	//	return RealtimeMesh::FRealtimeMeshRef(static_cast<RealtimeMesh::FRealtimeMesh*>(nullptr));
	//}

	void Initialize(FVector offset);

	void SetBounds(AWorldGrid *map, FVector spawnLoc);

	UFUNCTION(BlueprintCallable)
	UMaterialInterface *GetDisplayMaterial() const;

	UFUNCTION(BlueprintCallable)
	float GetTime() const;

	//UFUNCTION(Category = "RealtimeMesh|Providers|Collision", BlueprintCallable)
	//void SetCollisionSettings(const FRealtimeMeshCollisionSettings &NewCollisionSettings);

	UFUNCTION(BlueprintCallable)
	void SetDisplayMaterial(UMaterialInterface *InMaterial);

	UFUNCTION(Category = "RealtimeMesh|Providers|Collision", BlueprintCallable)
	void SetRenderableLODForCollision(int32 LODIndex);

	UFUNCTION(Category = "RealtimeMesh|Providers|Collision", BlueprintCallable)
	void SetRenderableSectionAffectsCollision(int32 chunkX, int32 chunkY, bool bCollisionEnabled, URealtimeMeshComponent *RealtimeMesh); //  URealtimeMeshProviderCollision *CollisionProvider,

	UFUNCTION(BlueprintCallable)
	void SetTime(float InTime);

	void drawSection(int32 index);

	void removeSection(int32 index);

	UFUNCTION(BlueprintCallable)
	void GenerateMap();

	void initMeshData();

public:
	// FBoxSphereBounds GetBounds() override;
	//bool GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRealtimeMeshRenderableMeshData &MeshData) override;
	//FRealtimeMeshSectionKey CreateMeshSection(const FRealtimeMeshLODKey& LODKey, const FRealtimeMeshSectionConfig& Config, const FChunkMeshData& MeshData,
	//	bool bShouldCreateCollision = false);
	//void createSection();
	//virtual FRealtimeMeshCollisionSettings GetCollisionSettings() override;
	//virtual bool IsThreadSafe() override;

private:
	void CalculateBounds();
	float CalculateHeightForPoint(float x, float y);
	void GenerateVertices();
	void GenerateTriangles();

	float GetNoiseValueForGridCoordinates(int x, int y);
	int GetIndexForGridCoordinates(int x, int y);
	FVector2D GetPositionForGridCoordinates(int x, int y);
	float PerlinNoise(const FVector &Location);
public: // sandbox

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Sandbox|Dig")
		void Interact(int32 faceIndex, float strength, bool isAdd);
		//void interact_Implementation(int32 faceIndex);
		//bool interact_Validate(int32 faceIndex);
};