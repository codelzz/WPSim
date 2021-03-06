// ref: LightmapRenderer.cpp 
// remove denoise logic
// remove bake what you see
// remove first bounce

#pragma once

#include "TilePool.h"
#include "RHIGPUReadback.h"
// TBB suffers from extreme fragmentation problem in editor
#include "Core/Private/HAL/Allocators/AnsiAllocator.h"

namespace RayTracing
{
	class FSceneRenderState;

	struct FLightmapTileRequest
	{
		FLightmapRenderStateRef RenderState;
		FTileVirtualCoordinates VirtualCoordinates;

		FLightmapTileRequest(
			FLightmapRenderStateRef RenderState,
			FTileVirtualCoordinates VirtualCoordinates)
			: RenderState(RenderState)
			, VirtualCoordinates(VirtualCoordinates)
		{}

		~FLightmapTileRequest() {} // this deletes the move constructor while keeps copy constructors

		FIntPoint OutputPhysicalCoordinates[8];
		IPooledRenderTarget* OutputRenderTargets[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
		uint32 TileAddressInWorkingSet = ~0u;
		uint32 TileAddressInScratch = ~0u;

		bool operator==(const FLightmapTileRequest& Rhs) const
		{
			return RenderState == Rhs.RenderState && VirtualCoordinates == Rhs.VirtualCoordinates;
		}

		bool IsScreenOutputTile() const
		{
			for (IPooledRenderTarget* RenderTarget : OutputRenderTargets)
			{
				if (RenderTarget != nullptr)
				{
					return true;
				}
			}

			return false;
		}
	};

	struct FLightmapReadbackGroup
	{
		bool bIsFree = true;
		int32 Revision;
		int32 GPUIndex = 0;
		TUniquePtr<FLightmapTilePoolGPU> ReadbackTilePoolGPU;
		TArray<FLightmapTileRequest> ConvergedTileRequests;
		TUniquePtr<FRHIGPUTextureReadback> StagingHQLayer0Readback;
		TUniquePtr<FRHIGPUTextureReadback> StagingHQLayer1Readback;
		TUniquePtr<FRHIGPUTextureReadback> StagingShadowMaskReadback;

		struct FTextureData
		{
			// Duplicate some metadata so the async thread doesn't need to access FLightmapReadbackGroup
			FIntPoint SizeInTiles;
			int32 RowPitchInPixels[3];
			TArray<FLinearColor, FAnsiAllocator> Texture[3];

			volatile int32 bDenoisingFinished = 0;
		};

		TUniquePtr<FTextureData> TextureData;
	};
	
	/*
	struct FLightmapTileDenoiseGroup
	{
		FLightmapTileDenoiseGroup(FLightmapTileRequest& TileRequest) : TileRequest(TileRequest) {}

		int32 Revision;
		FLightmapTileRequest TileRequest;
		bool bShouldBeCancelled = false;

		struct FTextureData
		{
			TArray<FLinearColor, FAnsiAllocator> Texture[2];

			volatile int32 bDenoisingFinished = 0;
		};

		TSharedPtr<FTextureData, ESPMode::ThreadSafe> TextureData;
		class FLightmapTileDenoiseAsyncTask* AsyncDenoisingWork = nullptr;
	};
	*/
	/*
	class FLightmapTileDenoiseAsyncTask : public IQueuedWork
	{
	public:
		FIntPoint Size;
		TSharedPtr<FLightmapTileDenoiseGroup::FTextureData, ESPMode::ThreadSafe> TextureData;

		virtual void DoThreadedWork();
		virtual void Abandon() {}
	};
	*/

	class FLightmapRenderer : public IVirtualTextureFinalizer
	{
	public:
		FLightmapRenderer(FSceneRenderState* InScene);

		void AddRequest(FLightmapTileRequest TileRequest);

		virtual void Finalize(FRHICommandListImmediate& RHICmdList) override;

		virtual ~FLightmapRenderer();

		void BackgroundTick();

		void BumpRevision();
		int32 GetCurrentRevision() { return CurrentRevision; }

		int32 FrameNumber = 0;

		// bool bUseFirstBounceRayGuiding = false;
		// int32 NumFirstBounceRayGuidingTrialSamples = 0;

		// bool bDenoiseDuringInteractiveBake = false;
		// bool bOnlyBakeWhatYouSee = false;

		TArray<TArray<FLightmapTileRequest>> TilesVisibleLastFewFrames;

		bool bIsRecordingTileRequests = false;
		TArray<FLightmapTileRequest> RecordedTileRequests;
		void DeduplicateRecordedTileRequests();

	private:
		int32 CurrentRevision = 0;
		int32 LastInvalidationFrame = 0;
		int32 Mip0WorkDoneLastFrame = 0;
		bool bIsExiting = false;
		bool bInsideBackgroundTick = false;
		bool bWasRunningAtFullSpeed = true;

		FSceneRenderState* Scene;

		TArray<FLightmapTileRequest> PendingTileRequests;

		FLightmapTilePoolGPU LightmapTilePoolGPU;

		TUniquePtr<FLightmapTilePoolGPU> ScratchTilePoolGPU;

		TArray<FLightmapReadbackGroup*> OngoingReadbacks;
		TArray<TUniquePtr<FLightmapReadbackGroup>> RecycledReadbacks;

		// TArray<FLightmapTileDenoiseGroup> OngoingDenoiseGroups;

		TUniquePtr<FLightmapTilePoolGPU> UploadTilePoolGPU;

		// FQueuedThreadPool* DenoisingThreadPool;

		// void RenderIrradianceCacheVisualization(FPostOpaqueRenderParameters& Parameters);
		// FDelegateHandle IrradianceCacheVisualizationDelegateHandle;
	};
}