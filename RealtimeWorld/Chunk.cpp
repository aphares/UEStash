// Copyright TriAxis Games, L.L.C. All Rights Reserved.

#include "Chunk.h"
#include "RealtimeMeshCore.h"
#include "Data/RealtimeMeshDataBuilder.h"
#include "RenderProxy/RealtimeMeshSectionGroupProxy.h"
#include "RenderProxy/RealtimeMeshVertexFactory.h"

#define LOCTEXT_NAMESPACE "Chunk"


namespace RealtimeMesh
{
	namespace RealtimeMeshSimpleInternal
	{
		template<typename TangentType, typename TexCoordType, typename IndexType>
		TSharedRef<RealtimeMesh::FRealtimeMeshVertexDataBuilder> BuildMeshData(FName ComponentName,
			const FChunkMeshData& MeshData, bool bRemoveDegenerates)
		{
			const auto Builder = MakeShared<RealtimeMesh::FRealtimeMeshVertexDataBuilder>();

			// Build position buffer
			const auto PositionBuffer = Builder->CreateVertexStream<FVector3f>(RealtimeMesh::FRealtimeMeshLocalVertexFactory::PositionStreamName);
			PositionBuffer->Append<FVector3f>(MeshData.Positions.Num(), [&MeshData](int32 Index) -> FVector3f { return FVector3f(MeshData.Positions[Index]); });

			// Build tangents buffer
			{
				const int32 TangentsToCopy = FMath::Min(MeshData.Positions.Num(), FMath::Max(MeshData.Tangents.Num(), MeshData.Normals.Num()));
				using TangentsBufferType = RealtimeMesh::FRealtimeMeshTangents<TangentType>;
				auto TangentsBuffer = Builder->CreateVertexStream<TangentsBufferType>(RealtimeMesh::FRealtimeMeshLocalVertexFactory::TangentsStreamName);
				TangentsBuffer->template Append<TangentsBufferType>(TangentsToCopy, [&MeshData](int32 Index) -> TangentsBufferType
					{
						const FVector Normal = MeshData.Normals.IsValidIndex(Index) ? MeshData.Normals[Index] : FVector::ZAxisVector;
						const FVector Tangent = MeshData.Tangents.IsValidIndex(Index) ? MeshData.Tangents[Index] : FVector::XAxisVector;
						const FVector Binormal = MeshData.Binormals.IsValidIndex(Index) ? MeshData.Binormals[Index] : FVector::CrossProduct(Normal, Tangent);
						const float TangentYSign = GetBasisDeterminantSign(Tangent, Binormal, Normal);

						return TangentsBufferType(FVector4(Normal), FVector4(Tangent, TangentYSign));
					});

				if (TangentsToCopy < MeshData.Positions.Num())
				{
					TangentsBuffer->AddZeroed(MeshData.Positions.Num() - TangentsToCopy);
				}
			}

			// Build color buffer from linear colors
			{
				const auto ColorBuffer = Builder->CreateVertexStream<FColor>(RealtimeMesh::FRealtimeMeshLocalVertexFactory::ColorStreamName);
				if (MeshData.LinearColors.Num() > 0)
				{
					ColorBuffer->Append<FColor>(MeshData.Positions.Num(), [&MeshData](int32 Index) -> FColor
						{
							return MeshData.LinearColors.IsValidIndex(Index) ? MeshData.LinearColors[Index].ToFColor(false) : FColor::White;
						});
				}
				else
				{
					ColorBuffer->Append<FColor>(MeshData.Positions.Num(), [&MeshData](int32 Index) -> FColor
						{
							return MeshData.Colors.IsValidIndex(Index) ? MeshData.Colors[Index] : FColor::White;
						});
				}
			}

			// Build tex coord buffer
			{
				if (MeshData.UV3.Num() > 0)
				{
					const int32 TexCoordsToCopy = FMath::Min(MeshData.Positions.Num(), FMath::Max(FMath::Max(MeshData.UV0.Num(), MeshData.UV1.Num()), FMath::Max(MeshData.UV2.Num(), MeshData.UV3.Num())));
					using TexCoordBufferType = RealtimeMesh::FRealtimeMeshTexCoord<TexCoordType, 4>;
					auto TexCoordBuffer = Builder->CreateVertexStream<TexCoordBufferType>(RealtimeMesh::FRealtimeMeshLocalVertexFactory::TexCoordsStreamName);
					TexCoordBuffer->template Append<TexCoordBufferType>(TexCoordsToCopy, [&MeshData](int32 Index) -> TexCoordBufferType
						{
							TexCoordBufferType NewUVs;
							NewUVs[0] = MeshData.UV0.IsValidIndex(Index) ? FVector2f(MeshData.UV0[Index]) : FVector2f::ZeroVector;
							NewUVs[1] = MeshData.UV1.IsValidIndex(Index) ? FVector2f(MeshData.UV1[Index]) : FVector2f::ZeroVector;
							NewUVs[2] = MeshData.UV2.IsValidIndex(Index) ? FVector2f(MeshData.UV2[Index]) : FVector2f::ZeroVector;
							NewUVs[3] = MeshData.UV3.IsValidIndex(Index) ? FVector2f(MeshData.UV3[Index]) : FVector2f::ZeroVector;
							return NewUVs;
						});

					if (TexCoordsToCopy < MeshData.Positions.Num())
					{
						TexCoordBuffer->AddZeroed(MeshData.Positions.Num() - TexCoordsToCopy);
					}
				}
				else if (MeshData.UV2.Num() > 0)
				{
					const int32 TexCoordsToCopy = FMath::Min(MeshData.Positions.Num(), FMath::Max(FMath::Max(MeshData.UV0.Num(), MeshData.UV1.Num()), MeshData.UV2.Num()));
					using TexCoordBufferType = RealtimeMesh::FRealtimeMeshTexCoord<TexCoordType, 3>;
					auto TexCoordBuffer = Builder->CreateVertexStream<TexCoordBufferType>(RealtimeMesh::FRealtimeMeshLocalVertexFactory::TexCoordsStreamName);
					TexCoordBuffer->template Append<TexCoordBufferType>(TexCoordsToCopy, [&MeshData](int32 Index) -> TexCoordBufferType
						{
							TexCoordBufferType NewUVs;
							NewUVs[0] = MeshData.UV0.IsValidIndex(Index) ? FVector2f(MeshData.UV0[Index]) : FVector2f::ZeroVector;
							NewUVs[1] = MeshData.UV1.IsValidIndex(Index) ? FVector2f(MeshData.UV1[Index]) : FVector2f::ZeroVector;
							NewUVs[2] = MeshData.UV2.IsValidIndex(Index) ? FVector2f(MeshData.UV2[Index]) : FVector2f::ZeroVector;
							return NewUVs;
						});

					if (TexCoordsToCopy < MeshData.Positions.Num())
					{
						TexCoordBuffer->AddZeroed(MeshData.Positions.Num() - TexCoordsToCopy);
					}
				}
				else if (MeshData.UV1.Num() > 0)
				{
					const int32 TexCoordsToCopy = FMath::Min(MeshData.Positions.Num(), FMath::Max(MeshData.UV0.Num(), MeshData.UV1.Num()));
					using TexCoordBufferType = RealtimeMesh::FRealtimeMeshTexCoord<TexCoordType, 2>;
					auto TexCoordBuffer = Builder->CreateVertexStream<TexCoordBufferType>(RealtimeMesh::FRealtimeMeshLocalVertexFactory::TexCoordsStreamName);
					TexCoordBuffer->template Append<TexCoordBufferType>(TexCoordsToCopy, [&MeshData](int32 Index) -> TexCoordBufferType
						{
							TexCoordBufferType NewUVs;
							NewUVs[0] = MeshData.UV0.IsValidIndex(Index) ? FVector2f(MeshData.UV0[Index]) : FVector2f::ZeroVector;
							NewUVs[1] = MeshData.UV1.IsValidIndex(Index) ? FVector2f(MeshData.UV1[Index]) : FVector2f::ZeroVector;
							return NewUVs;
						});

					if (TexCoordsToCopy < MeshData.Positions.Num())
					{
						TexCoordBuffer->AddZeroed(MeshData.Positions.Num() - TexCoordsToCopy);
					}
				}
				else
				{
					const int32 TexCoordsToCopy = FMath::Min(MeshData.Positions.Num(), MeshData.UV0.Num());
					using TexCoordBufferType = RealtimeMesh::FRealtimeMeshTexCoord<TexCoordType, 1>;
					auto TexCoordBuffer = Builder->CreateVertexStream<TexCoordBufferType>(RealtimeMesh::FRealtimeMeshLocalVertexFactory::TexCoordsStreamName);
					TexCoordBuffer->template Append<TexCoordBufferType>(MeshData.Positions.Num(), [&MeshData](int32 Index) -> TexCoordBufferType
						{
							TexCoordBufferType NewUVs;
							NewUVs[0] = MeshData.UV0.IsValidIndex(Index) ? FVector2f(MeshData.UV0[Index]) : FVector2f::ZeroVector;
							return NewUVs;
						});

					if (TexCoordsToCopy < MeshData.Positions.Num())
					{
						TexCoordBuffer->AddZeroed(MeshData.Positions.Num() - TexCoordsToCopy);
					}
				}
			}

			const TArray<int32>* FinalTriangles = &MeshData.Triangles;
			TArray<int32> CleanTriangles;
			if (bRemoveDegenerates)
			{
				// Get triangle indices, clamping to vertex range
				const int32 MaxIndex = MeshData.Positions.Num() - 1;
				const auto GetTriIndices = [&MeshData, MaxIndex](int32 Idx)
				{
					return TTuple<int32, int32, int32>(FMath::Min(MeshData.Triangles[Idx], MaxIndex),
						FMath::Min(MeshData.Triangles[Idx + 1], MaxIndex),
						FMath::Min(MeshData.Triangles[Idx + 2], MaxIndex));
				};

				const int32 NumTriIndices = (MeshData.Triangles.Num() / 3) * 3; // Ensure number of triangle indices is multiple of three

				// Detect degenerate triangles, i.e. non-unique vertex indices within the same triangle

				CleanTriangles.Reserve(MeshData.Triangles.Num());
				int32 NumDegenerateTriangles = 0;
				for (int32 IndexIdx = 0; IndexIdx < NumTriIndices; IndexIdx += 3)
				{
					int32 A, B, C;
					Tie(A, B, C) = GetTriIndices(IndexIdx);

					if (!(A == B || A == C || B == C))
					{
						CleanTriangles.Add(A);
						CleanTriangles.Add(B);
						CleanTriangles.Add(C);
					}
					else
					{
						NumDegenerateTriangles++;
					}
				}

				if (NumDegenerateTriangles > 0)
				{
					UE_LOG(LogTemp, Warning, TEXT("Detected %d degenerate triangle%s with non-unique vertex indices for created mesh section in '%s'; degenerate triangles will be dropped."),
						NumDegenerateTriangles, NumDegenerateTriangles > 1 ? TEXT("s") : TEXT(""), *ComponentName.ToString());
				}
				check(NumDegenerateTriangles == 0);

				FinalTriangles = &CleanTriangles;
			}


			// Build triangles buffer
			auto IndexBuffer = Builder->CreateIndexStream<IndexType>(RealtimeMesh::FRealtimeMeshLocalVertexFactory::TrianglesStreamName);
			IndexBuffer->template Append<IndexType>(FinalTriangles->Num(), [&FinalTriangles](int32 Index) -> IndexType
				{
					return (*FinalTriangles)[Index];
				});

			return Builder;
		}

