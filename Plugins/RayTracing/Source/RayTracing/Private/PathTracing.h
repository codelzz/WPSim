#include "ShaderParameterStruct.h"
#include "GlobalShader.h"


/**
 *  Path Tracing Ray Generation Shader
 * 
 * ref: LightmapRayTracing.h FLightmapPathTracingRGS
 */
class FPathTracingRGS : public FGlobalShader
{
	/* ����ȫ����ɫ�� */
	DECLARE_GLOBAL_SHADER(FPathTracingRGS);
	/* ʹ�� Root �����ṹ */
	SHADER_USE_ROOT_PARAMETER_STRUCT(FPathTracingRGS, FGlobalShader);

	/* �ж��Ƿ����Permutation */
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		// [TODO] support to check whether system support Ray Tracing Shader.
		// Ref: RayTracingDebug.cpp
		return true;
	}

	/* �޸ı��뻷�� */
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.CompilerFlags.Add(CFLAG_ForceDXC);
	}

	/* ��������ṹ */
	// ref: LightmapRayTraching.h
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		// SHADER_PARAMETER(RayTracingaccelerationstructure, TLAS)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutputSurface)
	END_SHADER_PARAMETER_STRUCT()

};