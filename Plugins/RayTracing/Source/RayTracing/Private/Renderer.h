#pragma once

#include "VirtualTexturing.h"
#include "Scene/GeometryInterface.h"

namespace RayTracing
{
class FSceneRenderState;

struct FTileRequest
{
	FRenderStateRef RenderState;
	FTileVirtualCoordinates VirtualCoordinates;

	FTileRequest(
		FRenderStateRef RenderState,
		FTileVirtualCoordinates VirtualCoordinates)
		: RenderState(RenderState)
		, VirtualCoordinates(VirtualCoordinates)
	{}

	~FTileRequest(){}

	bool operator==(const FTileRequest& Rhs) const
	{
		return RenderState == Rhs.RenderState && VirtualCoordinates == Rhs.VirtualCoordinates;
	}
};

class FRenderer : public IVirtualTextureFinalizer
{
public:
	FRenderer(FSceneRenderState* InScene);

	virtual void Finalize(FRHICommandListImmediate& RHICmdList) override;

	virtual ~FRenderer();

private:
	int32 CurrentRevision = 0; // the revision at the moment

	FSceneRenderState* Scene;

	TArray<FTileRequest> PendingTileRequests;

};

}