		template<typename TangentType, typename TexCoordType>
		inline TSharedRef<RealtimeMesh::FRealtimeMeshVertexDataBuilder> BuildMeshData(FName ComponentName,
			const FChunkMeshData& MeshData, bool bRemoveDegenerates)
		{
			if (MeshData.Positions.Num() > TNumericLimits<uint16>::Max())
			{
				return BuildMeshData<TangentType, TexCoordType, uint32>(ComponentName, MeshData, bRemoveDegenerates);
			}
			else
			{
				return BuildMeshData<TangentType, TexCoordType, uint16>(ComponentName, MeshData, bRemoveDegenerates);
			}
		}

		template<typename TangentType>
		inline TSharedRef<RealtimeMesh::FRealtimeMeshVertexDataBuilder> BuildMeshData(FName ComponentName, const FChunkMeshData& MeshData,
			bool bRemoveDegenerates)
		{
			if (MeshData.bUseHighPrecisionTexCoords)
			{
				return BuildMeshData<TangentType, FVector2f>(ComponentName, MeshData, bRemoveDegenerates);
			}
			else
			{
				return BuildMeshData<TangentType, FVector2DHalf>(ComponentName, MeshData, bRemoveDegenerates);
			}
		}

		inline TSharedRef<RealtimeMesh::FRealtimeMeshVertexDataBuilder> BuildMeshData(FName ComponentName,
			const FChunkMeshData& MeshData, bool bRemoveDegenerates)
		{
			if (MeshData.bUseHighPrecisionTangents)
			{
				return BuildMeshData<FPackedRGBA16N>(ComponentName, MeshData, bRemoveDegenerates);
			}
			else
			{
				return BuildMeshData<FPackedNormal>(ComponentName, MeshData, bRemoveDegenerates);
			}
		}
	}

