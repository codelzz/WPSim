#include "GraphicToolsBlueprintFunctionLib.h"

#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"
#include "Runtime/Engine/Classes/Engine/World.h"
#include "GlobalShader.h"
#include "PipelineStateCache.h"
#include "RHIStaticStates.h"
#include "SceneUtils.h"
#include "SceneInterface.h"
#include "ShaderParameterUtils.h"
#include "Logging/MessageLog.h"
#include "Internationalization/Internationalization.h"
#include "Runtime/RenderCore/Public/RenderTargetPool.h"


#define LOCTEXT_NAMESPACE "GraphicToolsPlugin"

class FCheckerBoardComputeShader : public FGlobalShader
{
	DECLARE_SHADER_TYPE(FCheckerBoardComputeShader, Global)

public:
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{

	}

	FCheckerBoardComputeShader() {}
	FCheckerBoardComputeShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		OutputSurface.Bind(Initializer.ParameterMap, TEXT("OutputSurface"));
	}

	void SetParameters(
		FRHICommandList& RHICmdList,
		FTexture2DRHIRef& InOutputSurfaceValue,
		FUnorderedAccessViewRHIRef& UAV
	)
	{
		FRHIComputeShader* ShaderRHI = RHICmdList.GetBoundComputeShader();
	
		RHICmdList.TransitionResource(ERHIAccess::ERWBarrier, EResourceTransitionPipeline::EComputeToCompute, UAV);
		// RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EComputeToCompute, UAV);
		OutputSurface.SetTexture(RHICmdList, ShaderRHI, InOutputSurfaceValue, UAV);
	}

	void UnsetParameters(FRHICommandList& RHICmdList, FUnorderedAccessViewRHIRef& UAV)
	{
		RHICmdList.TransitionResource(ERHIAccess::EReadable, EResourceTransitionPipeline::EComputeToCompute, UAV);
	//	RHICmdList.TransitionResource(EResourceTransitionAccess::EReadable, EResourceTransitionPipeline::EComputeToCompute, UAV);
		OutputSurface.UnsetUAV(RHICmdList, RHICmdList.GetBoundComputeShader());
	}

private:

	LAYOUT_FIELD(FRWShaderParameter, OutputSurface);
};
IMPLEMENT_SHADER_TYPE(, FCheckerBoardComputeShader, TEXT("/Plugin/RayTracing/Private/CheckerBoard.usf"), TEXT("MainCS"), SF_Compute);

static void DrawCheckerBoard_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	FTextureRenderTargetResource* TextureRenderTargetResource,
	ERHIFeatureLevel::Type FeatureLevel
)
{
	check(IsInRenderingThread());

	FTexture2DRHIRef RenderTargetTexture = TextureRenderTargetResource->GetRenderTargetTexture();
	uint32 SizeX = RenderTargetTexture->GetSizeX();
	uint32 SizeY = RenderTargetTexture->GetSizeY();

	uint32 GGroupSize = 32;
	FIntPoint FullResolution = FIntPoint(SizeX, SizeY);
	uint32 GroupSizeX = FMath::DivideAndRoundUp(SizeX, GGroupSize);
	uint32 GroupSizeY = FMath::DivideAndRoundUp(SizeY, GGroupSize);

	TShaderMapRef<FCheckerBoardComputeShader>ComputeShader(GetGlobalShaderMap(FeatureLevel));
	RHICmdList.SetComputeShader(ComputeShader.GetComputeShader());


	FRHIResourceCreateInfo CreateInfo;
	//Create a temp resource
	FTexture2DRHIRef GSurfaceTexture2D = RHICreateTexture2D(SizeX, SizeY, PF_FloatRGBA, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
	FUnorderedAccessViewRHIRef GUAV = RHICreateUnorderedAccessView(GSurfaceTexture2D);

	ComputeShader->SetParameters(RHICmdList, RenderTargetTexture, GUAV);
	DispatchComputeShader(RHICmdList, ComputeShader, GroupSizeX, GroupSizeY, 1);
	ComputeShader->UnsetParameters(RHICmdList, GUAV);

	FRHICopyTextureInfo CopyInfo;
	RHICmdList.CopyTexture(GSurfaceTexture2D, RenderTargetTexture, CopyInfo);

	/*
	TArray<FVector4> Bitmap;
	Bitmap.Init(FVector4(1.0f, 0.0f, 0.0f, 1.0f), SizeX * SizeY);
	uint32 LolStride = 0;
	uint8* TextureDataPtr = (uint8*)RHILockTexture2D(RenderTargetTexture, 0, EResourceLockMode::RLM_ReadOnly, LolStride, false);
	if (TextureDataPtr != nullptr) {
		uint8* ArrayData = (uint8*)Bitmap.GetData();
		FMemory::Memcpy(ArrayData, TextureDataPtr, GPixelFormats[PF_A32B32G32R32F].BlockBytes * SizeX * SizeY);
	}
	RHICmdList.UnlockTexture2D(RenderTargetTexture, 0, false);
	*/

	TArray<FVector4> Bitmap;
	Bitmap.Init(FVector4(1.0f, 0.0f, 0.0f, 1.0f), SizeX * SizeY);
	// for unknow reason TexCreate_UAV is not allow
	FRHIResourceCreateInfo CreateInfoX;
	FTexture2DRHIRef Texture2D = RHICreateTexture2D(SizeX, SizeY, PF_FloatRGBA, 1, 1, TexCreate_ShaderResource, CreateInfoX);
	RHICmdList.CopyTexture(RenderTargetTexture, Texture2D, CopyInfo);
	uint32 DestStride;
	uint8* DestBuffer = (uint8*)RHILockTexture2D(Texture2D, 0, RLM_ReadOnly, DestStride, false);
	if (DestBuffer != nullptr) {
		uint8* ArrayData = (uint8*)Bitmap.GetData();
		FMemory::Memcpy(ArrayData, DestBuffer, GPixelFormats[PF_A32B32G32R32F].BlockBytes * SizeX * SizeY);
	}
	RHIUnlockTexture2D(Texture2D, 0, false);

}

void UGraphicToolsBlueprintLibrary::DrawCheckerBoard(const UObject* WorldContextObject, UTextureRenderTarget2D* OutputRenderTarget)
{
	check(IsInGameThread());

	if (!OutputRenderTarget)
	{
		FMessageLog("Blueprint").Warning(
				LOCTEXT("UGraphicToolsBlueprintLibrary::DrawCheckerBoard", "DrawUVDisplacementToRenderTarget: Output render target is required."));
		return;
	}

	FTextureRenderTargetResource* TextureRenderTargetResource = OutputRenderTarget->GameThread_GetRenderTargetResource();
	ERHIFeatureLevel::Type FeatureLevel = WorldContextObject->GetWorld()->Scene->GetFeatureLevel();

	ENQUEUE_RENDER_COMMAND(CaptureCommand)
		(
			[TextureRenderTargetResource, FeatureLevel](FRHICommandListImmediate& RHICmdList)
			{
				DrawCheckerBoard_RenderThread
				(
					RHICmdList,
					TextureRenderTargetResource,
					FeatureLevel
				);
			}
	);
}

#undef LOCTEXT_NAMESPACE

