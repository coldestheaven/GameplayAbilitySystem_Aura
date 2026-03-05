// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HighlightInterface.generated.h"

/** UInterface 声明（不需要修改） */
UINTERFACE(MinimalAPI, BlueprintType)
class UHighlightInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 高亮接口
 *
 * 可以被鼠标悬停高亮显示的 Actor 实现此接口
 * 通过自定义深度（Custom Depth）实现轮廓描边效果
 *
 * 实现此接口的类：
 * - AAuraEnemy：鼠标悬停时显示红色描边（表示可攻击目标）
 * - ACheckpoint：鼠标悬停时显示棕褐色描边（表示可交互检查点）
 * - AMapEntrance：鼠标悬停时显示蓝色描边（表示地图入口）
 *
 * 高亮实现原理：
 * - 开启高亮：设置 Mesh 的 bRenderCustomDepth=true，CustomDepthStencilValue=颜色值
 * - 关闭高亮：设置 Mesh 的 bRenderCustomDepth=false
 * - 后处理材质根据 CustomDepthStencilValue 绘制对应颜色的描边
 *
 * 使用方式（在 PlayerController 的 CursorTrace 中）：
 *   if (ThisActor != LastActor)
 *   {
 *       if (LastActor) IHighlightInterface::Execute_UnHighlightActor(LastActor);
 *       if (ThisActor) IHighlightInterface::Execute_HighlightActor(ThisActor);
 *   }
 */
class AURACORE_API IHighlightInterface
{
	GENERATED_BODY()
public:
	/**
	 * 开启高亮描边（蓝图原生事件）
	 * 设置 Mesh 的自定义深度渲染，使后处理材质绘制轮廓描边
	 * 鼠标悬停在此 Actor 上时由 PlayerController 调用
	 */
	UFUNCTION(BlueprintNativeEvent)
	void HighlightActor();
	
	/**
	 * 关闭高亮描边（蓝图原生事件）
	 * 关闭 Mesh 的自定义深度渲染，移除轮廓描边
	 * 鼠标离开此 Actor 时由 PlayerController 调用
	 */
	UFUNCTION(BlueprintNativeEvent)
	void UnHighlightActor();

	/**
	 * 设置移动目标位置（蓝图原生事件）
	 * 当玩家点击此 Actor 时，返回推荐的移动目标位置
	 * 避免玩家移动到 Actor 正中心（可能被遮挡或重叠）
	 * @param OutDestination 输出：推荐的移动目标位置
	 */
	UFUNCTION(BlueprintNativeEvent)
	void SetMoveToLocation(FVector& OutDestination);
};
