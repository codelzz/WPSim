#pragma once

namespace RayTracing
{
	class FSceneRenderState
	{
	public:

		FRayTracingSceneRHIRef RayTracingScene;
		FRayTracingPipelineState* RayTracingPipelineState;

		void SetupRayTracingScene(int32 LODIndex = 0);
		void DestroyRayTracingScene();
	};
}