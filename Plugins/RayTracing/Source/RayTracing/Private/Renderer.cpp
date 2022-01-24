// Ref: LightmapRenderer.cpp remove:
// 1. denoise
// 2. bake what you see
// 3. FirstBounceRayGuidingTrialSamples

#include "Renderer.h"
#include "RayTracingModule.h"
#include "Common.h"
#include "PathTracing.h"
#include "Scene/Scene.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "Storage.h"
#include "ShaderParameterStruct.h"

namespace RayTracing
{
	// 146
	struct FGPUTileDescription
	{
		FIntPoint LightmapSize;
		FIntPoint VirtualTilePosition;
		FIntPoint WorkingSetPosition;
		FIntPoint ScratchPosition;
		FIntPoint OutputLayer0Position;
		FIntPoint OutputLayer1Position;
		FIntPoint OutputLayer2Position;
		int32 FrameIndex;
		int32 RenderPassIndex;
	};

	// 159
	struct FGPUBatchedTileRequests
	{
		FStructuredBufferRHIRef BatchedTilesBuffer;
		FShaderResourceViewRHIRef BatchedTilesSRV;
		TResourceArray<FGPUTileDescription> BatchedTilesDesc;
	};

	// 169
	FLightmapRenderer::FLightmapRenderer(FSceneRenderState* InScene)
		: Scene(InScene)
		, LightmapTilePoolGPU(FIntPoint(Scene->Settings->LightmapTilePoolSize))
	{
		const int32 PhysicalTileSize = GPreviewPhysicalTileSize;
		LightmapTilePoolGPU.Initialize(
		{
			{ PF_A32B32G32R32F, FIntPoint(PhysicalTileSize) }, // IrradianceAndSampleCount
			{ PF_A32B32G32R32F, FIntPoint(PhysicalTileSize) }, // SHDirectionality
			{ PF_A32B32G32R32F, FIntPoint(PhysicalTileSize) }, // ShadowMask
			{ PF_A32B32G32R32F, FIntPoint(PhysicalTileSize) }, // ShadowMaskSampleCount
			{ PF_A32B32G32R32F, FIntPoint(PhysicalTileSize) }, // SHCorrectionAndStationarySkyLightBentNormal
		});
	}

	// 221
	FLightmapRenderer::~FLightmapRenderer()
	{
	}

	// 228
	void FLightmapRenderer::AddRequest(FLightmapTileRequest TileRequest)
	{
		PendingTileRequests.AddUnique(TileRequest);
	}

	// 488
	void FSceneRenderState::SetupRayTracingScene(int32 LODIndex)
	{

	}

