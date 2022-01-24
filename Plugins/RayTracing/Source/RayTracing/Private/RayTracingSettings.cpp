
#include "RayTracingSettings.h"

ARayTracingSettingsActor::ARayTracingSettingsActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.DoNotCreateDefaultSubobject(TEXT("Sprite")))
{
#if WITH_EDITORONLY_DATA
	bActorLabelEditable = false;
#endif // WITH_EDITORONLY_DATA
	bIsEditorOnlyActor = true;

	Settings = ObjectInitializer.CreateDefaultSubobject<URayTracingSettings>(this, TEXT("RayTracingSettings"));
}
