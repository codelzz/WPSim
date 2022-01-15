
#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Components/MeshComponent.h"
#include "RayRender.generated.h"

UCLASS()
class RAYTRACING_API ARayRender : public AActor
{
	GENERATED_BODY()
public:
	ARayRender() {}

	UFUNCTION(BlueprintCallable, Category = "RayRender")
		void MainRayRender();


};