	UChunkSectionSimple::UChunkSectionSimple(const FRealtimeMeshClassFactoryRef& InClassFactory, const FRealtimeMeshRef& InMesh,
		FRealtimeMeshSectionKey InKey, const FRealtimeMeshSectionConfig& InConfig, const FRealtimeMeshStreamRange& InStreamRange)
		: FRealtimeMeshSectionData(InClassFactory, InMesh, InKey, InConfig, InStreamRange)
		, bShouldCreateMeshCollision(false)
	{
	}

	void UChunkSectionSimple::SetShouldCreateCollision(bool bNewShouldCreateMeshCollision)
	{
		if (bShouldCreateMeshCollision != bNewShouldCreateMeshCollision)
		{
			bShouldCreateMeshCollision = bNewShouldCreateMeshCollision;
			if (const auto Mesh = MeshWeak.Pin())
			{
				Mesh->MarkCollisionDirty();
			}
		}
	}

	void UChunkSectionSimple::OnStreamsChanged(const TArray<FRealtimeMeshStreamKey>& AddedOrUpdatedStreams,
		const TArray<FRealtimeMeshStreamKey>& RemovedStreams)
	{
		FRealtimeMeshSectionData::OnStreamsChanged(AddedOrUpdatedStreams, RemovedStreams);

		const auto PositionStreamKey = FRealtimeMeshStreamKey(ERealtimeMeshStreamType::Vertex, FRealtimeMeshLocalVertexFactory::PositionStreamName);
		const auto TriangleStreamKey = FRealtimeMeshStreamKey(ERealtimeMeshStreamType::Vertex, FRealtimeMeshLocalVertexFactory::TrianglesStreamName);
		const auto TexCoordStreamKey = FRealtimeMeshStreamKey(ERealtimeMeshStreamType::Vertex, FRealtimeMeshLocalVertexFactory::TexCoordsStreamName);

		bool bShouldMarkCollisionDirty = false;


		// Recalculate the bounds when needed
		if ((AddedOrUpdatedStreams.Num() == 0 && RemovedStreams.Num() == 0) ||
			AddedOrUpdatedStreams.Contains(PositionStreamKey) || RemovedStreams.Contains(PositionStreamKey))
		{
			UpdateBounds(RecalculateBounds());
			bShouldMarkCollisionDirty = bShouldCreateMeshCollision;
		}
		else if (AddedOrUpdatedStreams.Contains(TriangleStreamKey) || RemovedStreams.Contains(TriangleStreamKey) ||
			AddedOrUpdatedStreams.Contains(TexCoordStreamKey) || RemovedStreams.Contains(TexCoordStreamKey))
		{
			bShouldMarkCollisionDirty = bShouldCreateMeshCollision;
		}

		if (bShouldMarkCollisionDirty)
		{
			if (const auto Mesh = MeshWeak.Pin())
			{
				Mesh->MarkCollisionDirty();
			}
		}
	}

	void UChunkSectionSimple::UpdateStreamRange(const FRealtimeMeshStreamRange& InRange)
	{
		FRealtimeMeshSectionData::UpdateStreamRange(InRange);

		UpdateBounds(RecalculateBounds());
		if (const auto Mesh = MeshWeak.Pin())
		{
			Mesh->MarkCollisionDirty();
		}
	}

