#include "Renderer.h"
#include "PathTracing.h"
#include "Scene/Scene.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"

namespace RayTracing
{
	FRenderer::FRenderer(FSceneRenderState* InScene)
		: Scene(InScene)
	{

	}

	FRenderer::~FRenderer()
	{
		// delete ThreadPool;
		// send delegate
	}

	void FSceneRenderState::SetupRayTracingScene(int32 LODIndex)
	{

	}

	// Ref: FLightmapRender::Finalize
	void FRenderer::Finalize(FRHICommandListImmediate& RHICmdList)
	{
		// Scene->SetupRayTracingScene()

		{
			// get Number of sample perframe

			// for loop sample
			{
				// if pending GI tile requests
				{
					// for loop scratch layer
					{}
					{}

					// for GNumExplicitGPUsForRendering
					{}

					if (IsRayTracingEnabled())
					{
						// for GNumExplicitGPUsForRendering
						{
							// for PendingGITileRequests

							// if GPU Batched Tile Request > 0
							FRDGBuilder GraphBuilder(RHICmdList);
							// Register Texture
							FRDGTextureRef OutputSurface = GraphBuilder.RegisterExternalTexture(TilePoolGPU.PooledRenderTargets[0], TEXT("OutputSurface"));

							FIntPoint RayTracingResolution;
							// RayTracingResolution.X = 

							// Path Tracing GI
							{
								{
									FPathTracingRGS::FParameters* PassParameters = GraphBuilder.AllocParameters<FPathTracingRGS::FParameters>();
									PassParameters->OutputSurface = GraphBuilder.CreateUAV(OutputSurface);

									// SetupPathTracingLightParameters
									FPathTracingRGS::FPermutationDomain PermutationVector;
									// set permutation if necessary
									auto RayGenerationShader = GetGlobalShaderMap(GMaxRHIFeatureLevel)->GetShader<FPathTracingRGS>(PermutationVector);
									ClearUnusedGraphResources(RayGenerationShader, PassParameters);

									GraphBuilder.AddPass(
										RDG_EVENT_NAME("PathTracing %dx%d", )
									)

								}
							}

						}
					}
				}
			}
		}

	}
}