	/**
	 * Finalize renderer
	 *		for each tile performance PathTracing
	 */
	// 887
	void FLightmapRenderer::Finalize(FRHICommandListImmediate& RHICmdList)
	{
		TRACE_CPUPROFILER_EVENT_SCOPE(FRenderer::Finalize)

		const int32 VirtualTileSize = GPreviewVirtualTileSize;
		const int32 PhysicalTileSize = GPreviewPhysicalTileSize;

		// 891-894
		// [SKIP] If no pending tile requests exist
		if (PendingTileRequests.Num() == 0) { return; }
		
		// 896
		FMemMark Mark(FMemStack::Get()); // top-of-stack position in the memory stack

		// 899

		// 1410
		if (PendingTileRequests.Num() == 0) { return; }

		// 1417
		// what is it?
		int32 MostCommonLODIndex = 0;

		// 1419
		int32 NumRequestsPerLOD[MAX_STATIC_MESH_LODS] = { 0 };

		// 1421
		for (const FLightmapTileRequest& Tile : PendingTileRequests)
		{
			NumRequestsPerLOD[Tile.RenderState->GeometryInstanceRef.LODIndex]++;
		}

		// 1426
		if (bInsideBackgroundTick)
		{
			// Pick the most common LOD level
			for (int32 Index = 0; Index < MAX_STATIC_MESH_LODS; Index++)
			{
				if (NumRequestsPerLOD[Index] > NumRequestsPerLOD[MostCommonLODIndex])
				{
					MostCommonLODIndex = Index;
				}
			}
		}
		else
		{
			// Alternate between LOD levels when in realtime preview
			TArray<int32> NonZeroLODIndices;

			for (int32 Index = 0; Index < MAX_STATIC_MESH_LODS; Index++)
			{
				if (NumRequestsPerLOD[Index] > 0)
				{
					NonZeroLODIndices.Add(Index);
				}
			}

			check(NonZeroLODIndices.Num() > 0);

			MostCommonLODIndex = NonZeroLODIndices[FrameNumber % NonZeroLODIndices.Num()];
		}

		// 1455
		Scene->SetupRayTracingScene(MostCommonLODIndex);

		// 1565
		// Get number of sample per frame. If function in background tick then use full 
		// speed otherwise slow speed
		int32 NumSamplesPerFrame = (bInsideBackgroundTick) ? 
			Scene->Settings->TilePassesInFullSpeedMode :
			Scene->Settings->TilePassesInSlowMode;

		// 1567
		{
			// Pending Tile Requests which are waiting for processing
			// Filter condition:
			//	1. Tile not converged
			//  2. GeometryInstanceRef.LODIndex == MostCommonLODIndex
			TArray<FLightmapTileRequest> PendingGITileRequests = PendingTileRequests.FilterByPredicate(
				[NumGISamples = Scene->Settings->GISamples, MostCommonLODIndex](const FLightmapTileRequest& Tile) {
				return !Tile.RenderState->IsTileGIConverged(Tile.VirtualCoordinates, NumGISamples) 
					&& Tile.RenderState->GeometryInstanceRef.LODIndex == MostCommonLODIndex;
			});

			// 1572
			// RenderGI
			for (int32 SampleIndex = 0; SampleIndex < NumSamplesPerFrame; SampleIndex++)
			{
				// FMemMark marks a top - of - stack position in the memory stack.
				// When the marker is constructed or initialized with a particular 
				// memory stack, it saves the stack's current position. 
				// When marker is popped, it pops all items that were added to the 
				// stack subsequent to initialization.
				FMemMark PerSampleMark(FMemStack::Get());
				{
					if (PendingGITileRequests.Num() > 0)
					{
						// 1681
						if (IsRayTracingEnabled())
						{
							for (uint32 GPUIndex = 0; GPUIndex < GNumExplicitGPUsForRendering; GPUIndex++)
							{
								FGPUBatchedTileRequests GPUBatchedTileRequests;

								for (const FLightmapTileRequest& Tile : PendingGITileRequests)
								{
									uint32 AssignedGPUIndex = (Tile.RenderState->DistributionPrefixSum + Tile.RenderState->RetrieveTileStateIndex(Tile.VirtualCoordinates)) % GNumExplicitGPUsForRendering;
									if (AssignedGPUIndex != GPUIndex) continue;

									FGPUTileDescription TileDesc;
									TileDesc.LightmapSize = Tile.RenderState->GetSize();
									TileDesc.VirtualTilePosition = Tile.VirtualCoordinates.Position * VirtualTileSize;
									TileDesc.WorkingSetPosition = LightmapTilePoolGPU.GetPositionFromLinearAddress(Tile.TileAddressInWorkingSet) * PhysicalTileSize;
									TileDesc.ScratchPosition = ScratchTilePoolGPU->GetPositionFromLinearAddress(Tile.TileAddressInScratch) * PhysicalTileSize;
									TileDesc.OutputLayer0Position = Tile.OutputPhysicalCoordinates[0] * PhysicalTileSize;
									TileDesc.OutputLayer1Position = Tile.OutputPhysicalCoordinates[1] * PhysicalTileSize;
									TileDesc.OutputLayer2Position = Tile.OutputPhysicalCoordinates[2] * PhysicalTileSize;
									TileDesc.FrameIndex = Tile.RenderState->RetrieveTileState(Tile.VirtualCoordinates).Revision;
									TileDesc.RenderPassIndex = Tile.RenderState->RetrieveTileState(Tile.VirtualCoordinates).RenderPassIndex;
									if (!Tile.RenderState->IsTileGIConverged(Tile.VirtualCoordinates, Scene->Settings->GISamples))
									{
										Tile.RenderState->RetrieveTileState(Tile.VirtualCoordinates).RenderPassIndex++;

										if (/*Tile.VirtualCoordinates.MipLevel == 0 && */SampleIndex == 0)
										{
											if (!bInsideBackgroundTick)
											{
												Mip0WorkDoneLastFrame++;
											}
										}

										GPUBatchedTileRequests.BatchedTilesDesc.Add(TileDesc);
									}
								}

								if (GPUBatchedTileRequests.BatchedTilesDesc.Num() > 0)
								{
									FRHIResourceCreateInfo CreateInfo;
									CreateInfo.GPUMask = FRHIGPUMask::FromIndex(GPUIndex);
									CreateInfo.ResourceArray = &GPUBatchedTileRequests.BatchedTilesDesc;

									GPUBatchedTileRequests.BatchedTilesBuffer = RHICreateStructuredBuffer(sizeof(FGPUTileDescription), GPUBatchedTileRequests.BatchedTilesDesc.GetResourceDataSize(), BUF_Dynamic | BUF_ShaderResource, CreateInfo);
									GPUBatchedTileRequests.BatchedTilesSRV = RHICreateShaderResourceView(GPUBatchedTileRequests.BatchedTilesBuffer);
								}

								SCOPED_GPU_MASK(RHICmdList, FRHIGPUMask::FromIndex(GPUIndex));

								if (GPUBatchedTileRequests.BatchedTilesDesc.Num() > 0)
								{
									// `721
									FRDGBuilder GraphBuilder(RHICmdList);

									// custom
									FRDGTextureRef OutputSurface = GraphBuilder.RegisterExternalTexture(LightmapTilePoolGPU.PooledRenderTargets[0], TEXT("OutputSurface"));

									// 1752
									FIntPoint RayTracingResolution;
									RayTracingResolution.X = PhysicalTileSize * GPUBatchedTileRequests.BatchedTilesDesc.Num();
									RayTracingResolution.Y = PhysicalTileSize;

									// Path Tracing GI
									// 1757
									{
										{
											FPathTracingRGS::FParameters* PassParameters = GraphBuilder.AllocParameters<FPathTracingRGS::FParameters>();
											PassParameters->OutputSurface = GraphBuilder.CreateUAV(OutputSurface);

											FPathTracingRGS::FPermutationDomain PermutationVector;
											auto RayGenerationShader = GetGlobalShaderMap(GMaxRHIFeatureLevel)->GetShader<FPathTracingRGS>(PermutationVector);
											ClearUnusedGraphResources(RayGenerationShader, PassParameters);

											GraphBuilder.AddPass(
												RDG_EVENT_NAME("LightmapPathTracing %dx%d", RayTracingResolution.X, RayTracingResolution.Y),
												PassParameters,
												ERDGPassFlags::Compute,
												[PassParameters, this, RayTracingScene = Scene->RayTracingScene, PipelineState = Scene->RayTracingPipelineState, RayGenerationShader, RayTracingResolution, GPUIndex](FRHICommandList& RHICmdList)
											{
												FRayTracingShaderBindingsWriter GlobalResources;
												SetShaderParameters(GlobalResources, RayGenerationShader, *PassParameters);

												check(RHICmdList.GetGPUMask().HasSingleIndex());

												RHICmdList.RayTraceDispatch(PipelineState, RayGenerationShader.GetRayTracingShader(), RayTracingScene, GlobalResources, RayTracingResolution.X, RayTracingResolution.Y);
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
	}
}