	bool UChunkSectionSimple::GetPhysicsTriMeshData(FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
	{
		if (bShouldCreateMeshCollision)
		{
			if (const auto SectionGroup = GetSectionGroupAs<FChunkSectionGroupSimple>())
			{
				const auto PositionStream = SectionGroup->GetStream(FRealtimeMeshStreamKey(
					ERealtimeMeshStreamType::Vertex, FRealtimeMeshLocalVertexFactory::PositionStreamName));
				const auto TriangleStream = SectionGroup->GetStream(FRealtimeMeshStreamKey(
					ERealtimeMeshStreamType::Index, FRealtimeMeshLocalVertexFactory::TrianglesStreamName));

				const auto TexCoordsStream = SectionGroup->GetStream(FRealtimeMeshStreamKey(
					ERealtimeMeshStreamType::Vertex, FRealtimeMeshLocalVertexFactory::TexCoordsStreamName));

				if (PositionStream && TriangleStream)
				{
					if (PositionStream->Num() >= 3 && TriangleStream->Num() >= 3)
					{
						const int32 StartVertexIndex = CollisionData->Vertices.Num();

						// Copy in the vertices
						auto PositionsView = PositionStream->GetArrayView<FVector3f>();
						CollisionData->Vertices.Append(PositionsView.GetData(), PositionsView.Num());

						if (CollisionData->UVs.Num() < 1)
						{
							CollisionData->UVs.SetNum(1);
						}

						int32 NumRemainingTexCoords = PositionStream->Num();
						if (TexCoordsStream)
						{
							if (TexCoordsStream->GetLayout() == RealtimeMesh::GetRealtimeMeshBufferLayout<FVector2DHalf>())
							{
								const auto TexCoordsView = TexCoordsStream->GetArrayView<FVector2DHalf>().Left(PositionStream->Num());
								CollisionData->UVs[0].SetNum(TexCoordsView.Num());
								for (int32 TexCoordIdx = 0; TexCoordIdx < TexCoordsView.Num(); TexCoordIdx++)
								{
									CollisionData->UVs[0][TexCoordIdx] = FVector2D(TexCoordsView[TexCoordIdx]);
								}
							}
							else
							{
								const auto TexCoordsView = TexCoordsStream->GetArrayView<FVector2f>().Left(PositionStream->Num());
								CollisionData->UVs[0].SetNum(TexCoordsView.Num());
								for (int32 TexCoordIdx = 0; TexCoordIdx < TexCoordsView.Num(); TexCoordIdx++)
								{
									CollisionData->UVs[0][TexCoordIdx] = FVector2D(TexCoordsView[TexCoordIdx]);
								}
							}
							NumRemainingTexCoords -= TexCoordsStream->Num();
						}

						if (NumRemainingTexCoords > 0)
						{
							CollisionData->UVs[0].AddZeroed(NumRemainingTexCoords);
						}

						if (TriangleStream->GetLayout() == RealtimeMesh::GetRealtimeMeshBufferLayout<uint16>())
						{
							const auto TrianglesView = TriangleStream->GetArrayView<uint16>();

							for (int32 TriIdx = 0; TriIdx < TrianglesView.Num(); TriIdx += 3)
							{
								FTriIndices& Tri = CollisionData->Indices.AddDefaulted_GetRef();
								Tri.v0 = TrianglesView[TriIdx + 0] + StartVertexIndex;
								Tri.v1 = TrianglesView[TriIdx + 1] + StartVertexIndex;
								Tri.v2 = TrianglesView[TriIdx + 2] + StartVertexIndex;

								CollisionData->MaterialIndices.Add(Config.MaterialSlot);
							}
						}
						else
						{
							const auto TrianglesView = TriangleStream->GetArrayView<int32>();

							for (int32 TriIdx = 0; TriIdx < TrianglesView.Num(); TriIdx += 3)
							{
								FTriIndices& Tri = CollisionData->Indices.AddDefaulted_GetRef();
								Tri.v0 = TrianglesView[TriIdx + 0] + StartVertexIndex;
								Tri.v1 = TrianglesView[TriIdx + 1] + StartVertexIndex;
								Tri.v2 = TrianglesView[TriIdx + 2] + StartVertexIndex;

								CollisionData->MaterialIndices.Add(Config.MaterialSlot);
							}
						}

						return true;
					}
				}
			}
		}
		return false;
	}

	bool UChunkSectionSimple::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
	{
		if (bShouldCreateMeshCollision)
		{
			if (const auto SectionGroup = GetSectionGroupAs<FChunkSectionGroupSimple>())
			{
				const auto PositionStream = SectionGroup->GetStream(FRealtimeMeshStreamKey(
					ERealtimeMeshStreamType::Vertex, FRealtimeMeshLocalVertexFactory::PositionStreamName));
				const auto TriangleStream = SectionGroup->GetStream(FRealtimeMeshStreamKey(
					ERealtimeMeshStreamType::Index, FRealtimeMeshLocalVertexFactory::TrianglesStreamName));

				if (PositionStream && TriangleStream)
				{
					if (PositionStream->Num() >= 3 && TriangleStream->Num() >= 3)
					{
						return true;
					}
				}
			}
		}
		return false;
	}

	bool UChunkSectionSimple::Serialize(FArchive& Ar)
	{
		const bool bResult = FRealtimeMeshSectionData::Serialize(Ar);

		Ar << bShouldCreateMeshCollision;

		return bResult;
	}

	FBoxSphereBounds3f UChunkSectionSimple::RecalculateBounds() const
	{
		TOptional<FBoxSphereBounds3f> Bounds;

		if (const auto SectionGroup = GetSectionGroupAs<FChunkSectionGroupSimple>())
		{
			const auto Stream = SectionGroup->GetStream(FRealtimeMeshStreamKey(
				ERealtimeMeshStreamType::Vertex, FRealtimeMeshLocalVertexFactory::PositionStreamName));
			if (Stream && GetStreamRange().NumVertices() > 0 && GetStreamRange().GetMaxVertex() < Stream->Num())
			{
				auto TestLayout = GetRealtimeMeshBufferLayout<FVector3f>();
				if (Stream->GetLayout() == TestLayout)
				{
					const FVector3f* Points = Stream->GetData<FVector3f>() + GetStreamRange().GetMinVertex();
					Bounds = FBoxSphereBounds3f(Points, GetStreamRange().NumVertices());
				}
			}
		}

		// Set the bounds if we made one, or default them
		return Bounds.IsSet() ? Bounds.GetValue() : FBoxSphereBounds3f(FSphere3f(FVector3f::ZeroVector, 1.0f));
	}

	bool FChunkSectionGroupSimple::HasStreams() const
	{
		FReadScopeLock ScopeLock(Lock);
		return Streams.Num() > 0;
	}

	FRealtimeMeshDataStreamPtr FChunkSectionGroupSimple::GetStream(FRealtimeMeshStreamKey StreamKey) const
	{
		FReadScopeLock ScopeLock(Lock);
		return Streams.FindRef(StreamKey);
	}

	FRealtimeMeshStreamRange FChunkSectionGroupSimple::GetBaseRange() const
	{
		const FRealtimeMeshDataStreamPtr PositionStream = GetStream(FRealtimeMeshStreamKey(
			ERealtimeMeshStreamType::Vertex, FRealtimeMeshLocalVertexFactory::PositionStreamName));
		const FRealtimeMeshDataStreamPtr TriangleStream = GetStream(FRealtimeMeshStreamKey(ERealtimeMeshStreamType::Index, FRealtimeMeshLocalVertexFactory::TrianglesStreamName));

		return FRealtimeMeshStreamRange(0, PositionStream.IsValid() ? PositionStream->Num() : 0, 0, TriangleStream.IsValid() ? TriangleStream->Num() : 0);
	}

	void FChunkSectionGroupSimple::CreateOrUpdateStream(FRealtimeMeshStreamKey StreamKey, const FRealtimeMeshDataStreamRef& InStream)
	{
		FRWScopeLockEx ScopeLock(Lock, SLT_Write);
		Streams.FindOrAdd(StreamKey) = InStream;
		ScopeLock.Release();

		const auto UpdateData = InStream->GetStreamUpdateData(StreamKey);
		UpdateData->ConfigureBuffer(EBufferUsageFlags::Static, true);

		FRealtimeMeshSectionGroup::CreateOrUpdateStream(StreamKey, UpdateData);
	}

	void FChunkSectionGroupSimple::SetStreamData(const FChunkMeshData& MeshData)
	{
		if (MeshData.Positions.Num() < 3)
		{
			FMessageLog("RealtimeMesh").Error(FText::Format(
				LOCTEXT("RealtimeMeshSectionGroupSimple_SetStreamData_InvalidVertexCount", "Invalid vertex count {0} for mesh {1}"),
				MeshData.Positions.Num(), FText::FromName(GetMeshName())));
			return;
		}
		if (MeshData.Triangles.Num() < 3)
		{
			FMessageLog("RealtimeMesh").Error(FText::Format(
				LOCTEXT("RealtimeMeshSectionGroupSimple_SetStreamData_InvalidTriangleCount", "Invalid triangle count {0} for mesh {1}"),
				MeshData.Triangles.Num(), FText::FromName(GetMeshName())));
			return;
		}

		const auto UpdateData = RealtimeMeshSimpleInternal::BuildMeshData(GetMeshName(), MeshData, false /*RemoveDegenerates*/);
		SetAllStreams(UpdateData->GetBuffers());
	}

	void FChunkSectionGroupSimple::SetAllStreams(const TMap<FRealtimeMeshStreamKey, FRealtimeMeshDataStreamPtr>& InStreams)
	{
		FRWScopeLockEx ScopeLock(Lock, SLT_Write);

		TArray<FRealtimeMeshStreamKey> ExistingStreamKeys;
		Streams.GetKeys(ExistingStreamKeys);

		TArray<FRealtimeMeshStreamKey> RemovedStreams;
		for (const auto& StreamKey : ExistingStreamKeys)
		{
			if (!InStreams.Contains(StreamKey))
			{
				Streams.Remove(StreamKey);
				RemovedStreams.Add(StreamKey);
			}
		}

		TArray<FRealtimeMeshStreamKey> UpdatedStreams;
		TArray<FRealtimeMeshSectionGroupStreamUpdateDataRef> UpdatedStreamsInitData;
		for (const auto& NewStream : InStreams)
		{
			if (Streams.Contains(NewStream.Key))
			{
				Streams[NewStream.Key] = NewStream.Value;
			}
			else
			{
				Streams.Add(NewStream.Key, NewStream.Value);
			}
			UpdatedStreams.Add(NewStream.Key);
			UpdatedStreamsInitData.Add(NewStream.Value->GetStreamUpdateData(NewStream.Key));
		}

		ScopeLock.Release();

		FRealtimeMeshSectionGroup::SetAllStreams(UpdatedStreams, RemovedStreams, MoveTemp(UpdatedStreamsInitData));
	}

	void FChunkSectionGroupSimple::ClearStream(FRealtimeMeshStreamKey StreamKey)
	{
		FRWScopeLockEx ScopeLock(Lock, SLT_Write);
		if (Streams.Contains(StreamKey))
		{
			Streams[StreamKey]->Empty();
			ScopeLock.Release();

			FRealtimeMeshSectionGroup::ClearStream(StreamKey);
		}
		else
		{
			FMessageLog("RealtimeMesh").Error(
				FText::Format(LOCTEXT("ClearStreamInvalid", "Attempted to clear invalid stream {0} in Mesh:{1}"),
					FText::FromString(StreamKey.ToString()), FText::FromName(GetParentName())));
		}
	}

	void FChunkSectionGroupSimple::RemoveStream(FRealtimeMeshStreamKey StreamKey)
	{
		FRWScopeLockEx ScopeLock(Lock, SLT_Write);
		if (Streams.Contains(StreamKey))
		{
			Streams[StreamKey]->Empty();
			ScopeLock.Release();

			FRealtimeMeshSectionGroup::RemoveStream(StreamKey);
		}
		else
		{
			FMessageLog("RealtimeMesh").Error(
				FText::Format(LOCTEXT("RemoveStreamInvalid", "Attempted to remove invalid stream {0} in Mesh:{1}"),
					FText::FromString(StreamKey.ToString()), FText::FromName(GetParentName())));
		}
	}


	FRealtimeMeshSectionGroupProxyInitializationParametersRef FChunkSectionGroupSimple::GetInitializationParams() const
	{
		auto InitParams = FRealtimeMeshSectionGroup::GetInitializationParams();

		InitParams->Streams.Reserve(Streams.Num());
		for (auto Stream : Streams)
		{
			InitParams->Streams.Add(Stream.Value->GetStreamUpdateData(Stream.Key));
		}

		return InitParams;
	}

	bool FChunkSectionGroupSimple::Serialize(FArchive& Ar)
	{
		const bool bResult = FRealtimeMeshSectionGroup::Serialize(Ar);

		if (ensure(bResult))
		{
			int32 NumStreams = Streams.Num();
			Ar << NumStreams;

			/*if (Ar.IsLoading())
			{
				Streams.Empty();
				for (int32 Index = 0; Index < NumStreams; Index++)
				{
					FRealtimeMeshStreamKey StreamKey;
					Ar << StreamKey;
					FRealtimeMeshDataStreamRef Stream = MakeShared<FRealtimeMeshDataStream>();
					Ar << (*Stream);

					Streams.Add(StreamKey, Stream);
				}
			}
			else
			{
				for (const auto& Stream : Streams)
				{
					FRealtimeMeshStreamKey key = Stream.Key;
					Ar << key;
					Ar << (*Stream.Value);
				}
			}*/
		}

		return bResult;
	}

	bool FChunkSectionGroupSimple::GetPhysicsTriMeshData(FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
	{
		FRWScopeLockEx ScopeLock(Lock, SLT_ReadOnly);
		const auto LocalSections = Sections;
		ScopeLock.Release();

		bool bHasSectionData = false;
		for (const auto& Section : LocalSections)
		{
			bHasSectionData |= StaticCastSharedRef<UChunkSectionSimple>(Section)->GetPhysicsTriMeshData(CollisionData, InUseAllTriData);
		}
		return bHasSectionData;
	}

	bool FChunkSectionGroupSimple::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
	{
		FRWScopeLockEx ScopeLock(Lock, SLT_ReadOnly);
		const auto LocalSections = Sections;
		ScopeLock.Release();

		for (const auto& Section : LocalSections)
		{
			if (StaticCastSharedRef<UChunkSectionSimple>(Section)->ContainsPhysicsTriMeshData(InUseAllTriData))
			{
				return true;
			}
		}
		return false;
	}

	bool UChunkLODSimple::GetPhysicsTriMeshData(FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
	{
		FRWScopeLockEx ScopeLock(Lock, SLT_ReadOnly);
		const auto LocalSectionGroups = SectionGroups;
		ScopeLock.Release();

		bool bHasSectionData = false;
		for (const auto& SectionGroup : LocalSectionGroups)
		{
			bHasSectionData |= StaticCastSharedRef<FChunkSectionGroupSimple>(SectionGroup)->GetPhysicsTriMeshData(CollisionData, InUseAllTriData);
		}
		return bHasSectionData;
	}

	bool UChunkLODSimple::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
	{
		FRWScopeLockEx ScopeLock(Lock, SLT_ReadOnly);
		const auto LocalSectionGroups = SectionGroups;
		ScopeLock.Release();

		for (const auto& SectionGroup : LocalSectionGroups)
		{
			if (StaticCastSharedRef<FChunkSectionGroupSimple>(SectionGroup)->ContainsPhysicsTriMeshData(InUseAllTriData))
			{
				return true;
			}
		}
		return false;
	}

	bool UChunkDataSimple::GetPhysicsTriMeshData(FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
	{
		CollisionData->bFlipNormals = true;
		CollisionData->bDeformableMesh = false;
		CollisionData->bFastCook = CollisionConfig.bShouldFastCookMeshes;

		if (LODs.IsValidIndex(0))
		{
			return StaticCastSharedRef<UChunkLODSimple>(LODs[0])->GetPhysicsTriMeshData(CollisionData, InUseAllTriData);
		}
		return false;
	}

	bool UChunkDataSimple::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
	{
		if (LODs.IsValidIndex(0))
		{
			return StaticCastSharedRef<UChunkLODSimple>(LODs[0])->ContainsPhysicsTriMeshData(InUseAllTriData);
		}
		return false;
	}
}

UChunk::UChunk(const FObjectInitializer& ObjectInitializer)
	: URealtimeMesh(ObjectInitializer)
	, MeshRef(new RealtimeMesh::UChunkDataSimple())
{
	MeshRef->InitializeLODs(RealtimeMesh::TFixedLODArray<FRealtimeMeshLODConfig> { FRealtimeMeshLODConfig() });
}

void UChunk::Reset(bool bCreateNewMeshData)
{
	Super::Reset(bCreateNewMeshData);
}

FRealtimeMeshSectionGroupKey UChunk::CreateSectionGroup(const FRealtimeMeshLODKey& LODKey)
{
	if (const auto LOD = GetMesh()->GetLOD(LODKey))
	{
		return LOD->CreateSectionGroup();
	}

	FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("CreateSectionGroup_InvalidLODKey", "CreateSectionGroup: Invalid LOD key {0}"), FText::FromString(LODKey.ToString())));
	return FRealtimeMeshSectionGroupKey();
}

FRealtimeMeshSectionGroupKey UChunk::CreateSectionGroupWithMesh(const FRealtimeMeshLODKey& LODKey, const FChunkMeshData& MeshData)
{
	if (const auto LOD = MeshRef->GetLOD(LODKey))
	{
		const FRealtimeMeshSectionGroupKey SectionGroupKey = LOD->CreateSectionGroup();
		const auto SectionGroup = StaticCastSharedPtr<RealtimeMesh::FChunkSectionGroupSimple>(LOD->GetSectionGroup(SectionGroupKey));
		check(SectionGroup);
		SectionGroup->SetStreamData(MeshData);

		return SectionGroupKey;
	}

	FMessageLog("RealtimeMesh").Error(
		FText::Format(LOCTEXT("CreateSectionGroupInvalidLOD", "Attempted to create section group in invalid LOD {0} in Mesh:{1}"),
			FText::FromString(LODKey.ToString()), FText::FromName(GetFName())));
	return FRealtimeMeshSectionGroupKey();
}

void UChunk::UpdateSectionGroupMesh(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FChunkMeshData& MeshData)
{
	if (const auto LOD = MeshRef->GetLOD(SectionGroupKey.GetLODKey()))
	{
		if (const auto SectionGroup = LOD->GetSectionGroupAs<RealtimeMesh::FChunkSectionGroupSimple>(SectionGroupKey))
		{
			SectionGroup->SetStreamData(MeshData);
		}
		else
		{
			FMessageLog("RealtimeMesh").Error(
				FText::Format(LOCTEXT("UpdateSectionGroupInvalid", "Attempted to update invalid section group {0} in Mesh:{1}"),
					FText::FromString(SectionGroupKey.ToString()), FText::FromName(GetFName())));
		}
	}
	else
	{
		FMessageLog("RealtimeMesh").Error(
			FText::Format(LOCTEXT("UpdateSectionGroupInvalidLOD", "Attempted to update section group in invalid LOD {0} in Mesh:{1}"),
				FText::FromString(SectionGroupKey.GetLODKey().ToString()), FText::FromName(GetFName())));
	}
}

void UChunk::RemoveSectionGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey)
{
	if (const auto LOD = GetMesh()->GetLOD(SectionGroupKey.GetLODKey()))
	{
		LOD->RemoveSectionGroup(SectionGroupKey);
	}
	else
	{
		FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("RemoveSectionGroup_InvalidLODKey", "RemoveSectionGroup: Invalid LOD key {0}"), FText::FromString(SectionGroupKey.GetLODKey().ToString())));
	}
}

