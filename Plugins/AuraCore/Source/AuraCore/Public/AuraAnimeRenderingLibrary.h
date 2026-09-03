// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AuraAnimeRenderingLibrary.generated.h"

class UTextureRenderTarget2D;
class UTexture;

/**
 * 二次元（赛璐璐/卡通）渲染库（AuraCore 插件）
 *
 * HLSL 见 Plugins/AuraCore/Shaders/AuraAnimeRendering.usf，
 * 着色器类与 RDG 绘制见 Private/AuraAnimeRenderingLibrary.cpp。
 *
 * 两个入口：
 *   1. RenderAnimeSceneToRenderTarget —— 渲染程序化二次元场景
 *      StyleIndex: 0=完整场景(角色+描边+天空+集中线)
 *                 1=赛璐璐对比球(左连续光照/右分带，教学用)
 *                 2=场景+画面风格化(色阶+饱和度+暗角)
 *   2. ApplyAnimePostProcess —— 把【任意输入纹理】二次元化
 *      (Sobel 描边 + 色阶量化 + 饱和度提升)，本库新增"输入纹理采样"能力
 *
 * 核心技术：
 *   - Cel/赛璐璐着色：阶梯色带（step 阈值切带 + 冷紫阴影色）
 *   - Rim Light 边缘光
 *   - Ink 描边：SDF 边缘带（场景）/ Sobel 边缘检测（后处理）
 *   - Posterization 色阶量化、集中线（放射速度线）
 *   - SDF 数值梯度伪法线（2D 形状的体积光照）
 *
 * 使用示例（蓝图，Event Tick 逐帧）：
 *   // 直接看二次元场景
 *   RenderAnimeSceneToRenderTarget(MyRT, GameTime, 0);
 *   // 流水线：场景画到 RT_A → 后处理二次元化到 RT_B
 *   RenderAnimeSceneToRenderTarget(RT_A, GameTime, 0);
 *   ApplyAnimePostProcess(RT_A, RT_B, 1.0, 6, 1.4);
 *   // 也可把任意贴图资产传给 ApplyAnimePostProcess 二次元化
 */
UCLASS()
class AURACORE_API UAuraAnimeRenderingLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/**
	 * 渲染程序化二次元场景（蓝图可调用）
	 *
	 * @param RenderTarget 目标渲染目标（建议 512x512）
	 * @param Time         动画时间（秒），驱动云/集中线/发束
	 * @param StyleIndex   0=完整场景 1=赛璐璐对比球 2=场景+画面风格化
	 * @return true 表示已成功提交渲染命令
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraShader|Anime")
	static bool RenderAnimeSceneToRenderTarget(UTextureRenderTarget2D* RenderTarget, float Time = 0.f, int32 StyleIndex = 0);

	/**
	 * 把任意输入纹理二次元化（Sobel 描边 + 色阶量化 + 饱和度提升）
	 *
	 * 注意：
	 * - 源纹理与目标 RT 不能是同一张（读写冲突）
	 * - 源可以是贴图资产，也可以是另一张已渲染过的 RT（流水线用法）
	 *
	 * @param SourceTexture    输入纹理（贴图资产或另一张 RT）
	 * @param RenderTarget     输出渲染目标
	 * @param OutlineStrength  描边强度（0~2，默认 1）
	 * @param ColorSteps       色阶数（2~16，越小越"平涂"，默认 6）
	 * @param SaturationBoost  饱和度倍率（0~2，默认 1.4）
	 * @return true 表示已成功提交渲染命令
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraShader|Anime")
	static bool ApplyAnimePostProcess(UTexture* SourceTexture, UTextureRenderTarget2D* RenderTarget, float OutlineStrength = 1.f, int32 ColorSteps = 6, float SaturationBoost = 1.4f);
};
