// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "SaveInterface.generated.h"

/** UInterface 声明（不需要修改） */
UINTERFACE(MinimalAPI)
class USaveInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 存档接口
 *
 * 需要保存/加载状态的 Actor 实现此接口
 * 由 AAuraGameModeBase::SaveWorldState 和 LoadWorldState 调用
 *
 * 实现此接口的类：
 * - ACheckpoint：保存 bReached 状态（是否已到达）
 * - AMapEntrance：保存 bReached 状态
 * - AAuraEnemySpawnVolume：保存 bReached 状态（是否已生成敌人）
 *
 * 存档流程：
 *   SaveWorldState：遍历关卡中所有 Actor
 *     → 如果实现了 ISaveInterface，序列化其 SaveGame 属性到 FSavedActor.Bytes
 *   LoadWorldState：遍历关卡中所有 Actor
 *     → 如果实现了 ISaveInterface，反序列化 FSavedActor.Bytes 到 Actor 属性
 *     → 调用 LoadActor_Implementation 执行加载后的初始化逻辑
 */
class AURACORE_API ISaveInterface
{
	GENERATED_BODY()
public:
	/**
	 * 是否需要加载变换（蓝图可调用，蓝图原生事件）
	 * 返回 true 时，加载存档会恢复 Actor 的位置、旋转、缩放
	 * 返回 false 时，Actor 保持关卡中的原始变换（适用于固定位置的 Actor）
	 * @return true 表示需要恢复变换
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool ShouldLoadTransform();

	/**
	 * 加载 Actor 状态（蓝图可调用，蓝图原生事件）
	 * 在存档数据反序列化到 Actor 属性后调用
	 * 子类在此函数中执行加载后的初始化逻辑
	 * 例如：检查点在此函数中根据 bReached 决定是否显示发光特效
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void LoadActor();
};
