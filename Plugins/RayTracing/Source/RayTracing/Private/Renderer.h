#pragma once

#include "VirtualTexturing.h"

namespace RayTracing
{
class FSceneRenderState;

class FRenderer : public IVirtualTextureFinalizer
{
public:
	FRenderer(FSceneRenderState* InScene);

	virtual void Finalize(FRHICommandListImmediate& RHICmdList) override;

	virtual ~FRenderer();

private:
	FSceneRenderState* Scene;

};

}