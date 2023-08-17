// Fill out your copyright notice in the Description page of Project Settings.

#include "Chunker_PMC.h"
#include "Net/UnrealNetwork.h"
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

UMaterialInterface *UChunker_PMC::GetDisplayMaterial() const
{
	FScopeLock Lock(&PropertySyncRoot);
	return DisplayMaterial;
}

float UChunker_PMC::GetTime() const
{
	FScopeLock Lock(&PropertySyncRoot);
	return Time;
}

void UChunker_PMC::SetDisplayMaterial(UMaterialInterface *InMaterial)
{
	FScopeLock Lock(&PropertySyncRoot);
	DisplayMaterial = InMaterial;
}

void UChunker_PMC::SetTime(float InTime)
{
	{
		FScopeLock Lock(&PropertySyncRoot);
		Time = InTime;
	}
	// GetMesh()->MarkLODDirty(0);
}

void UChunker_PMC::GenerateMap()
{
	Normals.Init(FVector(0, 0, 1), VerticesArraySize);
	Tangents.Init(FProcMeshTangent(0, -1, 0), VerticesArraySize);
	UV0.SetNum(VerticesArraySize);
	VertexColors.Init(FColor::White, VerticesArraySize);

	GenerateVertices();
	GenerateTriangles();
	GenerateVertices();
}

void UChunker_PMC::initMeshData()
{
	FScopeLock Lock(&PropertySyncRoot);

	// BOUNDS
	GenerateMap();

	FVector nStream(0, 0, 1);
	FVector tStream(0, -1, 0);

	for (int setTan = 0; setTan < VerticesArraySize; setTan++)
	{
		FVector2D uvStream(UV0[setTan]);

		Tangents.Add(FProcMeshTangent(nStream, false));
		Tangents.Add(FProcMeshTangent(tStream, false));
		//UV0.Add(uvStream);
		//VertexColors.Add(FColor::White);
	}
}

void UChunker_PMC::CalculateBounds()
{
	LocalBounds = FBoxSphereBounds(FBox(FVector(MinX, MinY, 0.f) * mapRef->chunkSize, FVector(MaxX, MaxY, 1.0f) * mapRef->chunkSize));
}

float UChunker_PMC::CalculateHeightForPoint(float x, float y)
{
	return FMath::Sin(sqrt(x * x + y * y) + Time);
}

void UChunker_PMC::GenerateVertices()
{
	Vertices.Init(FVector(0, 0, 0), VerticesArraySize);
	for (uint32 y = 0; y < NoiseSamplesPerLine; y++)
	{
		for (uint32 x = 0; x < NoiseSamplesPerLine; x++)
		{
			// float NoiseResult = GetNoiseValueForGridCoordinates(x, y);
			uint32 index = GetIndexForGridCoordinates(x, y);
			FVector2D Position = GetPositionForGridCoordinates(x, y);
			double xCord = x * mapRef->mapResolution; // +sectionOffset.X * mapRef->chunkSize;
			double yCord = y * mapRef->mapResolution; // +sectionOffset.Y * mapRef->chunkSize;
			bool useHMap = true;
			if (mapRef->HeightMap.hMap.Num() == 0) {
				UE_LOG(LogStreaming, Log, TEXT("HEIGHT MAP IS INVALID"));
				useHMap = false;
			}
			float NoiseResult = useHMap ? mapRef->HeightMap.Get(x + sectionOffset.X, y + sectionOffset.Y) : 0;
			Vertices[index] = FVector(xCord, yCord, NoiseResult) + parentOffset; // NoiseResult);
			UV0[index] = FVector2D(x, y);
		}
	}
}

void UChunker_PMC::GenerateTriangles()
{
	uint32 QuadSize = 6;									// This is the number of triangle indexes making up a quad (square section of the grid)
	uint32 NumberOfQuadsPerLine = NoiseSamplesPerLine - 1; // We have one less quad per line than the amount of vertices, since each vertex is the start of a quad except the last ones
	// In our triangles array, we need 6 values per quad
	uint32 TrianglesArraySize = NumberOfQuadsPerLine * NumberOfQuadsPerLine * QuadSize;
	Triangles.Init(0, TrianglesArraySize);

	for (uint32 y = 0; y < NumberOfQuadsPerLine; y++)
	{
		for (uint32 x = 0; x < NumberOfQuadsPerLine; x++)
		{
			int QuadIndex = x + y * NumberOfQuadsPerLine;
			int TriangleIndex = QuadIndex * QuadSize;

			// Getting the indexes of the four vertices making up this quad
			uint32 bottomLeftIndex = GetIndexForGridCoordinates(x, y);
			uint32 topLeftIndex = GetIndexForGridCoordinates(x, y + 1);
			uint32 topRightIndex = GetIndexForGridCoordinates(x + 1, y + 1);
			uint32 bottomRightIndex = GetIndexForGridCoordinates(x + 1, y);

			// Assigning the 6 triangle points to the corresponding vertex indexes, by going counter-clockwise.
			Triangles[TriangleIndex] = bottomLeftIndex;
			Triangles[TriangleIndex + 1] = topLeftIndex;
			Triangles[TriangleIndex + 2] = topRightIndex;
			Triangles[TriangleIndex + 3] = bottomLeftIndex;
			Triangles[TriangleIndex + 4] = topRightIndex;
			Triangles[TriangleIndex + 5] = bottomRightIndex;
		}
	}
}

// Returns the scaled noise value for grid coordinates [x,y]
float UChunker_PMC::GetNoiseValueForGridCoordinates(int x, int y)
{
	return FMath::PerlinNoise3D(FVector(
			   (x * NoiseInputScale) + 0.1,
			   (y * NoiseInputScale) + 0.1,
			   1.0)) *
		   NoiseOutputScale;
}

uint32 UChunker_PMC::GetIndexForGridCoordinates(uint32 x, uint32 y)
{
	return x + y * NoiseSamplesPerLine;
}

FVector2D UChunker_PMC::GetPositionForGridCoordinates(int x, int y)
{
	return FVector2D(
		x * mapRef->mapResolution + sectionOffset.X * mapRef->chunkSize,
		y * mapRef->mapResolution + sectionOffset.Y * mapRef->chunkSize);
}

float UChunker_PMC::PerlinNoise(const FVector &Location)
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

void UChunker_PMC::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// TODO: maybe can delete?
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UChunker_PMC, Vertices)
	DOREPLIFETIME(UChunker_PMC, Triangles)
	DOREPLIFETIME(UChunker_PMC, Normals)
	DOREPLIFETIME(UChunker_PMC, VertexColors)
	DOREPLIFETIME(UChunker_PMC, Tangents)
	DOREPLIFETIME(UChunker_PMC, UV0)
		//, Triangles, Normals, VertexColors, Tangents, UV0);
}