FRealtimeMeshSectionKey UChunk::CreateSectionInGroup(const FRealtimeMeshSectionGroupKey& SectionGroupKey, const FRealtimeMeshSectionConfig& Config,
	const FRealtimeMeshStreamRange& StreamRange, bool bShouldCreateCollision)
{
	if (const auto LOD = GetMesh()->GetLOD(SectionGroupKey.GetLODKey()))
	{
		if (const auto SectionGroup = LOD->GetSectionGroup(SectionGroupKey))
		{
			const FRealtimeMeshSectionKey SectionKey = SectionGroup->CreateSection(Config, StreamRange);
			const auto& Section = SectionGroup->GetSectionAs<RealtimeMesh::UChunkSectionSimple>(SectionKey);
			check(Section);

			Section->SetShouldCreateCollision(bShouldCreateCollision);
			return SectionKey;
		}
		else
		{
			FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("CreateMeshSectionInGroup_InvalidSectionGroupKey", "CreateMeshSectionInGroup: Invalid section group key {0}"), FText::FromString(SectionGroupKey.ToString())));
		}
	}
	else
	{
		FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("CreateMeshSectionInGroup_InvalidLODKey", "CreateMeshSectionInGroup: Invalid LOD key {0}"), FText::FromString(SectionGroupKey.GetLODKey().ToString())));
	}
	return FRealtimeMeshSectionKey();
}

