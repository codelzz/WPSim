#include "Common.h"
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

	/**
	 * Finalize renderer
	 *		for each tile performance PathTracing
	 */
	// 887
	void FRenderer::Finalize(FRHICommandListImmediate& RHICmdList)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FRenderer::Finalize)

		// 891-894
		if (PendingTileRequests.Num() == 0)
		{
				return;
		}
		
		FMemMark Mark(FMemStack::Get()); // top-of-stack position in the memory stack

		// [Why?] Upload & copy coverged tiles direcly
		// 899
		{
			TArray<FTileRequest> TileUploadRquests = PendingTileRequests.FilterByPredicate(
				[CurrentRevision = CurrentRevision](const FTileRequest& Tile)
			{
					Tile.RenderState
				return Tile.RenderState->DoesTileHaveValidCPUData(Tile.VirtualCoordinates, CurrentRevision) || (Tile.RenderState->RetrieveTileState(Tile.VirtualCoordinates).OngoingReadbackRevision == CurrentRevision);
			});
		}



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
							
							RayTracingResolution.X = GPreviewPhysicalTileSize * GPUBatchedTileRequests.BatchedTilesDesc.Num();
							RayTracingResolution.Y = GPreviewPhysicalTileSize;
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
										RDG_EVENT_NAME("PathTracing %dx%d", RayTracingResolution.X, RayTracingResolution.Y),
										PassParameters,
										ERDGPassFlags::Compute,
										[PassParameters,
										this,
										RayTracingScene = Scene->RayTracingScene,
										RayTracingPipelineState = Scene->RayTracingPipelineState,
										RayGenerationShader,
										RayTracingResolution,
										GPUIndex](FRHICommandList& RHICmdList)
									{
										FRayTracingShaderBindingsWriter GlobalResources;
										SetShaderParameters(GlobalResources, RayGenerationShader, *PassParameters);
										
										check(RHICmdList.GetGPUMask().HasSingleIndex());

										RHICmdList.RayTraceDispatch(
											RayTracingPipelineState,
											RayGenerationShader.GetRayTracingShader(),
											RayTracingScene,
											GlobalResources,
											RayTracingResolution.X,
											RayTracingResolution.Y);
									});
								}
							}
							GraphBuilder.Execute();
						}
					}
				}
			}
		}

	}
}
