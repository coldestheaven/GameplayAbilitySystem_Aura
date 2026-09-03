// Copyright Druid Mechanics

#include "AuraAnimeRenderingLibrary.h"

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraph.h"
#include "RenderGraphUtils.h"
#include "RenderingThread.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture.h"

// ───────── 全屏三角形顶点着色器 ─────────
class FAuraAnimeVS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FAuraAnimeVS);
public:
	FAuraAnimeVS() = default;
	FAuraAnimeVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return true; }
};

// ───────── 二次元场景像素着色器 ─────────
class FAuraAnimeScenePS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FAuraAnimeScenePS);
	SHADER_USE_PARAMETER_STRUCT(FAuraAnimeScenePS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FVector2f, TextureSize)
		SHADER_PARAMETER(float, Time)
		SHADER_PARAMETER(uint32, StyleIndex)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return true; }
};

// ───────── 后处理像素着色器（新增：输入纹理采样） ─────────
class FAuraAnimePostProcessPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FAuraAnimePostProcessPS);
	SHADER_USE_PARAMETER_STRUCT(FAuraAnimePostProcessPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FVector2f, TextureSize)
		SHADER_PARAMETER(float, OutlineStrength)
		SHADER_PARAMETER(float, ColorSteps)
		SHADER_PARAMETER(float, SaturationBoost)
		// RDG 通道中绑纹理须用 RDG 宏（成员类型为 FRDGTextureRef，而非直接 RHI 指针）
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SourceTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, SourceTextureSampler)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters) { return true; }
};

IMPLEMENT_GLOBAL_SHADER(FAuraAnimeVS,             "/Plugin/AuraCore/AuraAnimeRendering.usf", "AnimeVS",             SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FAuraAnimeScenePS,        "/Plugin/AuraCore/AuraAnimeRendering.usf", "AnimeScenePS",        SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FAuraAnimePostProcessPS,  "/Plugin/AuraCore/AuraAnimeRendering.usf", "AnimePostProcessPS",  SF_Pixel);

// ───────── 光栅 Pass 公共绘制（全屏三角形 + PSO + 参数绑定） ─────────
template <typename TPixelShader>
static void AddFullscreenAnimePass(
	FRDGBuilder& GraphBuilder,
	FGlobalShaderMap* ShaderMap,
	FRDGTextureRef DestTexture,
	typename TPixelShader::FParameters* PassParameters,
	FIntPoint TextureSize,
	const TCHAR* PassName)
{
	TShaderMapRef<FAuraAnimeVS> VertexShader(ShaderMap);
	TShaderMapRef<TPixelShader> PixelShader(ShaderMap);

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("%s", PassName),
		PassParameters,
		ERDGPassFlags::Raster,
		[VertexShader, PixelShader, PassParameters, TextureSize](FRHICommandList& RHICmdList)
		{
			FGraphicsPipelineStateInitializer GraphicsPSO;
			GraphicsPSO.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSO.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
			GraphicsPSO.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
			GraphicsPSO.PrimitiveType = PT_TriangleList;
			GraphicsPSO.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSO.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
			// UE5.8：自由函数版移除了默认参，需显式传 bApplyBindings=false
			SetGraphicsPipelineState(RHICmdList, GraphicsPSO, false);

			SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), *PassParameters);

			RHICmdList.SetViewport(0, 0, 0.0f, TextureSize.X, TextureSize.Y, 1.0f);
			RHICmdList.DrawPrimitive(0, 1, 1);
		});
}

// ───────── 场景渲染（渲染线程） ─────────
static void RenderAnimeScene_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureRHIRef OutputTexture, FIntPoint TextureSize, float Time, int32 StyleIndex)
{
	FRDGBuilder GraphBuilder(RHICmdList);
	FRDGTextureRef DestRef = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(OutputTexture, TEXT("AuraAnimeRT")));

	FAuraAnimeScenePS::FParameters* Params = GraphBuilder.AllocParameters<FAuraAnimeScenePS::FParameters>();
	Params->TextureSize = FVector2f(TextureSize.X, TextureSize.Y);
	Params->Time = Time;
	Params->StyleIndex = static_cast<uint32>(StyleIndex);
	Params->RenderTargets[0] = FRenderTargetBinding(DestRef, ERenderTargetLoadAction::EClear);

	AddFullscreenAnimePass<FAuraAnimeScenePS>(GraphBuilder, GetGlobalShaderMap(GMaxRHIFeatureLevel), DestRef, Params, TextureSize, TEXT("AuraAnimeScene"));
	GraphBuilder.Execute();
}

