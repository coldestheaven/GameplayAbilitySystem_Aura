// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AuraShaderDemoLibrary.generated.h"

class UTextureRenderTarget2D;

/**
 * Aura 着色器演示工具库（AuraCore 插件）
 *
 * 暴露 C++ 全局着色器（FGlobalShader）的蓝图入口。
 * 着色器实现见 Private/AuraShaderDemoLibrary.cpp，
 * HLSL 见 Plugins/AuraCore/Shaders/AuraShaderDemo.usf
 * （插件 Shaders 目录自动映射为虚拟路径 /Plugin/AuraCore）。
 *
 * 数据流：
 *   蓝图/C++ 调用 RenderPatternToRenderTarget
 *     → ENQUEUE_RENDER_COMMAND 切到渲染线程
 *     → RDG 图注册外部渲染目标 + 添加栅格通道
 *     → 全屏三角形绘制 AuraPatternPS（程序化图案）
 *
 * 使用示例：
 *   // 在任意 Actor/Widget 蓝图中：
 *   UAuraShaderDemoLibrary::RenderPatternToRenderTarget(MyRenderTarget, ElapsedTime, 0);
 *   // 再把 MyRenderTarget 设给 UMG Image 控件（SetBrushFromRenderTarget）
 */
UCLASS()
class AURACORE_API UAuraShaderDemoLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/**
	 * 将奥术主题的程序化图案绘制到渲染目标（蓝图可调用）
	 *
	 * @param RenderTarget 目标渲染目标（尺寸即输出尺寸）
	 * @param Time         动画时间（秒），逐帧累加可获得动态效果
	 * @param PatternType  图案类型：0=奥术波浪(默认) 1=同心圆环 2=棋盘
	 * @return true 表示已成功提交渲染命令（无效目标返回 false）
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraShader|Demo")
	static bool RenderPatternToRenderTarget(UTextureRenderTarget2D* RenderTarget, float Time = 0.f, int32 PatternType = 0);
};
