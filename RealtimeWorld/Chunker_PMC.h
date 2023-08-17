// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "RealtimeMeshComponent.h"
#include "RealtimeMeshSimple.h"
#include "Chunker_PMC.generated.h"

class AWorldGrid;

UCLASS(Blueprintable, ClassGroup = Rendering, HideCategories = (Object, Activation, Cooking))
class TERRSHALOM_API UChunker_PMC : public URealtimeMeshSimple //UProceduralMeshComponent
{
	GENERATED_BODY()
public:
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

	FVector2D sectionOffset;

	uint32 NoiseSamplesPerLine;
	int VerticesArraySize;
	float NoiseInputScale = 0.05 * 0.8; // Making this smaller will "stretch" the perlin noise terrain
	float NoiseOutputScale = 2000;		// Making this bigger will scale the terrain's height
	bool interactLock = false;


	UPROPERTY(Replicated)
		TArray<FVector> Vertices;

	UPROPERTY(Replicated)
		TArray<int32> Triangles;

	UPROPERTY(Replicated)
		TArray<FVector> Normals;

	UPROPERTY(Replicated)
		TArray<FColor> VertexColors;

	UPROPERTY(Replicated)
		TArray<FProcMeshTangent> Tangents;

	UPROPERTY(Replicated)
		TArray<FVector2D> UV0;

	//FChunkMeshData meshData;
	//TArray<FRealtimeMeshSectionKey> LODKeys; // assuming 1 mesh 
	// Other things needed to generate the mesh
	//TArray<FVector> Normals;
	//TArray<FVector> Tangents;
	//TArray<FVector2D> TexCoords;
	//TArray<FColor> Colors;
	// 
	//virtual RealtimeMesh::FRealtimeMeshRef GetMesh() const
	//{
	//	// We should not ever bee here
	//	check(false);
	//	return RealtimeMesh::FRealtimeMeshRef(static_cast<RealtimeMesh::FRealtimeMesh*>(nullptr));
	//}

	UFUNCTION(BlueprintCallable)
	UMaterialInterface *GetDisplayMaterial() const;

	UFUNCTION(BlueprintCallable)
	float GetTime() const;

	//UFUNCTION(Category = "RealtimeMesh|Providers|Collision", BlueprintCallable)
	//void SetCollisionSettings(const FRealtimeMeshCollisionSettings &NewCollisionSettings);

	UFUNCTION(BlueprintCallable)
	void SetDisplayMaterial(UMaterialInterface *InMaterial);

	UFUNCTION(BlueprintCallable)
	void SetTime(float InTime);

	UFUNCTION(BlueprintCallable)
	void GenerateMap();

	void initMeshData();

	void CalculateBounds();

	// FBoxSphereBounds GetBounds() override;
	//bool GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRealtimeMeshRenderableMeshData &MeshData) override;
	//FRealtimeMeshSectionKey CreateMeshSection(const FRealtimeMeshLODKey& LODKey, const FRealtimeMeshSectionConfig& Config, const FChunkMeshData& MeshData,
	//	bool bShouldCreateCollision = false);
	//void createSection();
	//virtual FRealtimeMeshCollisionSettings GetCollisionSettings() override;
	//virtual bool IsThreadSafe() override;

private:
	float CalculateHeightForPoint(float x, float y);
	void GenerateVertices();
	void GenerateTriangles();

	float GetNoiseValueForGridCoordinates(int x, int y);
	uint32 GetIndexForGridCoordinates(uint32 x, uint32 y);
	FVector2D GetPositionForGridCoordinates(int x, int y);
	float PerlinNoise(const FVector& Location);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};