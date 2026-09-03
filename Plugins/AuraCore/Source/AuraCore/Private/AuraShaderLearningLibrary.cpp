// Copyright Druid Mechanics

#include "AuraShaderLearningLibrary.h"

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraph.h"
#include "RenderGraphUtils.h"
#include "RenderingThread.h"
#include "Engine/TextureRenderTarget2D.h"

// ───────── 全屏三角形顶点着色器（Lesson 1~5 共用） ─────────
class FAuraLessonVS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FAuraLessonVS);
public:
	FAuraLessonVS() = default;
	FAuraLessonVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};

// ───────── 像素着色器（LessonIndex 分发到 .usf 里对应课程实现） ─────────
class FAuraLessonPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FAuraLessonPS);
	SHADER_USE_PARAMETER_STRUCT(FAuraLessonPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FVector2f, TextureSize)
		SHADER_PARAMETER(float, Time)
		SHADER_PARAMETER(uint32, LessonIndex)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};

// ───────── Lesson 6 计算着色器（UAV 写入） ─────────
class FAuraComputeCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FAuraComputeCS);
	SHADER_USE_PARAMETER_STRUCT(FAuraComputeCS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FVector2f, TextureSize)
		SHADER_PARAMETER(float, Time)
		// RDG 通道中 UAV 须用 RDG 宏（成员为 FRDGTextureUAVRef，接受 GraphBuilder.CreateUAV 返回值）
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutputTexture)
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};

// ───────── Blit 像素着色器（计算结果 → 渲染目标，替代签名已变的 AddDrawTexturePass） ─────────
class FAuraBlitPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FAuraBlitPS);
	SHADER_USE_PARAMETER_STRUCT(FAuraBlitPS, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE(Texture2D, SourceTexture)
		SHADER_PARAMETER_SAMPLER(SamplerState, SourceTextureSampler)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
};

IMPLEMENT_GLOBAL_SHADER(FAuraLessonVS,  "/Plugin/AuraCore/AuraShaderLessons.usf", "AuraLessonVS",     SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FAuraLessonPS,  "/Plugin/AuraCore/AuraShaderLessons.usf", "AuraLessonPS",     SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FAuraComputeCS, "/Plugin/AuraCore/AuraShaderLessons.usf", "Lesson6_ComputeCS", SF_Compute);
IMPLEMENT_GLOBAL_SHADER(FAuraBlitPS,    "/Plugin/AuraCore/AuraShaderLessons.usf", "AuraBlitPS",       SF_Pixel);

/**
 * 光栅管线渲染（Lesson 1~5）：全屏三角形 + 像素着色器
 * 与 AuraShaderDemoLibrary 的绘制模式相同，可对照学习
 */
static void RenderLesson_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureRHIRef OutputTexture, FIntPoint TextureSize, float Time, int32 LessonIndex)
{
	FRDGBuilder GraphBuilder(RHICmdList);

	FRDGTextureRef OutputTextureRef = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(OutputTexture, TEXT("AuraLessonRT")));

	FAuraLessonPS::FParameters* PassParameters = GraphBuilder.AllocParameters<FAuraLessonPS::FParameters>();
	PassParameters->TextureSize = FVector2f(TextureSize.X, TextureSize.Y);
	PassParameters->Time = Time;
	PassParameters->LessonIndex = static_cast<uint32>(LessonIndex);
	PassParameters->RenderTargets[0] = FRenderTargetBinding(OutputTextureRef, ERenderTargetLoadAction::EClear);

	FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<FAuraLessonVS> VertexShader(ShaderMap);
	TShaderMapRef<FAuraLessonPS> PixelShader(ShaderMap);

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("AuraLesson%d %dx%d", LessonIndex, TextureSize.X, TextureSize.Y),
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

	GraphBuilder.Execute();
}

/**
 * 计算管线渲染（Lesson 6）：
 * 1. RDG 创建 UAV 可写的浮点中间纹理（渲染目标本身不保证支持 UAV，故用中间纹理）
 * 2. CS Dispatch：按 8x8 线程组分发，每线程写一个像素
 * 3. AddDrawTexturePass：把计算结果（SRV 采样）拷贝到外部渲染目标
 */
