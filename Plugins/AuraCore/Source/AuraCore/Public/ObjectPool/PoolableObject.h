// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PoolableObject.generated.h"

/** UInterface 声明（不需要修改） */
UINTERFACE(MinimalAPI, Blueprintable)
class UPoolableObject : public UInterface
{
	GENERATED_BODY()
};

/**
 * 可池化对象接口
 *
 * 需要被对象池（UObjectPoolSubsystem）管理的 Actor 实现此接口
 * 对象池通过此接口通知 Actor 被取出或归还，Actor 可以在回调中重置/清理状态
 *
 * 实现此接口的类：
 * - AAuraProjectile：投射物，从池中取出时重置 bHit 状态，归还时停止音效
 *
 * 使用方式：
 *   // 在 Actor 类中实现接口
 *   virtual void OnAcquiredFromPool_Implementation() override
 *   {
 *       bHit = false;
 *       SetActorHiddenInGame(false);
 *       SetActorEnableCollision(true);
 *   }
 *   virtual void OnReturnedToPool_Implementation() override
 *   {
 *       SetActorHiddenInGame(true);
 *       SetActorEnableCollision(false);
 *       if (LoopingSoundComponent) LoopingSoundComponent->Stop();
 *   }
 */
class AURACORE_API IPoolableObject
{
	GENERATED_BODY()

public:
	/**
	 * 从对象池中取出时调用（蓝图原生事件，蓝图可调用）
	 * 在此函数中重置 Actor 状态，准备复用
	 * 例如：重置 bHit 标志、激活碰撞、显示 Actor、重新播放特效
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectPool")
	void OnAcquiredFromPool();

	/**
	 * 归还到对象池时调用（蓝图原生事件，蓝图可调用）
	 * 在此函数中清理 Actor 状态，准备回收
	 * 例如：停止音效、隐藏 Actor、禁用碰撞、停止移动
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectPool")
	void OnReturnedToPool();

	/**
	 * 检查对象是否可以归还到池中（蓝图原生事件，蓝图可调用）
	 * 对象池在归还前调用此函数，如果返回 false 则直接销毁 Actor
	 * @return true 表示可以归还复用，false 表示应该销毁（如 Actor 状态已损坏）
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectPool")
	bool CanReturnToPool() const;
};
