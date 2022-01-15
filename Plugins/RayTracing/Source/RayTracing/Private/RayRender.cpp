#include "RayRender.h"
#include "ShaderCompilerCore.h"

// 计算全局着色器 - 负责主要计算
class FRayComputeShader : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FRayComputeShader);

public:
	FRayComputeShader(){}
	FRayComputeShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		OutputSurface.Bind(Initializer.ParameterMap, TEXT("OutputSurface"));
	}

	static bool ShouldCache(EShaderPlatform Platform)
	{
		return IsFeatureLevelSupported(Platform, ERHIFeatureLevel::SM5);	
	}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Platform, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Platform, OutEnvironment);
		OutEnvironment.CompilerFlags.Add(ECompilerFlags::CFLAG_StandardOptimization);
	}

	void SetSurfaces(FRHICommandList& RHICmdList, FUnorderedAccessViewRHIRef OutputSurfaceUAV)
	{
		FRHIComputeShader* ComputeShaderRHI = RHICmdList.GetBoundComputeShader();
		if (OutputSurface.IsBound())
		{
			RHICmdList.SetUAVParameter(ComputeShaderRHI, OutputSurface.GetBaseIndex(), OutputSurfaceUAV);
		}
	}

	void UnbindBuffers(FRHICommandList& RHICmdList)
	{
		FRHIComputeShader* ComputeShaderRHI = RHICmdList.GetBoundComputeShader();
		if (OutputSurface.IsBound())
		{
			RHICmdList.SetUAVParameter(ComputeShaderRHI, OutputSurface.GetBaseIndex(), FUnorderedAccessViewRHIRef());
		}
	}

private:
	LAYOUT_FIELD(FShaderResourceParameter, OutputSurface);
};

IMPLEMENT_SHADER_TYPE(, FRayComputeShader, TEXT("/Plugin/RayTracing/Private/RayTracing.usf"), TEXT("MainCS"), SF_Compute);

// 函数末尾的SaveArrayToTexture(), 需要在RayTracing_renderThread 前实现
void SaveArrayToTexture(TArray<FVector4>* BitmapRef, uint32 TextureSizeX, uint32 TextureSizeY)
{
	TArray<FColor>BitMapToBeSave;

	for (int32 PixelIndex = 0; PixelIndex < BitmapRef->Num(); PixelIndex++)
	{
		uint8 cr = 255.99f * (*BitmapRef)[PixelIndex].X;
		uint8 cg = 255.99f * (*BitmapRef)[PixelIndex].Y;
		uint8 cb = 255.99f * (*BitmapRef)[PixelIndex].Z;
		uint8 ca = 255.99f * (*BitmapRef)[PixelIndex].W;
		BitMapToBeSave.Add(FColor(cr, cg, cb, ca));
	}

	// if the format and texture type is supported
	if (BitMapToBeSave.Num())
	{
		// Create screenshot folder of not already present.
		IFileManager::Get().MakeDirectory(*FPaths::ScreenShotDir(), true);

		const FString ScreenFileName(FPaths::ScreenShotDir() / TEXT("RenderOutputTexture"));

		uint32 ExtendXWithMSAA = BitMapToBeSave.Num() / TextureSizeY;

		// Save the contents of the array to a bitmap file. (24bit only so alpha channel is dropped)
		FFileHelper::CreateBitmap(*ScreenFileName, ExtendXWithMSAA, TextureSizeY, BitMapToBeSave.GetData());

		UE_LOG(LogConsoleResponse, Display, TEXT("Content was saved to \"%s\""), *FPaths::ScreenShotDir());
	}
	else
	{
		UE_LOG(LogConsoleResponse, Error, TEXT("Failed to save BMP, format or texture type is not supported"));
	}

}

// 被逻辑线程的 MainRayRender() 调用, 负责在渲染线程控制CS,
static void RayTracing_RenderThread(
	FRHICommandListImmediate& RHICmdList,
	ERHIFeatureLevel::Type FeatureLevel
)
{
	check(IsInRenderingThread());
	TArray<FVector4> Bitmap;

	FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
	TShaderMapRef<FRayComputeShader>ComputeShader(GlobalShaderMap);
	RHICmdList.SetComputeShader(ComputeShader.GetComputeShader());

	int32 SizeX = 256;
	int32 SizeY = 256;

	FRHIResourceCreateInfo CreateInfo;
	FTexture2DRHIRef Texture = RHICreateTexture2D(SizeX, SizeY, PF_A32B32G32R32F, 1, 1, TexCreate_ShaderResource | TexCreate_UAV, CreateInfo);
	FUnorderedAccessViewRHIRef TextureUAV = RHICreateUnorderedAccessView(Texture);
	ComputeShader->SetSurfaces(RHICmdList, TextureUAV);
	DispatchComputeShader(RHICmdList, ComputeShader, SizeX / 32, SizeY / 32, 1);
	ComputeShader->UnbindBuffers(RHICmdList);

	FRHIResourceCreateInfo CreateInfoCone;
	FTexture2DRHIRef TextureCone = RHICreateTexture2D(SizeX, SizeY, PF_FloatRGBA, 1, 1, TexCreate_ShaderResource, CreateInfoCone);

	FRHICopyTextureInfo CopyInfo;
	RHICmdList.CopyTexture(Texture, TextureCone, CopyInfo);


	Bitmap.Init(FVector4(1.0f, 0.0f, 0.0f, 1.0f), SizeX * SizeY);

	uint32 LolStride = 0;
	/* 
	 * Assertion failed: !bRequiresResourceStateTracking 
	 * [File:D:\Build\++UE4\Sync\Engine\Source\Runtime\D3D12RHI\Private\../Public/D3D12Resources.h] [Line: 219] 
	 */
	uint8* TextureDataPtr = (uint8*)RHILockTexture2D(TextureCone, 0, EResourceLockMode::RLM_ReadOnly, LolStride, false);
	if (TextureDataPtr != nullptr) {
		uint8* ArrayData = (uint8*)Bitmap.GetData();
		FMemory::Memcpy(ArrayData, TextureDataPtr, GPixelFormats[PF_A32B32G32R32F].BlockBytes * SizeX * SizeY);
	}
	RHICmdList.UnlockTexture2D(TextureCone, 0, false);
	SaveArrayToTexture(&Bitmap, SizeX, SizeY);

	Texture.SafeRelease();
	TextureUAV.SafeRelease();
	TextureCone.SafeRelease();
}

/* 渲染器主函数,加入光追渲染线程 */
void ARayRender::MainRayRender()
{
	ERHIFeatureLevel::Type FeatureLevel = GetWorld()->Scene->GetFeatureLevel();
	ENQUEUE_RENDER_COMMAND(RayTracingCommand)(
		[FeatureLevel](FRHICommandListImmediate& RHICmdList)
		{
			RayTracing_RenderThread(
				RHICmdList,
				FeatureLevel
			);
		}
	);
}