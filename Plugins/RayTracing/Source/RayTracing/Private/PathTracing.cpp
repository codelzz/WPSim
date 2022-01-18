#include "PathTracing.h"

IMPLEMENT_GLOBAL_SHADER(FPathTracingRGS, TEXT("/Plugin/RayTracing/Private/PathTracing.usf"), TEXT("PathTracingMainRG"), SF_RayGen);
