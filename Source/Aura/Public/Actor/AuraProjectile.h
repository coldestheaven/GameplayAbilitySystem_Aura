// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AuraAbilityTypes.h"
#include "ObjectPool/PoolableObject.h"
#include "GameFramework/Actor.h"
#include "AuraProjectile.generated.h"

class UNiagaraSystem;
class USphereComponent;
class UProjectileMovementComponent;

/**
 * Aura 投射物基类
 *
 * 所有飞行投射物（火焰箭、火球、电击球等）均继承自此类
 * 同时实现了 IPoolableObject 接口，支持对象池复用
 *
 * 功能：
 * - 使用 ProjectileMovementComponent 控制飞行轨迹（支持追踪目标）
 * - 使用 SphereComponent 进行碰撞检测
 * - 命中目标时应用 DamageEffectParams 中定义的伤害效果
 * - 支持循环音效（飞行中播放）和命中特效（命中时播放）
 * - 支持对象池：从池中取出时重置状态，归还时停止特效和音效
 *
 * 生命周期：
 *   生成 → BeginPlay（播放循环音效）→ 飞行
 *   → OnSphereOverlap（命中目标）→ OnHit（播放命中特效、应用伤害）→ 销毁/归还对象池
 *
 * 网络说明：
 * - 投射物在服务端生成，通过 Actor 复制同步到客户端
 * - 伤害只在服务端应用，客户端只负责显示特效
 */
UCLASS()
class AURA_API AAuraProjectile : public AActor, public IPoolableObject
{
	GENERATED_BODY()
	
public:	
	AAuraProjectile();

	/**
	 * 投射物移动组件
	 * 控制投射物的飞行速度、方向和追踪行为
	 * 可配置为直线飞行或追踪目标（HomingTarget）
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	/**
	 * 伤害效果参数
	 * 包含伤害类型、数值、Debuff 参数等，在生成投射物时由技能设置
	 * 命中目标时通过 UAuraAbilitySystemLibrary::ApplyDamageEffect 应用
	 * ExposeOnSpawn 允许在 SpawnActor 时直接设置此参数
	 */
	UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true))
	FDamageEffectParams DamageEffectParams;

	/**
	 * 追踪目标的场景组件
	 * 当投射物需要追踪目标时，将此组件附加到目标 Actor 上
	 * ProjectileMovement 的 HomingTargetComponent 指向此组件
	 */
	UPROPERTY()
	TObjectPtr<USceneComponent> HomingTargetSceneComponent;

public:
	/* ======================== IPoolableObject 接口实现 ======================== */

	/**
	 * 从对象池取出时调用
	 * 重置投射物状态（bHit=false），重新激活碰撞和移动
	 */
	virtual void OnAcquiredFromPool_Implementation() override;

	/**
	 * 归还对象池时调用
	 * 停止循环音效，隐藏 Actor，禁用碰撞和移动
	 */
	virtual void OnReturnedToPool_Implementation() override;

	/**
	 * 判断是否可以归还对象池
	 * @return true 表示可以归还（通常在命中或超时后返回 true）
	 */
	virtual bool CanReturnToPool_Implementation() const override;

protected:
	/** 开始播放循环飞行音效 */
	virtual void BeginPlay() override;

	/**
	 * 命中处理（蓝图可调用）
	 * 播放命中特效和音效，应用伤害，然后销毁或归还对象池
	 * 子类可重写以添加额外的命中逻辑
	 */
	UFUNCTION(BlueprintCallable)
	virtual void OnHit();

	/**
	 * Actor 销毁时调用
	 * 如果未命中（bHit=false），在销毁时播放命中特效（防止特效丢失）
	 */
	virtual void Destroyed() override;

	/**
	 * 球形碰撞体重叠回调
	 * 当投射物与其他 Actor 发生重叠时调用
	 * 验证目标有效性后调用 OnHit
	 */
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/**
	 * 球形碰撞体（投射物的碰撞检测组件）
	 * 半径决定命中判定范围，与目标重叠时触发 OnSphereOverlap
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<USphereComponent> Sphere;

	/**
	 * 验证重叠目标是否有效
	 * 过滤条件：目标不是自身、不是友方、目标有 ASC
	 * @param OtherActor 重叠的目标 Actor
	 * @return true 表示目标有效，可以造成伤害
	 */
	bool IsValidOverlap(AActor* OtherActor);

	/** 是否已命中（防止多次触发命中逻辑） */
	bool bHit = false;

	/** 循环飞行音效的音频组件（用于在命中时停止音效） */
	UPROPERTY()
	TObjectPtr<UAudioComponent> LoopingSoundComponent;

private:
	/**
	 * 投射物最大存活时间（秒）
	 * 超过此时间未命中目标则自动销毁，防止投射物永久存在
	 * 默认 15 秒
	 */
	UPROPERTY(EditDefaultsOnly)
	float LifeSpan = 15.f;

	/** 命中时播放的 Niagara 粒子特效（爆炸、火花等） */
	UPROPERTY(EditAnywhere)
	TObjectPtr<UNiagaraSystem> ImpactEffect;

	/** 命中时播放的音效（爆炸声、撞击声等） */
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> ImpactSound;

	/** 飞行中循环播放的音效（飞行风声、魔法嗡嗡声等） */
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> LoopingSound;
};