FRealtimeMeshSectionKey UChunk::CreateMeshSection(const FRealtimeMeshLODKey& LODKey, const FRealtimeMeshSectionConfig& Config,
	const FChunkMeshData& MeshData, bool bShouldCreateCollision)
{
	if (const auto LOD = MeshRef->GetLOD(LODKey))
	{
		const FRealtimeMeshSectionGroupKey SectionGroupKey = LOD->CreateSectionGroup();
		const auto SectionGroup = StaticCastSharedPtr<RealtimeMesh::FChunkSectionGroupSimple>(LOD->GetSectionGroup(SectionGroupKey));
		check(SectionGroup);
		SectionGroup->SetStreamData(MeshData);

		const FRealtimeMeshSectionKey SectionKey = SectionGroup->CreateSection(Config, SectionGroup->GetBaseRange());
		const auto& Section = SectionGroup->GetSectionAs<RealtimeMesh::UChunkSectionSimple>(SectionKey);
		check(Section);

		Section->SetShouldCreateCollision(bShouldCreateCollision);
		return SectionKey;
	}
	else
	{
		FMessageLog("RealtimeMesh").Error(
			FText::Format(LOCTEXT("CreateMeshSectionInvalidLOD", "Attempted to create section in invalid LOD {0} in Mesh:{1}"),
				FText::FromString(LODKey.ToString()), FText::FromName(GetFName())));
		return FRealtimeMeshSectionKey();
	}
}

