#pragma once

#include "Core/Private/HAL/Allocators/AnsiAllocator.h"

namespace RayTracing
{

	// 13
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

		~FLightmapTileRequest(){}
	
		// 34
		bool operator==(const FLightmapTileRequest& Rhs) const
		{
			return RenderState == Rhs.RenderState && VirtualCoordinates == Rhs.VirtualCoordinates;
		}
	};

	class FLightmapRenderer : public IVirtualTextureFinalizer
	{
	public:
		FLightmapRenderer(FSceneRenderState* InScene);

		virtual void Finalize(FRHICommandListImmediate& RHICmdList) override;

		virtual ~FLightmapRenderer();

	private:
		int32 CurrentRevision = 0; // the revision at the moment

		FSceneRenderState* Scene;

		TArray<FLightmapTileRequest> PendingTileRequests;

	};
}