#pragma once

#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RayTracingBlueprintFunctionLib.generated.h"

class UTextureRenderTarget2D;

UCLASS(MinimalAPI, meta = (ScriptName = "RayTracing"))
class URayTracingBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	URayTracingBlueprintFunctionLibrary() {}

	UFUNCTION(BlueprintCallable, Category = "RayTracing", meta = (WorldContext = "WorldContextObject"))
		static void DrawRayTracingResult(
			const UObject* WorldContextObject,
			class UTextureRenderTarget2D* OutputRenderTarget
		);

};
