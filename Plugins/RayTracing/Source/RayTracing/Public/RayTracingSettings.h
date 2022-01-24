// Ref: GPULightmassSettings.h remove bake, denoise

#include "GameFramework/Info.h"
#include "Subsystems/WorldSubsystem.h"
#include "RayTracingSettings.generated.h"

// 25
UCLASS(BlueprintType)
class RAYTRACING_API URayTracingSettings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = General)
		bool bShowProgressBars = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = General)
		bool bCompressLightmaps = true;

	// Total number of ray paths executed per texel across all bounces.
	// Set this to the lowest value that gives artifact-free results with the denoiser.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GlobalIllumination, DisplayName = "GI Samples", meta = (ClampMin = "32", ClampMax = "65536", UIMax = "8192"))
		int32 GISamples = 512;

	// Number of samples for stationary shadows, which are calculated and stored separately from GI.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = GlobalIllumination, meta = (ClampMin = "32", ClampMax = "65536", UIMax = "8192"))
		int32 StationaryLightShadowSamples = 128;

	// Baking speed multiplier when Realtime is enabled in the viewer.
	// Setting this too high can cause the editor to become unresponsive with heavy scenes.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = System, DisplayName = "Realtime Workload Factor", meta = (ClampMin = "1", ClampMax = "64"))
		int32 TilePassesInSlowMode = 1;

	// Baking speed multiplier when Realtime is disabled in the viewer.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = System, DisplayName = "Non-Realtime Workload Factor", meta = (ClampMin = "1", ClampMax = "64"))
		int32 TilePassesInFullSpeedMode = 8;

	// GPU RayTracing manages a pool for calculations of visible tiles. The pool size should be set based on the size of the
	// viewport and how many tiles will be visible on screen at once. Increasing this number increases GPU memory usage.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = System, meta = (ClampMin = "16", ClampMax = "128"))
		int32 LightmapTilePoolSize = 32;
};

// 129
UCLASS(NotPlaceable)
class RAYTRACING_API ARayTracingSettingsActor : public AInfo
{
	GENERATED_UCLASS_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Settings)
		URayTracingSettings* Settings;
};
