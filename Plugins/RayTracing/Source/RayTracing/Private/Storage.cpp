#include "Common.h"
#include "Storage.h"
#include "Async/ParallelFor.h"

namespace RayTracing
{
	// 16
	TDoubleLinkedList<FTileDataLayer*> FTileDataLayer::AllUncompressedTiles;

	// 18
	int64 FTileDataLayer::Compress(bool bParallelCompression)
	{
		const int32 VirtualTileSize = GPreviewVirtualTileSize;

		const int32 CompressMemoryBound = FCompression::CompressMemoryBound(NAME_LZ4, sizeof(FLinearColor) * VirtualTileSize * VirtualTileSize);

		if (Data.Num() > 0)
		{
			CompressedData.Empty();
			CompressedData.AddUninitialized(CompressMemoryBound);
			int32 CompressedSize = CompressMemoryBound;
			check(FCompression::CompressMemory(NAME_LZ4, CompressedData.GetData(), CompressedSize, Data.GetData(), sizeof(FLinearColor) * VirtualTileSize * VirtualTileSize));
			CompressedData.SetNum(CompressedSize);
			Data.Empty();

			check(bNodeAddedToList);
			if (!bParallelCompression)
			{
				AllUncompressedTiles.RemoveNode(&Node, false);
			}
			bNodeAddedToList = false;

			return sizeof(FLinearColor) * VirtualTileSize * VirtualTileSize - CompressedSize;
		}
		else
		{
			check(CompressedData.Num() > 0);
			return 0;
		}
	}

	// 147
	FLightmapRenderState::FLightmapRenderState(Initializer InInitializer, FGeometryInstanceRenderStateRef GeometryInstanceRef)
		: Name(InInitializer.Name)
		, ResourceCluster(InInitializer.ResourceCluster)
		, LightMapCoordinateScaleBias(InInitializer.LightMapCoordinateScaleBias)
		, GeometryInstanceRef(GeometryInstanceRef)
		, Size(InInitializer.Size)
		, MaxLevel(InInitializer.MaxLevel)
	{
		for (int32 MipLevel = 0; MipLevel <= MaxLevel; MipLevel++)
		{
			TileStates.AddDefaulted(GetPaddedSizeInTilesAtMipLevel(MipLevel).X * GetPaddedSizeInTilesAtMipLevel(MipLevel).Y);
		}

		for (int32 MipLevel = 0; MipLevel <= MaxLevel; MipLevel++)
		{
			TileRelevantLightSampleCountStates.AddDefaulted(GetPaddedSizeInTilesAtMipLevel(MipLevel).X * GetPaddedSizeInTilesAtMipLevel(MipLevel).Y);
		}

		{
			FPrecomputedLightingUniformParameters Parameters;

			{
				Parameters.StaticShadowMapMasks = FVector4(1, 1, 1, 1);
				Parameters.InvUniformPenumbraSizes = FVector4(0, 0, 0, 0);
				Parameters.ShadowMapCoordinateScaleBias = FVector4(1, 1, 0, 0);

				const uint32 NumCoef = FMath::Max<uint32>(NUM_HQ_LIGHTMAP_COEF, NUM_LQ_LIGHTMAP_COEF);
				for (uint32 CoefIndex = 0; CoefIndex < NumCoef; ++CoefIndex)
				{
					Parameters.LightMapScale[CoefIndex] = FVector4(1, 1, 1, 1);
					Parameters.LightMapAdd[CoefIndex] = FVector4(0, 0, 0, 0);
				}

				FMemory::Memzero(Parameters.LightmapVTPackedPageTableUniform);

				for (uint32 LayerIndex = 0u; LayerIndex < 5u; ++LayerIndex)
				{
					Parameters.LightmapVTPackedUniform[LayerIndex] = FUintVector4(ForceInitToZero);
				}
			}

			Parameters.LightMapCoordinateScaleBias = LightMapCoordinateScaleBias;

			SetPrecomputedLightingBuffer(TUniformBufferRef<FPrecomputedLightingUniformParameters>::CreateUniformBufferImmediate(Parameters, UniformBuffer_MultiFrame));
		}
	}

	
	// 220
	bool FLightmapRenderState::DoesTileHaveValidCPUData(FTileVirtualCoordinates Coords, int32 CurrentRevision)
	{
		return RetrieveTileState(Coords).CPURevision == CurrentRevision;
	}
}