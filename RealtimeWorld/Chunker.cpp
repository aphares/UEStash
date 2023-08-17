// Fill out your copyright notice in the Description page of Project Settings.

#include "Chunker.h"
#include "RealtimeMesh.h"
#include "RealtimeMeshLibrary.h"
#include "WorldGrid.h"

// Implementation of 1D, 2D and 3D Perlin noise based on Ken Perlin's improved version https://mrl.nyu.edu/~perlin/noise/
// (See Random3.tps for additional third party software info.)
namespace FMathPerlinHelpers
{
	const int16 MAP_CHUNKS = 255;
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
		155, 122, 210, 168, 254, 162, 129, 33, 18, 209, 61, 191, 199, 157, 245, 55, 164, 167, 215, 246, 144, 107, 235};

	// Gradient functions for 1D, 2D and 3D Perlin noise

	FORCEINLINE float Grad1(int32 Hash, float X)
	{
		// Slicing Perlin's 3D improved noise would give us only scales of -1, 0 and 1; this looks pretty bad so let's use a different sampling
		static const float Grad1Scales[16] = {-8 / 8, -7 / 8., -6 / 8., -5 / 8., -4 / 8., -3 / 8., -2 / 8., -1 / 8., 1 / 8., 2 / 8., 3 / 8., 4 / 8., 5 / 8., 6 / 8., 7 / 8., 8 / 8};
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

UChunker::UChunker()
{
}

void ConvertQuadToTriangles(TArray<int32>& Triangles, int32 Vert0, int32 Vert1, int32 Vert2, int32 Vert3)
{
	Triangles.Add(Vert0);
	Triangles.Add(Vert1);
	Triangles.Add(Vert3);

	Triangles.Add(Vert1);
	Triangles.Add(Vert2);
	Triangles.Add(Vert3);
}

void UChunker::Initialize(FVector offset)
{
	int MaxLOD = 2;
	//TArray<FRealtimeMeshLODProperties> NewLODs;
	//for (int32 LODIndex = 0; LODIndex <= MaxLOD; LODIndex++)
	//{
	//	FRealtimeMeshLODProperties LODProperties;
	//	LODProperties.ScreenSize = 0.01;
	//	NewLODs.Add(LODProperties);
	//}
	//ConfigureLODs(NewLODs);

	MinX = MinY = MAP_CHUNKS;
	MaxX = MaxY = -MAP_CHUNKS;
	MinValue = -1;
	MaxValue = 1;
	PointsSX = 128;
	PointsSY = PointsSX;
	SizeX = mapRef->chunkSize;
	SizeY = mapRef->chunkSize;
	SizeZ = mapRef->chunkSize / 5;
	parentOffset = offset;

	//LODKeys.Reserve(MaxLOD);
	initMeshData();

	for (int LODIndex = 0; LODIndex <= MaxLOD; LODIndex++)
	{
		FRealtimeMeshSectionConfig Properties = FRealtimeMeshSectionConfig(ERealtimeMeshSectionDrawType::Dynamic, 0);
		Properties.bCastsShadow = true;
		Properties.bIsVisible = true;
		LODKeys.Add(CreateMeshSection(FRealtimeMeshLODKey(LODIndex), Properties, meshData, true));
	}
	CalculateBounds();
}

void UChunker::SetBounds(AWorldGrid *map, FVector spawnLoc)
{
	mapRef = map;
	NoiseSamplesPerLine = mapRef->chunkSize / mapRef->mapResolution + 1;
	VerticesArraySize = NoiseSamplesPerLine * NoiseSamplesPerLine;
	sectionOffset = FVector2D(spawnLoc.X, spawnLoc.Y);
}

UMaterialInterface *UChunker::GetDisplayMaterial() const
{
	FScopeLock Lock(&PropertySyncRoot);
	return DisplayMaterial;
}

//void UChunker::SetCollisionSettings(const FRealtimeMeshCollisionSettings &NewCollisionSettings)
//{
//	{
//		FScopeLock Lock(&SyncRoot);
//		CollisionSettings = NewCollisionSettings;
//	}
//	MarkCollisionDirty();
//}

float UChunker::GetTime() const
{
	FScopeLock Lock(&PropertySyncRoot);
	return Time;
}

void UChunker::SetDisplayMaterial(UMaterialInterface *InMaterial)
{
	FScopeLock Lock(&PropertySyncRoot);
	DisplayMaterial = InMaterial;
}

void UChunker::SetRenderableLODForCollision(int32 LODIndex)
{
	bool bMarkCollisionDirty = false;
	{
		//FScopeLock Lock(&SyncRoot);
		//if (LODForMeshCollision != LODIndex)
		//{
		//	LODForMeshCollision = LODIndex;
		//	RenderableCollisionData.Empty();

		//	bMarkCollisionDirty = true;
		//}
	}

	if (bMarkCollisionDirty)
	{
		GetMesh()->MarkCollisionDirty();
	}
}

void UChunker::SetRenderableSectionAffectsCollision(int32 chunkX, int32 chunkY, bool bCollisionEnabled, URealtimeMeshComponent *RealtimeMesh)
{
	if (FMath::Abs(chunkX) >= MAP_CHUNKS || FMath::Abs(chunkY) >= MAP_CHUNKS)
		return;
	bool bShouldMarkCollisionDirty = false;
	FScopeLock Lock(&SyncRoot);
	int32 SectionId = szudzikPair(chunkX, chunkY);
	int32 *isLoaded = PlayerMap.Find(SectionId);
	if (bCollisionEnabled)
	{
		if (isLoaded)
		{
			PlayerMap.Emplace(SectionId, *isLoaded + 1);
		}
		else
		{
			PlayerMap.Emplace(SectionId, 1);
			// CollisionProvider->SetRenderableSectionAffectsCollision(SectionId, true);
			//..RenderableCollisionData.Add(SectionId);
			bShouldMarkCollisionDirty = true;
		}
	}
	else if (isLoaded)
	{
		if (*isLoaded > 0)
		{
			PlayerMap.Emplace(SectionId, *isLoaded - 1);
		}
		else if (chunkX != 0 || chunkY != 0)
		{ // tmp keep persitent start
			PlayerMap.Remove(SectionId);
			// CollisionProvider->SetRenderableSectionAffectsCollision(SectionId, false);
			// RenderableCollisionData.Remove(SectionId);
			bShouldMarkCollisionDirty = true;
		}
	}

	if (bShouldMarkCollisionDirty)
	{
		GetMesh()->MarkCollisionDirty();
		//RealtimeMesh->Initialize(CollisionProvider);
	}
}

void UChunker::SetTime(float InTime)
{
	{
		FScopeLock Lock(&PropertySyncRoot);
		Time = InTime;
	}
	// GetMesh()->MarkLODDirty(0);
}

void drawSection(int32 index)
{
}

void removeSection(int32 index)
{
}

//FBoxSphereBounds UChunker::GetBounds()
//{
//	return LocalBounds;
//}

void UChunker::GenerateMap()
{
	meshData.Normals.Init(FVector(0, 0, 1), VerticesArraySize);
	meshData.Tangents.Init(FVector(0, -1, 0), VerticesArraySize);
	meshData.UV0.SetNum(VerticesArraySize);
	meshData.Colors.Init(FColor::White, VerticesArraySize);

	GenerateVertices();
	GenerateTriangles();
	GenerateVertices();
}

void UChunker::initMeshData()
{
	FScopeLock Lock(&PropertySyncRoot);

	//check(LODIndex <= 2);
	// float dx = (MaxX - MinX) / (float)PointsSX; //change of x between two points
	// float dy = (MaxY - MinY) / (float)PointsSY; //change of y between two points
	// for (int32 yindex = 0; yindex < PointsSY; yindex++)
	//{
	//	float yalpha = (float)yindex / (float)(PointsSY - 1);
	//	float ypos = FMath::Lerp(-SizeY / 2, SizeY / 2, yalpha);
	//	float yvalue = FMath::Lerp(MinY, MaxY, yalpha);
	//	for (int32 xindex = 0; xindex < PointsSX; xindex++)
	//	{
	//		float xalpha = (float)xindex / (float)(PointsSX - 1);
	//		float xpos = FMath::Lerp(-SizeX / 2, SizeX / 2, xalpha);
	//		float xvalue = FMath::Lerp(MinX, MaxX, xalpha);

	//		FVector Position(xpos, ypos,
	//			FMath::GetMappedRangeValueClamped(
	//				FVector2D(MinValue, MaxValue),
	//				FVector2D(-SizeZ / 2, SizeZ / 2),
	//				CalculateHeightForPoint(xvalue, yvalue)
	//			)
	//		);

	//		float dfdx = (CalculateHeightForPoint(xvalue + dx, yvalue) - CalculateHeightForPoint(xvalue - dx, yvalue)) / (2 * dx); //derivative of f over x
	//		float dfdy = (CalculateHeightForPoint(xvalue, yvalue + dy) - CalculateHeightForPoint(xvalue, yvalue - dy)) / (2 * dy); //derivative of f over y

	//		FVector Normal(-dfdx, -dfdy, 1);
	//		Normal.Normalize();
	//		FVector Tangent(1, 0, dfdx);
	//		Tangent.Normalize();

	//		FVector2D TexCoords(xalpha, yalpha);

	//		MeshData.Positions.Add(Position);
	//		MeshData.Tangents.Add(Normal, Tangent);
	//		MeshData.TexCoords.Add(TexCoords);
	//		MeshData.Colors.Add(FColor::White);

	//		if (xindex != PointsSX - 1 && yindex != PointsSY - 1)
	//		{
	//			int32 AIndex = xindex + yindex * PointsSX;
	//			int32 BIndex = AIndex + 1;
	//			int32 CIndex = AIndex + PointsSX;
	//			int32 DIndex = CIndex + 1;
	//			MeshData.Triangles.AddTriangle(AIndex, CIndex, BIndex);
	//			MeshData.Triangles.AddTriangle(BIndex, CIndex, DIndex);
	//		}
	//	}
	//}
	// sectionOffset = szudzikPair(SectionId);

	// if (MinX > sectionOffset.X) {
	//	MinX = sectionOffset.X;
	// }

	// if (MaxX <= sectionOffset.X) {
	//	MaxX = sectionOffset.X + 1;
	// }

	// if (MinY > sectionOffset.Y) {
	//	MinY = sectionOffset.Y;
	// }

	// if (MaxY <= sectionOffset.Y) {
	//	MaxY = sectionOffset.Y + 1;
	// }

	// BOUNDS
	GenerateMap();

	FVector nStream(0, 0, 1);
	FVector tStream(0, -1, 0);

	for (int setTan = 0; setTan < VerticesArraySize; setTan++)
	{
		FVector2D uvStream(meshData.UV0[setTan]);

		meshData.Tangents.Add(nStream);
		meshData.Tangents.Add(tStream);
		meshData.UV0.Add(uvStream);
		meshData.Colors.Add(FColor::White);
	}
	//MeshData.Triangles.Append(Triangles);
	//return true;
}

//FRealtimeMeshCollisionSettings UChunker::GetCollisionSettings()
//{
//	FRealtimeMeshCollisionSettings Settings;
//	Settings.bUseAsyncCooking = false;
//	Settings.bUseComplexAsSimple = true;
//
//	return Settings;
//}
//
//bool UChunker::IsThreadSafe()
//{
//	return true;
//}

void UChunker::CalculateBounds()
{
	LocalBounds = FBoxSphereBounds(FBox(FVector(MinX, MinY, 0.f) * mapRef->chunkSize, FVector(MaxX, MaxY, 1.0f) * mapRef->chunkSize));
}

float UChunker::CalculateHeightForPoint(float x, float y)
{
	return FMath::Sin(sqrt(x * x + y * y) + Time);
}

void UChunker::GenerateVertices()
{
	meshData.Positions.Init(FVector(0, 0, 0), VerticesArraySize);
	for (int y = 0; y < NoiseSamplesPerLine; y++)
	{
		for (int x = 0; x < NoiseSamplesPerLine; x++)
		{
			// float NoiseResult = GetNoiseValueForGridCoordinates(x, y);
			int index = GetIndexForGridCoordinates(x, y);
			FVector2D Position = GetPositionForGridCoordinates(x, y);
			double xCord = x * mapRef->mapResolution; // +sectionOffset.X * mapRef->chunkSize;
			double yCord = y * mapRef->mapResolution; // +sectionOffset.Y * mapRef->chunkSize;
			float NoiseResult = mapRef->HeightMap.Get(x + sectionOffset.X, y + sectionOffset.Y);
			meshData.Positions[index] = FVector(xCord, yCord, NoiseResult) + parentOffset; // NoiseResult);
			meshData.UV0[index] = FVector2D(x, y);
		}
	}
}

void UChunker::GenerateTriangles()
{
	int QuadSize = 6;									// This is the number of triangle indexes making up a quad (square section of the grid)
	int NumberOfQuadsPerLine = NoiseSamplesPerLine - 1; // We have one less quad per line than the amount of vertices, since each vertex is the start of a quad except the last ones
	// In our triangles array, we need 6 values per quad
	int TrianglesArraySize = NumberOfQuadsPerLine * NumberOfQuadsPerLine * QuadSize;
	meshData.Triangles.Init(0, TrianglesArraySize);

	for (int y = 0; y < NumberOfQuadsPerLine; y++)
	{
		for (int x = 0; x < NumberOfQuadsPerLine; x++)
		{
			int QuadIndex = x + y * NumberOfQuadsPerLine;
			int TriangleIndex = QuadIndex * QuadSize;

			// Getting the indexes of the four vertices making up this quad
			int bottomLeftIndex = GetIndexForGridCoordinates(x, y);
			int topLeftIndex = GetIndexForGridCoordinates(x, y + 1);
			int topRightIndex = GetIndexForGridCoordinates(x + 1, y + 1);
			int bottomRightIndex = GetIndexForGridCoordinates(x + 1, y);

			// Assigning the 6 triangle points to the corresponding vertex indexes, by going counter-clockwise.
			meshData.Triangles[TriangleIndex] = bottomLeftIndex;
			meshData.Triangles[TriangleIndex + 1] = topLeftIndex;
			meshData.Triangles[TriangleIndex + 2] = topRightIndex;
			meshData.Triangles[TriangleIndex + 3] = bottomLeftIndex;
			meshData.Triangles[TriangleIndex + 4] = topRightIndex;
			meshData.Triangles[TriangleIndex + 5] = bottomRightIndex;
		}
	}
}

// Returns the scaled noise value for grid coordinates [x,y]
float UChunker::GetNoiseValueForGridCoordinates(int x, int y)
{
	return FMath::PerlinNoise3D(FVector(
			   (x * NoiseInputScale) + 0.1,
			   (y * NoiseInputScale) + 0.1,
			   1.0)) *
		   NoiseOutputScale;
}

int UChunker::GetIndexForGridCoordinates(int x, int y)
{
	return x + y * NoiseSamplesPerLine;
}

FVector2D UChunker::GetPositionForGridCoordinates(int x, int y)
{
	return FVector2D(
		x * mapRef->mapResolution + sectionOffset.X * mapRef->chunkSize,
		y * mapRef->mapResolution + sectionOffset.Y * mapRef->chunkSize);
}

float UChunker::PerlinNoise(const FVector &Location)
{
	float Xfl = FMath::FloorToFloat((float)Location.X); // LWC_TODO: Precision loss
	float Yfl = FMath::FloorToFloat((float)Location.Y);
	float Zfl = FMath::FloorToFloat((float)Location.Z);
	int32 Xi = (int32)(Xfl)&255;
	int32 Yi = (int32)(Yfl)&255;
	int32 Zi = (int32)(Zfl)&255;
	float X = (float)Location.X - Xfl;
	float Y = (float)Location.Y - Yfl;
	float Z = (float)Location.Z - Zfl;
	float Xm1 = X - 1.0f;
	float Ym1 = Y - 1.0f;
	float Zm1 = Z - 1.0f;

	int32 P[512];

	for (int i = 0; i < 512; i++)
		P[i] = mapRef->randGen.RandRange(0, 256);

	int32 A = P[Xi] + Yi;
	int32 AA = P[A] + Zi;
	int32 AB = P[A + 1] + Zi;

	int32 B = P[Xi + 1] + Yi;
	int32 BA = P[B] + Zi;
	int32 BB = P[B + 1] + Zi;

	float U = SmoothCurve(X);
	float V = SmoothCurve(Y);
	float W = SmoothCurve(Z);

	// Note: range is already approximately -1,1 because of the specific choice of direction vectors for the Grad3 function
	// This analysis (http://digitalfreepen.com/2017/06/20/range-perlin-noise.html) suggests scaling by 1/sqrt(3/4) * 1/maxGradientVectorLen, but the choice of gradient vectors makes this overly conservative
	// Scale factor of .97 is (1.0/the max values of a billion random samples); to be 100% sure about the range I also just Clamp it for now.
	return FMath::Clamp(0.97f *
							FMath::Lerp(FMath::Lerp(FMath::Lerp(Grad3(P[AA], X, Y, Z), Grad3(P[BA], Xm1, Y, Z), U),
													FMath::Lerp(Grad3(P[AB], X, Ym1, Z), Grad3(P[BB], Xm1, Ym1, Z), U),
													V),
										FMath::Lerp(FMath::Lerp(Grad3(P[AA + 1], X, Y, Zm1), Grad3(P[BA + 1], Xm1, Y, Zm1), U),
													FMath::Lerp(Grad3(P[AB + 1], X, Ym1, Zm1), Grad3(P[BB + 1], Xm1, Ym1, Zm1), U),
													V),
										W),
						-1.0f, 1.0f);
};

FChunkMeshData& AppendBoxMesh(FVector BoxRadius, FTransform BoxTransform, FChunkMeshData& MeshData)
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
	const int32 StartVertex = MeshData.Positions.Num();
	const int32 NumVerts = 24; // 6 faces x 4 verts per face
	const int32 NumIndices = 36;

	// Make sure the secondary arrays are the same length, zeroing them if necessary
	MeshData.Normals.SetNumZeroed(StartVertex);
	MeshData.Tangents.SetNumZeroed(StartVertex);
	MeshData.UV0.SetNumZeroed(StartVertex);

	MeshData.Positions.Reserve(StartVertex + NumVerts);
	MeshData.Normals.Reserve(StartVertex + NumVerts);
	MeshData.Tangents.Reserve(StartVertex + NumVerts);
	MeshData.UV0.Reserve(StartVertex + NumVerts);
	MeshData.Triangles.Reserve(MeshData.Triangles.Num() + NumIndices);

	const auto WriteToNextFour = [](TArray<FVector>& Array, const FVector& Value)
	{
		Array.Add(Value);
		Array.Add(Value);
		Array.Add(Value);
		Array.Add(Value);
	};

	const auto WriteQuadPositions = [&MeshData](const FVector& VertA, const FVector& VertB, const FVector& VertC, const FVector& VertD)
	{
		MeshData.Positions.Add(VertA);
		MeshData.Positions.Add(VertB);
		MeshData.Positions.Add(VertC);
		MeshData.Positions.Add(VertD);
	};

	WriteQuadPositions(BoxVerts[0], BoxVerts[1], BoxVerts[2], BoxVerts[3]);
	WriteToNextFour(MeshData.Normals, BoxTransform.TransformVectorNoScale(FVector(0.0f, 0.0f, 1.0f)));
	WriteToNextFour(MeshData.Tangents, BoxTransform.TransformVectorNoScale(FVector(0.0f, -1.0f, 0.0f)));
	ConvertQuadToTriangles(MeshData.Triangles, StartVertex + 0, StartVertex + 1, StartVertex + 2, StartVertex + 3);

	WriteQuadPositions(BoxVerts[4], BoxVerts[0], BoxVerts[3], BoxVerts[7]);
	WriteToNextFour(MeshData.Normals, BoxTransform.TransformVectorNoScale(FVector(-1.0, 0.0, 0.0)));
	WriteToNextFour(MeshData.Tangents, BoxTransform.TransformVectorNoScale(FVector(0.0f, -1.0f, 0.0f)));
	ConvertQuadToTriangles(MeshData.Triangles, StartVertex + 4, StartVertex + 5, StartVertex + 6, StartVertex + 7);

	WriteQuadPositions(BoxVerts[5], BoxVerts[1], BoxVerts[0], BoxVerts[4]);
	WriteToNextFour(MeshData.Normals, BoxTransform.TransformVectorNoScale(FVector(0.0, 1.0, 0.0)));
	WriteToNextFour(MeshData.Tangents, BoxTransform.TransformVectorNoScale(FVector(-1.0f, 0.0f, 0.0f)));
	ConvertQuadToTriangles(MeshData.Triangles, StartVertex + 8, StartVertex + 9, StartVertex + 10, StartVertex + 11);

	WriteQuadPositions(BoxVerts[6], BoxVerts[2], BoxVerts[1], BoxVerts[5]);
	WriteToNextFour(MeshData.Normals, BoxTransform.TransformVectorNoScale(FVector(1.0, 0.0, 0.0)));
	WriteToNextFour(MeshData.Tangents, BoxTransform.TransformVectorNoScale(FVector(0.0f, 1.0f, 0.0f)));
	ConvertQuadToTriangles(MeshData.Triangles, StartVertex + 12, StartVertex + 13, StartVertex + 14, StartVertex + 15);

	WriteQuadPositions(BoxVerts[7], BoxVerts[3], BoxVerts[2], BoxVerts[6]);
	WriteToNextFour(MeshData.Normals, BoxTransform.TransformVectorNoScale(FVector(0.0, -1.0, 0.0)));
	WriteToNextFour(MeshData.Tangents, BoxTransform.TransformVectorNoScale(FVector(1.0f, 0.0f, 0.0f)));
	ConvertQuadToTriangles(MeshData.Triangles, StartVertex + 16, StartVertex + 17, StartVertex + 18, StartVertex + 19);

	WriteQuadPositions(BoxVerts[7], BoxVerts[6], BoxVerts[5], BoxVerts[4]);
	WriteToNextFour(MeshData.Normals, BoxTransform.TransformVectorNoScale(FVector(0.0, 0.0, -1.0)));
	WriteToNextFour(MeshData.Tangents, BoxTransform.TransformVectorNoScale(FVector(0.0f, 1.0f, 0.0f)));
	ConvertQuadToTriangles(MeshData.Triangles, StartVertex + 20, StartVertex + 21, StartVertex + 22, StartVertex + 23);

	// UVs
	for (int32 Index = 0; Index < 6; Index++)
	{
		MeshData.UV0.Add(FVector2D(0.0f, 0.0f));
		MeshData.UV0.Add(FVector2D(0.0f, 1.0f));
		MeshData.UV0.Add(FVector2D(1.0f, 1.0f));
		MeshData.UV0.Add(FVector2D(1.0f, 0.0f));
	}

	return MeshData;
}


/**************[[ SANDBOX ]]*****************/
void UChunker::Interact_Implementation(int32 faceIndex, float strength, bool isAdd)
{
	if (!interactLock) {
		interactLock = true;
		const int32 thisTri = (int32) faceIndex * 3;

		const int32 addOrSubtract = isAdd ? 1 : -1;
		const float baseChange = strength * addOrSubtract;

		meshData.Positions[meshData.Triangles[thisTri]].Z += baseChange + FMath::RandRange(-strength, strength);
		meshData.Positions[meshData.Triangles[thisTri] + 1].Z += baseChange + FMath::RandRange(-strength, strength);
		meshData.Positions[meshData.Triangles[thisTri] + 2].Z += baseChange + FMath::RandRange(-strength, strength);

		int MaxLOD = 2;
		for (int LODIndex = 0; LODIndex <= MaxLOD; LODIndex++)
		{
			UpdateSectionMesh(LODKeys[LODIndex], meshData);
		}
		OnGenerateMesh(this);
		interactLock = false;
	}
}

bool UChunker::Interact_Validate(int32 faceIndex, float strength, bool isAdd)
{
	return true;
}