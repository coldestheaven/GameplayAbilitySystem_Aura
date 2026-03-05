// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Actor/AuraProjectile.h"
#include "AuraFireBall.generated.h"

/**
 * 火球 Actor
 *
 * 用于火焰爆炸技能（UAuraFireBlast）的特殊投射物
 * 与普通投射物不同，火球会飞出后自动返回施法者
 * 去程和回程均可对命中的敌人造成伤害
 *
 * 工作流程：
 * 1. 生成后调用 StartOutgoingTimeline（蓝图事件）开始飞出动画
 * 2. 飞出到最大距离后，蓝图时间轴触发返回逻辑
 * 3. 返回时追踪 ReturnToActor（施法者）
 * 4. 命中目标时（去程或回程）应用 ExplosionDamageParams 中的伤害
 * 5. 返回到施法者附近后销毁
 *
 * 注意：
 * - 去程伤害使用基类的 DamageEffectParams
 * - 爆炸伤害（范围伤害）使用 ExplosionDamageParams
 */
UCLASS()
class AURA_API AAuraFireBall : public AAuraProjectile
{
	GENERATED_BODY()
public:

	/**
	 * 蓝图事件：开始飞出时间轴
	 * 在蓝图中实现火球飞出的运动曲线（加速飞出，然后减速返回）
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void StartOutgoingTimeline();

	/**
	 * 返回目标 Actor（施法者）
	 * 火球飞出后会追踪此 Actor 返回
	 * 由 UAuraFireBlast::SpawnFireBalls 在生成时设置
	 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> ReturnToActor;

	/**
	 * 爆炸伤害参数（范围伤害）
	 * 火球命中目标时触发的范围爆炸伤害参数
	 * 与基类的 DamageEffectParams（直接伤害）不同，此参数用于范围爆炸
	 * 由 UAuraFireBlast::SpawnFireBalls 在生成时设置
	 */
	UPROPERTY(BlueprintReadWrite)
	FDamageEffectParams ExplosionDamageParams;
	
protected:
	/** 初始化：设置 LifeSpan 等参数 */
	virtual void BeginPlay() override;

	/**
	 * 球形碰撞体重叠回调（重写基类）
	 * 火球命中目标时：
	 * - 去程：对目标造成直接伤害（DamageEffectParams）
	 * - 回程：对命中区域造成范围爆炸伤害（ExplosionDamageParams）
	 */
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	/**
	 * 命中处理（重写基类）
	 * 火球命中时触发范围爆炸，对周围所有敌人造成伤害
	 */
	virtual void OnHit() override;
};