void UChunk::UpdateSectionMesh(const FRealtimeMeshSectionKey& SectionKey, const FChunkMeshData& MeshData)
{
	if (const auto LOD = MeshRef->GetLOD(SectionKey.GetLODKey()))
	{
		if (const auto SectionGroup = LOD->GetSectionGroupAs<RealtimeMesh::FChunkSectionGroupSimple>(SectionKey.GetSectionGroupKey()))
		{
			if (SectionGroup->NumSections() == 1)
			{
				if (const auto Section = SectionGroup->GetSection(SectionKey))
				{
					SectionGroup->SetStreamData(MeshData);
					Section->UpdateStreamRange(SectionGroup->GetBaseRange());
				}
				else
				{
					FMessageLog("RealtimeMesh").Error(
						FText::Format(LOCTEXT("UpdateSectionInvalid", "Attempted to update invalid section {0} in Mesh:{1}"),
							FText::FromString(SectionKey.ToString()), FText::FromName(GetFName())));
				}
			}
			else
			{
				FMessageLog("RealtimeMesh").Error(
					FText::Format(LOCTEXT("UpdateSectionInvalidSectionGroup", "Attempted to update section {0} in section group with multiple sections in Mesh:{1}"),
						FText::FromString(SectionKey.ToString()), FText::FromName(GetFName())));
			}
		}
		else
		{
			FMessageLog("RealtimeMesh").Error(
				FText::Format(LOCTEXT("UpdateSectionInvalidSectionGroup", "Attempted to update section {0} in invalid section group in Mesh:{1}"),
					FText::FromString(SectionKey.ToString()), FText::FromName(GetFName())));
		}
	}
	else
	{
		FMessageLog("RealtimeMesh").Error(
			FText::Format(LOCTEXT("UpdateSectionInvalidLOD", "Attempted to update section in invalid LOD {0} in Mesh:{1}"),
				FText::FromString(SectionKey.GetLODKey().ToString()), FText::FromName(GetFName())));
	}
}

void UChunk::UpdateSectionSegment(const FRealtimeMeshSectionKey& SectionKey,
	const FRealtimeMeshStreamRange& StreamRange)
{
	if (const auto LOD = MeshRef->GetLOD(SectionKey.GetLODKey()))
	{
		if (const auto SectionGroup = LOD->GetSectionGroupAs<RealtimeMesh::FChunkSectionGroupSimple>(SectionKey.GetSectionGroupKey()))
		{
			if (const auto Section = SectionGroup->GetSection(SectionKey))
			{
				Section->UpdateStreamRange(StreamRange);
			}
			else
			{
				FMessageLog("RealtimeMesh").Error(
					FText::Format(LOCTEXT("UpdateSectionInvalid", "Attempted to update invalid section {0} in Mesh:{1}"),
						FText::FromString(SectionKey.ToString()), FText::FromName(GetFName())));
			}
		}
		else
		{
			FMessageLog("RealtimeMesh").Error(
				FText::Format(LOCTEXT("UpdateSectionInvalidSectionGroup", "Attempted to update section {0} in invalid section group in Mesh:{1}"),
					FText::FromString(SectionKey.ToString()), FText::FromName(GetFName())));
		}
	}
	else
	{
		FMessageLog("RealtimeMesh").Error(
			FText::Format(LOCTEXT("UpdateSectionInvalidLOD", "Attempted to update section in invalid LOD {0} in Mesh:{1}"),
				FText::FromString(SectionKey.GetLODKey().ToString()), FText::FromName(GetFName())));
	}
}

FRealtimeMeshSectionConfig UChunk::GetSectionConfig(const FRealtimeMeshSectionKey& SectionKey) const
{
	if (const auto LOD = GetMesh()->GetLOD(SectionKey.GetLODKey()))
	{
		if (const auto SectionGroup = LOD->GetSectionGroup(SectionKey.GetSectionGroupKey()))
		{
			if (const auto Section = SectionGroup->GetSection(SectionKey))
			{
				return Section->GetConfig();
			}
			else
			{
				FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("GetSectionConfig_InvalidSectionKey", "GetSectionConfig: Invalid section key {0}"), FText::FromString(SectionKey.ToString())));
			}
		}
		else
		{
			FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("GetSectionConfig_InvalidSectionGroupKey", "GetSectionConfig: Invalid section group key {0}"), FText::FromString(SectionKey.GetSectionGroupKey().ToString())));
		}
	}
	else
	{
		FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("GetSectionConfig_InvalidLODKey", "GetSectionConfig: Invalid LOD key {0}"), FText::FromString(SectionKey.GetLODKey().ToString())));
	}
	return FRealtimeMeshSectionConfig();
}