static void RenderComputeLesson_RenderThread(FRHICommandListImmediate& RHICmdList, FTextureRHIRef OutputTexture, FIntPoint TextureSize, float Time)
{
	FRDGBuilder GraphBuilder(RHICmdList);

	FRDGTextureRef DestTextureRef = GraphBuilder.RegisterExternalTexture(CreateRenderTarget(OutputTexture, TEXT("AuraLessonRT")));

	// UAV 可写的中间纹理（浮点格式，SRV + UAV 双标志）
	FRDGTextureDesc ComputeDesc = FRDGTextureDesc::Create2D(TextureSize, PF_FloatRGBA, FClearValueBinding::Black, TexCreate_ShaderResource | TexCreate_UAV);
	FRDGTextureRef ComputeTexture = GraphBuilder.CreateTexture(ComputeDesc, TEXT("AuraComputeOut"));

	FGlobalShaderMap* ShaderMap = GetGlobalShaderMap(GMaxRHIFeatureLevel);
	TShaderMapRef<FAuraComputeCS> ComputeShader(ShaderMap);

	FAuraComputeCS::FParameters* ComputeParameters = GraphBuilder.AllocParameters<FAuraComputeCS::FParameters>();
	ComputeParameters->TextureSize = FVector2f(TextureSize.X, TextureSize.Y);
	ComputeParameters->Time = Time;
	ComputeParameters->OutputTexture = GraphBuilder.CreateUAV(ComputeTexture);

	// 线程组数 = ceil(像素数 / 8)（与 .usf 的 [numthreads(8,8,1)] 对应）
	const FIntVector GroupCount = FComputeShaderUtils::GetGroupCount(FIntVector(TextureSize.X, TextureSize.Y, 1), 8);

	FComputeShaderUtils::AddPass(
		GraphBuilder,
		RDG_EVENT_NAME("AuraLesson6Compute %dx%d", TextureSize.X, TextureSize.Y),
		ComputeShader,
		ComputeParameters,
		GroupCount);

	// 计算结果 → 渲染目标：自写 blit pass（采样拷贝，自动处理 FloatRGBA→RT 格式转换，
	// 替代 UE5.8 中签名已变的 AddDrawTexturePass）
	{
		FAuraBlitPS::FParameters* BlitParams = GraphBuilder.AllocParameters<FAuraBlitPS::FParameters>();
		BlitParams->SourceTexture = ComputeTexture;
		BlitParams->SourceTextureSampler = TStaticSamplerState<SF_Point>::GetRHI();
		BlitParams->RenderTargets[0] = FRenderTargetBinding(DestTextureRef, ERenderTargetLoadAction::EClear);

		TShaderMapRef<FAuraLessonVS> VertexShader(ShaderMap);
		TShaderMapRef<FAuraBlitPS> BlitShader(ShaderMap);

		GraphBuilder.AddPass(
			RDG_EVENT_NAME("AuraLessonBlit %dx%d", TextureSize.X, TextureSize.Y),
			BlitParams,
			ERDGPassFlags::Raster,
			[VertexShader, BlitShader, BlitParams, TextureSize](FRHICommandList& RHICmdList)
			{
				FGraphicsPipelineStateInitializer GraphicsPSO;
				GraphicsPSO.BlendState = TStaticBlendState<>::GetRHI();
				GraphicsPSO.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
				GraphicsPSO.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None>::GetRHI();
				GraphicsPSO.PrimitiveType = PT_TriangleList;
				GraphicsPSO.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
				GraphicsPSO.BoundShaderState.PixelShaderRHI = BlitShader.GetPixelShader();
				SetGraphicsPipelineState(RHICmdList, GraphicsPSO, false);

				SetShaderParameters(RHICmdList, BlitShader, BlitShader.GetPixelShader(), *BlitParams);

				RHICmdList.SetViewport(0, 0, 0.0f, TextureSize.X, TextureSize.Y, 1.0f);
				RHICmdList.DrawPrimitive(0, 1, 1);
			});
	}

	GraphBuilder.Execute();
}

bool UAuraShaderLearningLibrary::RenderLessonToRenderTarget(UTextureRenderTarget2D* RenderTarget, int32 LessonIndex, float Time)
{
	if (!RenderTarget || !RenderTarget->GetRenderTargetResource())
	{
		UE_LOG(LogTemp, Warning, TEXT("[AuraShaderLessons] Invalid render target."));
		return false;
	}

	const int32 ClampedLesson = FMath::Clamp(LessonIndex, 1, 5);
	const FIntPoint TextureSize(RenderTarget->SizeX, RenderTarget->SizeY);
	FTextureRenderTargetResource* RTResource = RenderTarget->GetRenderTargetResource();

	ENQUEUE_RENDER_COMMAND(AuraLessonShader)(
		[RTResource, TextureSize, Time, ClampedLesson](FRHICommandListImmediate& RHICmdList)
		{
			RenderLesson_RenderThread(RHICmdList, RTResource->TextureRHI, TextureSize, Time, ClampedLesson);
		});

	return true;
}

bool UAuraShaderLearningLibrary::RenderComputeLessonToRenderTarget(UTextureRenderTarget2D* RenderTarget, float Time)
{
	if (!RenderTarget || !RenderTarget->GetRenderTargetResource())
	{
		UE_LOG(LogTemp, Warning, TEXT("[AuraShaderLessons] Invalid render target."));
		return false;
	}

	const FIntPoint TextureSize(RenderTarget->SizeX, RenderTarget->SizeY);
	FTextureRenderTargetResource* RTResource = RenderTarget->GetRenderTargetResource();

	ENQUEUE_RENDER_COMMAND(AuraComputeLessonShader)(
		[RTResource, TextureSize, Time](FRHICommandListImmediate& RHICmdList)
		{
			RenderComputeLesson_RenderThread(RHICmdList, RTResource->TextureRHI, TextureSize, Time);
		});

	return true;
}
