// Ref: LightmapStorage.h

#pragma once

#include "Common.h"
#include "EntityArray.h"
#include "LightMap.h"
#include "UObject/GCObjectScopeGuard.h"
#include "VT/LightmapVirtualTexture.h"
// TBB suffers from extreme fragmentation problem in editor
#include "Core/Private/HAL/Allocators/AnsiAllocator.h"

namespace RayTracing
{
	// 17
	struct FTileDataLayer
	{
		TArray<FLinearColor, FAnsiAllocator> Data;
		TArray<uint8> CompressedData;

		TDoubleLinkedList<FTileDataLayer*>::TDoubleLinkedListNode Node;
		bool bNodeAddedToList = false;

		static TDoubleLinkedList<FTileDataLayer*> AllUncompressedTiles;

		FTileDataLayer() : Node(this)
		{
			Data.AddZeroed(GPreviewVirtualTileSize * GPreviewVirtualTileSize);

			AllUncompressedTiles.AddHead(&Node);
			bNodeAddedToList = true;
		}

		~FTileDataLayer()
		{
			if (bNodeAddedToList)
			{
				AllUncompressedTiles.RemoveNode(&Node, false);
			}
		}

		int64 Compress(bool bParallelCompression = false);
		void Decompress();
		void AllocateForWrite();
		static void Evict();
	};

	// 49
	struct FTileStorage
	{
		TUniquePtr<FTileDataLayer> CPUTextureData[(int32)ELightMapVirtualTextureType::Count];
		TUniquePtr<FTileDataLayer> CPUTextureRawData[(int32)ELightMapVirtualTextureType::Count];

		FTileStorage()
		{
			for (int32 Index = 0; Index < UE_ARRAY_COUNT(CPUTextureData); Index++)
			{
				CPUTextureData[Index] = MakeUnique<FTileDataLayer>();
			}

			for (int32 Index = 0; Index < UE_ARRAY_COUNT(CPUTextureRawData); Index++)
			{
				CPUTextureRawData[Index] = MakeUnique<FTileDataLayer>();
			}
		}
	};

	// 68
	class FLightmap
	{
	public:
		FLightmap(FString InName, FIntPoint InSize);

		void CreateGameThreadResources();

		FIntPoint GetPaddedSizeInTiles() const
		{
			return FIntPoint(
				FMath::DivideAndRoundUp(Size.X, GPreviewVirtualTileSize),
				FMath::DivideAndRoundUp(Size.Y, GPreviewVirtualTileSize));
		}

		FString Name;
		FIntPoint Size;
		
		// 84
		TUniquePtr<FLightmapResourceCluster> ResourceCluster;
		TUniquePtr<FGCObjectScopeGuard> TextureUObjectGuard = nullptr;
		ULightMapVirtualTexture2D* TextureUObject = nullptr;
		TUniquePtr<FMeshMapBuildData> MeshMapBuildData;
		TRefCountPtr<FLightMap2D> LightmapObject;
		
		// 90
		int32 NumStationaryLightsPerShadowChannel[4] = { 0, 0, 0, 0 };
	};

	// 94
	using FlightmapRef = TEntityArray<FLightmap>::EntityRefType;

	class FRenderState : public FLightCacheInterface
	{
	public:
		// 99
		struct Initializer
		{
			FString Name;
			FIntPoint Size{ EForceInit::ForceInitToZero };
			int32 MaxLevel = -1;
			// FLightmapResourceCluster* ResourceCluster = nullptr;
			FVector4 CoordinateScaleBias{ EForceInit::ForceInitToZero };

			bool IsValid()
			{
				// return Size.X > 0 && Size.Y > 0 && MaxLevel >= 0 && ResourceCluster != nullptr;
				return Size.X > 0 && Size.Y > 0 && MaxLevel >= 0;
			}
		};

		// 113
		FRenderState(Initializer InInitializer, FGeometryInstanceRenderStateRef GeometryInstanceRef);


		struct FTileState
		{
			int32 Revision = -1;
			int32 RenderPassIndex = 0;
			int32 CPURevision = -1;
			int32 OngoingReadbackRevision = -1;

			void Invalidate()
			{
				Revision = -1;
				RenderPassIndex = 0;
				InvalidateCPUData();
			}

			void InvalidateCPUData()
			{
				CPURevision = -1;
				OngoingReadbackRevision = -1;
			}
		};

		// 118
		FIntPoint GetPaddedSizeInTiles() const
		{
			return FIntPoint(
				FMath::DivideAndRoundUp(Size.X, GPreviewVirtualTileSize),
				FMath::DivideAndRoundUp(Size.Y, GPreviewVirtualTileSize));
		}

		// 174
		FTileState& RetrieveTileState(FTileVirtualCoordinates Coords)
		{
			check(Coords.MipLevel <= MaxLevel);

			int32 MipOffset = 0;
			for (int32 MipLevel = 0; MipLevel < Coords.MipLevel; MipLevel++)
			{
				MipOffset += FMath::DivideAndRoundUp(GetPaddedSizeInTiles().X, 1 << MipLevel) * FMath::DivideAndRoundUp(GetPaddedSizeInTiles().Y, 1 << MipLevel);
			}

			int32 LinearIndex = MipOffset + Coords.Position.Y * FMath::DivideAndRoundUp(GetPaddedSizeInTiles().X, 1 << Coords.MipLevel) + Coords.Position.X;
			return TileStates[LinearIndex];
		}

		// 227
		bool DoesTileHaveValidCPUData(FTileVirtualCoordinates Coords, int32 CurrentRevision);
	
	// 295
	private:
		FIntPoint Size;
		int32 MaxLevel;
		TArray<FTileState> TileStates;
	};
}