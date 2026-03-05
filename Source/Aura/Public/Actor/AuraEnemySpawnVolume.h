// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interaction/SaveInterface.h"
#include "AuraEnemySpawnVolume.generated.h"

class AAuraEnemySpawnPoint;
class UBoxComponent;

/**
 * 敌人生成体积 Actor
 *
 * 实现了 ISaveInterface，支持存档系统
 * 管理一组 AAuraEnemySpawnPoint，当玩家进入触发区域时生成所有敌人
 *
 * 功能：
 * - 使用 BoxComponent 作为触发区域
 * - 玩家进入时触发所有 SpawnPoints 生成敌人
 * - 生成后标记 bReached=true，防止重复生成
 * - bReached 会被存档，重新加载关卡时不会再次生成
 *
 * 存档说明：
 * - bReached 标记了 SaveGame 说明符，会被序列化到存档
 * - LoadActor_Implementation 在加载存档时调用，如果 bReached=true 则不重新绑定重叠回调
 *
 * 使用方式：
 *   在关卡中放置此 Actor，配置 SpawnPoints 数组（添加多个 AuraEnemySpawnPoint）
 *   调整 Box 大小以覆盖玩家需要进入的区域
 */
UCLASS()
class AURA_API AAuraEnemySpawnVolume : public AActor, public ISaveInterface
{
	GENERATED_BODY()
	
public:	
	AAuraEnemySpawnVolume();

	/* ======================== Save Interface 实现 ======================== */

	/**
	 * 加载 Actor 状态（重写基类）
	 * 如果 bReached=true（已触发过），则不重新绑定重叠回调
	 * 防止重新加载关卡时再次生成已经生成过的敌人
	 */
	virtual void LoadActor_Implementation() override;

	/* ======================== end Save Interface ======================== */

	/**
	 * 是否已触发（已生成过敌人）
	 * SaveGame 说明符确保此值被序列化到存档
	 * true：已触发，不再重复生成
	 */
	UPROPERTY(BlueprintReadOnly, SaveGame)
	bool bReached = false;

protected:
	/** 初始化：绑定 Box 的重叠回调 */
	virtual void BeginPlay() override;

	/**
	 * Box 碰撞体重叠回调
	 * 玩家进入 Box 范围时调用
	 * 触发所有 SpawnPoints 生成敌人，然后标记 bReached=true
	 */
	UFUNCTION()
	virtual void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * 敌人生成点数组
	 * 包含此体积管理的所有生成点
	 * 在 Details 面板中配置（拖入场景中的 AuraEnemySpawnPoint Actor）
	 */
	UPROPERTY(EditAnywhere)
	TArray<AAuraEnemySpawnPoint*> SpawnPoints;

private:
	/**
	 * Box 碰撞体（触发区域）
	 * 玩家进入此 Box 时触发敌人生成
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> Box;
};
