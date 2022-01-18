#include "ShaderParameterStruct.h"
#include "GlobalShader.h"


/**
 *  Path Tracing Ray Generation Shader
 * 
 * ref: LightmapRayTracing.h FLightmapPathTracingRGS
 */
class FPathTracingRGS : public FGlobalShader
{
	/* 声明全局着色器 */
	DECLARE_GLOBAL_SHADER(FPathTracingRGS);
	/* 使用 Root 参数结构 */
	SHADER_USE_ROOT_PARAMETER_STRUCT(FPathTracingRGS, FGlobalShader);

	/* 判断是否编译Permutation */
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		// [TODO] support to check whether system support Ray Tracing Shader.
		// Ref: RayTracingDebug.cpp
		return true;
	}

	/* 修改编译环境 */
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.CompilerFlags.Add(CFLAG_ForceDXC);
	}

	/* 定义参数结构 */
	// ref: LightmapRayTraching.h
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		// SHADER_PARAMETER(RayTracingaccelerationstructure, TLAS)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutputSurface)
	END_SHADER_PARAMETER_STRUCT()

};