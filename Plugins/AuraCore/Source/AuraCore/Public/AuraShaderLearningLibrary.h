// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AuraShaderLearningLibrary.generated.h"

class UTextureRenderTarget2D;

/**
 * Aura Shader 学习课程库（AuraCore 插件）
 *
 * 六课渐进式 Shader 教学，HLSL 见 Plugins/AuraCore/Shaders/AuraShaderLessons.usf，
 * C++ 着色器类与 RDG 绘制见 Private/AuraShaderLearningLibrary.cpp。
 *
 * 课程表（LessonIndex 传入 RenderLessonToRenderTarget）：
 *   1: 渐变与 Uniform 参数 —— 最基础：常量绑定、UV 插值、step/smoothstep
 *   2: SDF 有符号距离场   —— 形状数学、布尔运算(min/max)、抗锯齿、辉光
 *   3: 程序化噪声         —— Hash、Value Noise、FBM 分形叠加、火焰配色
 *   4: UV 变换            —— 漩涡扭曲、万花筒折叠（先改坐标再采样）
 *   5: 程序化光照         —— 高度差分法线、Blinn-Phong 漫反射/高光、移动光源
 *   6: 计算着色器         —— 单独入口 RenderComputeLessonToRenderTarget：
 *                            UAV 写入、SV_DispatchThreadID、[numthreads] 线程组
 *
 * 使用示例（蓝图，建议放在 Event Tick 里逐帧刷新）：
 *   UAuraShaderLearningLibrary::RenderLessonToRenderTarget(MyRT, 3, GameTimeInSeconds);
 *   UAuraShaderLearningLibrary::RenderComputeLessonToRenderTarget(MyRT, GameTimeInSeconds);
 *   然后把 MyRT 设给 UMG Image（SetBrushFromRenderTarget）即可查看。
 */
UCLASS()
class AURACORE_API UAuraShaderLearningLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/**
	 * 渲染第 1~5 课的像素着色器课程（光栅管线：全屏三角形）
	 *
	 * @param RenderTarget 目标渲染目标（尺寸即输出尺寸，建议 512x512）
	 * @param LessonIndex  课程编号 1~5（越界自动回退到 1）
	 * @param Time         动画时间（秒），逐帧累加可看到动态效果
	 * @return true 表示已成功提交渲染命令
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraShader|Lessons")
	static bool RenderLessonToRenderTarget(UTextureRenderTarget2D* RenderTarget, int32 LessonIndex = 1, float Time = 0.f);

	/**
	 * 渲染第 6 课：计算着色器（GPGPU 管线）
	 *
	 * 流程：CS 写 UAV 中间纹理(RDG 创建) → AddDrawTexturePass 拷贝到渲染目标。
	 * 对比第 1~5 课可直观看到两种管线（光栅 vs 计算）的差异。
	 *
	 * @param RenderTarget 目标渲染目标
	 * @param Time         动画时间（秒）
	 * @return true 表示已成功提交渲染命令
	 */
	UFUNCTION(BlueprintCallable, Category = "AuraShader|Lessons")
	static bool RenderComputeLessonToRenderTarget(UTextureRenderTarget2D* RenderTarget, float Time = 0.f);
};
