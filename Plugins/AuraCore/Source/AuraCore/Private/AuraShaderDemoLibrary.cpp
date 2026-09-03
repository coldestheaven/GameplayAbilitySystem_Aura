// Copyright Druid Mechanics

#include "AuraShaderDemoLibrary.h"

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraph.h"
#include "RenderGraphUtils.h"
#include "RenderingThread.h"
#include "Engine/TextureRenderTarget2D.h"

// ───────── 全屏三角形顶点着色器（无参数，用 SV_VertexID 生成顶点） ─────────
class FAuraPatternVS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FAuraPatternVS);
public:
	FAuraPatternVS() = default;
	FAuraPatternVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};

// ───────── 奥术图案像素着色器（参数与 .usf 逐名对应） ─────────
class FAuraPatternPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FAuraPatternPS);
	SHADER_USE_PARAMETER_STRUCT(FAuraPatternPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FVector2f, TextureSize)
		SHADER_PARAMETER(float, Time)
		SHADER_PARAMETER(uint32, PatternType)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};

// 着色器注册：/Plugin/AuraCore 由插件 Shaders 目录自动映射
IMPLEMENT_GLOBAL_SHADER(FAuraPatternVS, "/Plugin/AuraCore/AuraShaderDemo.usf", "AuraPatternVS", SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FAuraPatternPS, "/Plugin/AuraCore/AuraShaderDemo.usf", "AuraPatternPS", SF_Pixel);

/**
 * 渲染线程实现：RDG 栅格通道，全屏三角形绘制图案到外部渲染目标
 * RDG 自动处理纹理状态转换（无需手写 Transition）
 */
static void RenderAuraPattern_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureRHIRef OutputTexture, FIntPoint TextureSize, float Time, int32 PatternType)
{
	FRDGBuilder GraphBuilder(RHICmdList);

	FRDGTextureRef OutputTextureRef = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(OutputTexture, TEXT("AuraPatternRT")));

	FAuraPatternPS::FParameters* PassParameters = GraphBuilder.AllocParameters<FAuraPatternPS::FParameters>();
	PassParameters->TextureSize = FVector2f(TextureSize.X, TextureSize.Y);
	PassParameters->Time = Time;
	PassParameters->PatternType = static_cast<uint32>(PatternType);
	PassParameters->RenderTargets[0] = FRenderTargetBinding(OutputTextureRef, ERenderTargetLoadAction::EClear);

	FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<FAuraPatternVS> VertexShader(ShaderMap);
	TShaderMapRef<FAuraPatternPS> PixelShader(ShaderMap);

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("AuraPattern %dx%d Type=%d", TextureSize.X, TextureSize.Y, PatternType),
		PassParameters,
		ERDGPassFlags::Raster,
		[VertexShader, PixelShader, PassParameters, TextureSize](FRHICommandList& RHICmdList)
		{
			// PSO：无混合 / 无深度测试 / 实线 / 三角形列表
			FGraphicsPipelineStateInitializer GraphicsPSO;
			GraphicsPSO.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSO.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
			GraphicsPSO.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
			GraphicsPSO.PrimitiveType = PT_TriangleList;
			GraphicsPSO.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSO.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			// UE5.8：自由函数版移除了默认参，需显式传 bApplyBindings=false
			SetGraphicsPipelineState(RHICmdList, GraphicsPSO, false);

			// 按参数结构批量绑定像素着色器常量
			SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), *PassParameters);

			// 视口覆盖整个 RT，画 1 个三角形（3 顶点由 SV_VertexID 生成，无需顶点缓冲）
			RHICmdList.SetViewport(0, 0, 0.0f, TextureSize.X, TextureSize.Y, 1.0f);
			RHICmdList.DrawPrimitive(0, 1, 1);
		});

	GraphBuilder.Execute();
}

bool UAuraShaderDemoLibrary::RenderPatternToRenderTarget(UTextureRenderTarget2D* RenderTarget, float Time, int32 PatternType)
{
	if (!RenderTarget || !RenderTarget->GetRenderTargetResource())
	{
		UE_LOG(LogTemp, Warning, TEXT("[AuraShaderDemo] Invalid render target."));
		return false;
	}

	const FIntPoint TextureSize(RenderTarget->SizeX, RenderTarget->SizeY);
	FTextureRenderTargetResource* RTResource = RenderTarget->GetRenderTargetResource();

	// 切换到渲染线程执行绘制
	ENQUEUE_RENDER_COMMAND(AuraPatternShader)(
		[RTResource, TextureSize, Time, PatternType](FRHICommandListImmediate& RHICmdList)
		{
			RenderAuraPattern_RenderThread(RHICmdList, RTResource->TextureRHI, TextureSize, Time, PatternType);
		});

	return true;
}