void UChunk::UpdateSectionConfig(const FRealtimeMeshSectionKey& SectionKey, const FRealtimeMeshSectionConfig& Config)
{
	if (const auto LOD = GetMesh()->GetLOD(SectionKey.GetLODKey()))
	{
		if (const auto SectionGroup = LOD->GetSectionGroup(SectionKey.GetSectionGroupKey()))
		{
			if (const auto Section = SectionGroup->GetSection(SectionKey))
			{
				Section->UpdateConfig(Config);
			}
			else
			{
				FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("UpdateSectionConfig_InvalidSectionKey", "UpdateSectionConfig: Invalid section key {0}"), FText::FromString(SectionKey.ToString())));
			}
		}
		else
		{
			FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("UpdateSectionConfig_InvalidSectionGroupKey", "UpdateSectionConfig: Invalid section group key {0}"), FText::FromString(SectionKey.GetSectionGroupKey().ToString())));
		}
	}
	else
	{
		FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("UpdateSectionConfig_InvalidLODKey", "UpdateSectionConfig: Invalid LOD key {0}"), FText::FromString(SectionKey.GetLODKey().ToString())));
	}
}

bool UChunk::IsSectionVisible(const FRealtimeMeshSectionKey& SectionKey) const
{
	if (const auto LOD = GetMesh()->GetLOD(SectionKey.GetLODKey()))
	{
		if (const auto SectionGroup = LOD->GetSectionGroup(SectionKey.GetSectionGroupKey()))
		{
			if (const auto Section = SectionGroup->GetSection(SectionKey))
			{
				return Section->IsVisible();
			}
			else
			{
				FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("IsSectionVisible_InvalidSectionKey", "IsSectionVisible: Invalid section key {0}"), FText::FromString(SectionKey.ToString())));
			}
		}
		else
		{
			FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("IsSectionVisible_InvalidSectionGroupKey", "IsSectionVisible: Invalid section group key {0}"), FText::FromString(SectionKey.GetSectionGroupKey().ToString())));
		}
	}
	else
	{
		FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("IsSectionVisible_InvalidLODKey", "IsSectionVisible: Invalid LOD key {0}"), FText::FromString(SectionKey.GetLODKey().ToString())));
	}
	return false;
}

void UChunk::SetSectionVisibility(const FRealtimeMeshSectionKey& SectionKey, bool bIsVisible)
{
	if (const auto LOD = GetMesh()->GetLOD(SectionKey.GetLODKey()))
	{
		if (const auto SectionGroup = LOD->GetSectionGroup(SectionKey.GetSectionGroupKey()))
		{
			if (const auto Section = SectionGroup->GetSection(SectionKey))
			{
				Section->SetVisibility(bIsVisible);
			}
			else
			{
				FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("SetSectionVisibility_InvalidSectionKey", "SetSectionVisibility: Invalid section key {0}"), FText::FromString(SectionKey.ToString())));
			}
		}
		else
		{
			FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("SetSectionVisibility_InvalidSectionGroupKey", "SetSectionVisibility: Invalid section group key {0}"), FText::FromString(SectionKey.GetSectionGroupKey().ToString())));
		}
	}
	else
	{
		FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("SetSectionVisibility_InvalidLODKey", "SetSectionVisibility: Invalid LOD key {0}"), FText::FromString(SectionKey.GetLODKey().ToString())));
	}
}

bool UChunk::IsSectionCastingShadow(const FRealtimeMeshSectionKey& SectionKey) const
{
	if (const auto LOD = GetMesh()->GetLOD(SectionKey.GetLODKey()))
	{
		if (const auto SectionGroup = LOD->GetSectionGroup(SectionKey.GetSectionGroupKey()))
		{
			if (const auto Section = SectionGroup->GetSection(SectionKey))
			{
				return Section->IsCastingShadow();
			}
			else
			{
				FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("IsSectionCastingShadow_InvalidSectionKey", "IsSectionCastingShadow: Invalid section key {0}"), FText::FromString(SectionKey.ToString())));
			}
		}
		else
		{
			FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("IsSectionCastingShadow_InvalidSectionGroupKey", "IsSectionCastingShadow: Invalid section group key {0}"), FText::FromString(SectionKey.GetSectionGroupKey().ToString())));
		}
	}
	else
	{
		FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("IsSectionCastingShadow_InvalidLODKey", "IsSectionCastingShadow: Invalid LOD key {0}"), FText::FromString(SectionKey.GetLODKey().ToString())));
	}
	return false;
}

void UChunk::SetSectionCastShadow(const FRealtimeMeshSectionKey& SectionKey, bool bCastShadow)
{
	if (const auto LOD = GetMesh()->GetLOD(SectionKey.GetLODKey()))
	{
		if (const auto SectionGroup = LOD->GetSectionGroup(SectionKey.GetSectionGroupKey()))
		{
			if (const auto Section = SectionGroup->GetSection(SectionKey))
			{
				Section->SetCastShadow(bCastShadow);
			}
			else
			{
				FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("SetSectionCastShadow_InvalidSectionKey", "SetSectionCastShadow: Invalid section key {0}"), FText::FromString(SectionKey.ToString())));
			}
		}
		else
		{
			FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("SetSectionCastShadow_InvalidSectionGroupKey", "SetSectionCastShadow: Invalid section group key {0}"), FText::FromString(SectionKey.GetSectionGroupKey().ToString())));
		}
	}
	else
	{
		FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("SetSectionCastShadow_InvalidLODKey", "SetSectionCastShadow: Invalid LOD key {0}"), FText::FromString(SectionKey.GetLODKey().ToString())));
	}
}

void UChunk::RemoveSection(const FRealtimeMeshSectionKey& SectionKey)
{
	if (const auto LOD = GetMesh()->GetLOD(SectionKey.GetLODKey()))
	{
		if (const auto SectionGroup = LOD->GetSectionGroup(SectionKey.GetSectionGroupKey()))
		{
			SectionGroup->RemoveSection(SectionKey);
		}
		else
		{
			FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("RemoveSection_InvalidSectionGroupKey", "RemoveSection: Invalid section group key {0}"), FText::FromString(SectionKey.GetSectionGroupKey().ToString())));
		}
	}
	else
	{
		FMessageLog("RealtimeMesh").Error(FText::Format(LOCTEXT("RemoveSection_InvalidLODKey", "RemoveSection: Invalid LOD key {0}"), FText::FromString(SectionKey.GetLODKey().ToString())));
	}
}



#undef LOCTEXT_NAMESPACE
