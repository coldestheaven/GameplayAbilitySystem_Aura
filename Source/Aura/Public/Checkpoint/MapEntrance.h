// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Checkpoint/Checkpoint.h"
#include "MapEntrance.generated.h"

/**
 * 地图入口 Actor
 *
 * 继承自 ACheckpoint，是一种特殊的检查点，用于切换到另一个地图
 *
 * 与普通检查点的区别：
 * - 触碰后不是保存当前位置，而是切换到 DestinationMap 指定的地图
 * - 玩家在目标地图的出生位置由 DestinationPlayerStartTag 决定
 * - 高亮效果不同（重写了 HighlightActor_Implementation）
 *
 * 使用方式：
 *   在关卡出口处放置此 Actor
 *   配置 DestinationMap（目标地图）和 DestinationPlayerStartTag（目标出生点）
 *   玩家触碰后自动保存当前关卡状态并切换到目标地图
 */
UCLASS()
class AURA_API AMapEntrance : public ACheckpoint
{
	GENERATED_BODY()
public:
	AMapEntrance(const FObjectInitializer& ObjectInitializer);

	/* ======================== Highlight Interface 重写 ======================== */

	/**
	 * 开启高亮描边（重写基类）
	 * 地图入口使用不同的高亮颜色（如蓝色）以区别于普通检查点
	 */
	virtual void HighlightActor_Implementation() override;

	/* ======================== end Highlight Interface ======================== */

	/* ======================== Save Interface 重写 ======================== */

	/**
	 * 加载 Actor 状态（重写基类）
	 * 地图入口不需要恢复发光状态（每次进入关卡都是新的）
	 */
	virtual void LoadActor_Implementation() override;

	/* ======================== end Save Interface ======================== */

	/**
	 * 目标地图（软引用，避免强制加载）
	 * 玩家触碰此入口后切换到此地图
	 * 在 Details 面板中配置
	 */
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> DestinationMap;

	/**
	 * 目标地图中的玩家出生点标签
	 * 对应目标地图中某个 APlayerStart 的 PlayerStartTag
	 * 玩家将在目标地图的此出生点处出现
	 */
	UPROPERTY(EditAnywhere)
	FName DestinationPlayerStartTag;
	
protected:
	/**
	 * 球形碰撞体重叠回调（重写基类）
	 * 玩家进入范围时：
	 * 1. 保存当前关卡状态（SaveWorldState）
	 * 2. 设置目标地图和出生点信息到存档
	 * 3. 调用 GameMode::TravelToMap 切换地图
	 */
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	                             UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
	                             const FHitResult& SweepResult) override;
};
