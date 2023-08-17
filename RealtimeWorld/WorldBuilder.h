// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RealtimeMeshActor.h"
#include "RealtimeMeshSimple.h"
#include "Components/BoxComponent.h"
#include "WorldBuilder.generated.h"

class AWorldGrid;
class APC;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class TERRSHALOM_API AWorldBuilder : public ARealtimeMeshActor
{
	GENERATED_BODY()

public:
	AWorldBuilder();

private:
	/** Whether we need to rebuild or not. */
	int32 bRebuild : 1;

	UFUNCTION()
	void OverlapBegin(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);
	UFUNCTION()
	void OverlapEnd(UPrimitiveComponent *OverlappedComp, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex);

protected:
	// UPROPERTY(ReplicatedUsing = OnRep_RealtimeMesh, VisibleAnywhere, BlueprintReadOnly, Meta = (ExposeFunctionCategories = "Mesh,Rendering,Physics,Components|RealtimeMesh", AllowPrivateAccess = "true"))

	/** RepNotify for changes made to current health.*/
	// UFUNCTION(Server, Reliable, WithValidation)
	//	void OnRep_RealtimeMesh();
	virtual void BeginPlay() override;

public:
	UPROPERTY()
	AWorldGrid *mapRef;

	// UPROPERTY(Category = Map, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	//	class URealtimeMeshComponent* RealtimeMesh;

	UPROPERTY(Category = Map, EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent *baseMesh;

	UPROPERTY(EditAnywhere)
	class UInstancedStaticMeshComponent *Walls;

	UPROPERTY(Category = Bounds, VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent *leftBds;

	UPROPERTY(Category = Bounds, VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent *topBds;

	UPROPERTY(Category = Bounds, VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent *rightBds;

	UPROPERTY(Category = Bounds, VisibleAnywhere, BlueprintReadOnly)
	UBoxComponent *botBds;

	UPROPERTY(EditAnywhere)
	int32 chunkSize;
	float chunkEdge;

	UPROPERTY()
	TSet<APC *> Players;

	// void OnConstruction(const FTransform& Transform) override;

	// void callInteract();

	// virtual void OnGenerateMesh_Implementation() override;

	virtual void Tick(float DeltaTime) override;

	void initWorldBuilder(AWorldGrid *map);

	/** Property replication */
	// void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	// From UChunk
public:
	mutable FCriticalSection PropertySyncRoot;
	FBoxSphereBounds LocalBounds;

	UPROPERTY()
	URealtimeMeshSimple *realMesh;

	// UPROPERTY()
	// FRealtimeMeshCollisionSettings CollisionSettings;

	UPROPERTY(VisibleAnywhere, BlueprintGetter = GetDisplayMaterial, BlueprintSetter = SetDisplayMaterial)
	UMaterialInterface *DisplayMaterial;

	UPROPERTY()
	int32 LODForMeshCollision;

	// UPROPERTY()
	// TMap<int32, FRealtimeMeshRenderableCollisionData> RenderableCollisionData;

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

	// TArray<FVector> Positions;
	// TArray<int> Triangles;
	int NoiseSamplesPerLine;
	int VerticesArraySize;
	FVector2D sectionOffset;
	float NoiseInputScale = 0.05 * 0.8; // Making this smaller will "stretch" the perlin noise terrain
	float NoiseOutputScale = 2000;		// Making this bigger will scale the terrain's height
	bool interactLock = false;

	FRealtimeMeshSimpleMeshData meshData;
	TArray<FRealtimeMeshSectionKey> LODKeys; // assuming 1 mesh
											 // Other things needed to generate the mesh
											 // TArray<FVector> Normals;
	// TArray<FVector> Tangents;
	// TArray<FVector2D> TexCoords;
	// TArray<FColor> Colors;

public:
	// virtual RealtimeMesh::FRealtimeMeshRef GetMesh() const
	//{
	//	// We should not ever bee here
	//	check(false);
	//	return RealtimeMesh::FRealtimeMeshRef(static_cast<RealtimeMesh::FRealtimeMesh*>(nullptr));
	// }

	void Initialize(FVector offset);

	void SetBounds(AWorldGrid *map, FVector spawnLoc);

	UFUNCTION(BlueprintCallable)
	UMaterialInterface *GetDisplayMaterial() const;

	UFUNCTION(BlueprintCallable)
	float GetTime() const;

	// UFUNCTION(Category = "RealtimeMesh|Providers|Collision", BlueprintCallable)
	// void SetCollisionSettings(const FRealtimeMeshCollisionSettings &NewCollisionSettings);

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
	// bool GetSectionMeshForLOD(int32 LODIndex, int32 SectionId, FRealtimeMeshRenderableMeshData &MeshData) override;
	// FRealtimeMeshSectionKey CreateMeshSection(const FRealtimeMeshLODKey& LODKey, const FRealtimeMeshSectionConfig& Config, const FChunkMeshData& MeshData,
	//	bool bShouldCreateCollision = false);
	// void createSection();
	// virtual FRealtimeMeshCollisionSettings GetCollisionSettings() override;
	// virtual bool IsThreadSafe() override;
	void CalculateBounds();

private:
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
	// void interact_Implementation(int32 faceIndex);
	// bool interact_Validate(int32 faceIndex);
};