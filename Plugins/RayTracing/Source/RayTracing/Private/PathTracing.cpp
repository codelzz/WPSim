#include "PathTracing.h"

IMPLEMENT_GLOBAL_SHADER(FPathTracingRGS, "/Plugin/RayTracing/Private/PathTracing.usf", "PathTracingMainRG", SF_RayGen);
