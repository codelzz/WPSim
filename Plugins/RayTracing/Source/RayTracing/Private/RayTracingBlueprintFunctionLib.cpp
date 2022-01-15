#include "RayTracingBlueprintFunctionLib.h"
#include "RayTracingModule.h"

#define LOCTEXT_NAMESPACE "RayTracingPlugin"

void URayTracingBlueprintFunctionLibrary::DrawRayTracingResult(const UObject* WorldContextObject, UTextureRenderTarget2D* OutputRenderTarget)
{
	check(IsInGameThread());

	UE_LOG(LogRayTracing, Log, TEXT("DrawRayTracingResult"));
}

#undef LOCTEXT_NAMESPACE