// ───────── 后处理（渲染线程）：源纹理 SRV 采样 → 目标 RT ─────────
static void ApplyAnimePostProcess_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureRHIRef SourceTexture, FTextureRHIRef OutputTexture, FIntPoint TextureSize, float OutlineStrength, int32 ColorSteps, float SaturationBoost)
{
	FRDGBuilder GraphBuilder(RHICmdList);

	// 源与目标都注册为渲染图外部资源（RDG 管理 SRV 与状态）
	FRDGTextureRef SourceRef = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(SourceTexture, TEXT("AuraAnimeSrc")));
	FRDGTextureRef DestRef = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(OutputTexture, TEXT("AuraAnimeRT")));

	FAuraAnimePostProcessPS::FParameters* Params = GraphBuilder.AllocParameters<FAuraAnimePostProcessPS::FParameters>();
	Params->TextureSize = FVector2f(TextureSize.X, TextureSize.Y);
	Params->OutlineStrength = OutlineStrength;
	Params->ColorSteps = static_cast<float>(ColorSteps);
	Params->SaturationBoost = SaturationBoost;
	Params->SourceTexture = SourceRef;
	Params->SourceTextureSampler = TStaticSamplerState<SF_Point>::GetRHI();
	Params->RenderTargets[0] = FRenderTargetBinding(DestRef, ERenderTargetLoadAction::EClear);

	AddFullscreenAnimePass<FAuraAnimePostProcessPS>(GraphBuilder, GetGlobalShaderMap(GMaxRHIFeatureLevel), DestRef, Params, TextureSize, TEXT("AuraAnimePostProcess"));
	GraphBuilder.Execute();
}

// ───────── 蓝图入口 ─────────
bool UAuraAnimeRenderingLibrary::RenderAnimeSceneToRenderTarget(UTextureRenderTarget2D* RenderTarget, float Time, int32 StyleIndex)
{
	if (!RenderTarget || !RenderTarget->GetRenderTargetResource())
	{
		UE_LOG(LogTemp, Warning, TEXT("[AuraAnime] Invalid render target."));
		return false;
	}

	const FIntPoint TextureSize(RenderTarget->SizeX, RenderTarget->SizeY);
	FTextureRenderTargetResource* RTResource = RenderTarget->GetRenderTargetResource();

	ENQUEUE_RENDER_COMMAND(AuraAnimeScene)(
		[RTResource, TextureSize, Time, StyleIndex](FRHICommandListImmediate& RHICmdList)
		{
			RenderAnimeScene_RenderThread(RHICmdList, RTResource->TextureRHI, TextureSize, Time, StyleIndex);
		});
	return true;
}

bool UAuraAnimeRenderingLibrary::ApplyAnimePostProcess(UTexture* SourceTexture, UTextureRenderTarget2D* RenderTarget, float OutlineStrength, int32 ColorSteps, float SaturationBoost)
{
	if (!SourceTexture || !RenderTarget || !RenderTarget->GetRenderTargetResource())
	{
		UE_LOG(LogTemp, Warning, TEXT("[AuraAnime] Invalid source texture or render target."));
		return false;
	}

	FTextureResource* SourceResource = SourceTexture->GetResource();
	if (!SourceResource || !SourceResource->TextureRHI.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("[AuraAnime] Source texture has no valid GPU resource (RT needs to be rendered at least once)."));
		return false;
	}
	if (SourceResource->TextureRHI == RenderTarget->GetRenderTargetResource()->TextureRHI)
	{
		UE_LOG(LogTemp, Warning, TEXT("[AuraAnime] Source and destination must be different textures."));
		return false;
	}

	const FIntPoint TextureSize(RenderTarget->SizeX, RenderTarget->SizeY);
	FTextureRHIRef SourceRHI = SourceResource->TextureRHI;
	FTextureRenderTargetResource* RTResource = RenderTarget->GetRenderTargetResource();

	ENQUEUE_RENDER_COMMAND(AuraAnimePostProcess)(
		[SourceRHI, RTResource, TextureSize, OutlineStrength, ColorSteps, SaturationBoost](FRHICommandListImmediate& RHICmdList)
		{
			ApplyAnimePostProcess_RenderThread(RHICmdList, SourceRHI, RTResource->TextureRHI, TextureSize, OutlineStrength, ColorSteps, SaturationBoost);
		});
	return true;
}
