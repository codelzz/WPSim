// Copyright Epic Games, Inc. All Rights Reserved.

#include "RayTracingModule.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FRayTracingModule"

DEFINE_LOG_CATEGORY(LogRayTracing);

void FRayTracingModule::StartupModule()
{
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("RayTracing"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/RayTracing"), PluginShaderDir);
}

void FRayTracingModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FRayTracingModule, RayTracing)