#pragma once

#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GraphicToolsBlueprintFunctionLib.generated.h"

class UTextureRenderTarget2D;

UCLASS(MinimalAPI, meta=(ScriptName = "GraphicTools"))
class UGraphicToolsBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UGraphicToolsBlueprintLibrary(){}

	UFUNCTION(BlueprintCallable, Category = "SLSGraphicTools", meta = (WorldContext = "WorldContextObject"))
		static void DrawCheckerBoard(
			const UObject* WorldContextObject,
			class UTextureRenderTarget2D* OutputRenderTarget
		);

};


