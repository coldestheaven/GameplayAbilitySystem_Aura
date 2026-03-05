// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "AuraEffectActor.generated.h"

class UAbilitySystemComponent;
class UGameplayEffect;

/**
 * GameplayEffect 应用策略枚举
 * 决定 GE 在什么时机被应用到目标
 */
UENUM(BlueprintType)
enum class EEffectApplicationPolicy : uint8
{
	ApplyOnOverlap,     // 目标进入重叠区域时立即应用 GE
	ApplyOnEndOverlap,  // 目标离开重叠区域时应用 GE
	DoNotApply          // 不自动应用（由蓝图手动控制）
};

/**
 * GameplayEffect 移除策略枚举
 * 决定持续型 GE（Infinite Duration）何时被移除
 */
UENUM(BlueprintType)
enum class EEffectRemovalPolicy : uint8
{
	RemoveOnEndOverlap, // 目标离开重叠区域时移除 GE（适用于持续增益区域）
	DoNotRemove         // 不自动移除（GE 持续到自然结束或手动移除）
};

/**
 * Aura 效果 Actor 基类
 *
 * 场景中可以对角色施加 GameplayEffect 的 Actor（如药水、增益区域、陷阱等）
 *
 * 功能：
 * - 支持三种类型的 GE：即时（Instant）、持续（Duration）、无限（Infinite）
 * - 每种 GE 可独立配置应用策略（进入/离开时应用）
 * - 无限 GE 支持配置移除策略（离开时移除）
 * - 支持正弦波上下浮动动画（SinusoidalMovement）
 * - 支持自旋动画（Rotates）
 * - 可配置是否对敌人生效（bApplyEffectsToEnemies）
 *
 * 使用示例（蓝图子类）：
 *   // 创建一个生命药水：
 *   InstantGameplayEffectClass = GE_HealInstant
 *   InstantEffectApplicationPolicy = ApplyOnOverlap
 *   bDestroyOnEffectApplication = true（拾取后销毁）
 *
 *   // 创建一个持续增益区域：
 *   InfiniteGameplayEffectClass = GE_SpeedBoost
 *   InfiniteEffectApplicationPolicy = ApplyOnOverlap
 *   InfiniteEffectRemovalPolicy = RemoveOnEndOverlap
 */
UCLASS()
class AURA_API AAuraEffectActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AAuraEffectActor();

	/** 每帧更新：处理浮动动画和旋转动画 */
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	/** 当前帧计算出的目标位置（由 ItemMovement 更新，蓝图可读写） */
	UPROPERTY(BlueprintReadWrite)
	FVector CalculatedLocation;

	/** 当前帧计算出的目标旋转（由 ItemMovement 更新，蓝图可读写） */
	UPROPERTY(BlueprintReadWrite)
	FRotator CalculatedRotation;

	/**
	 * 是否启用自旋动画
	 * true：Actor 每帧按 RotationRate 旋转
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Movement")
	bool bRotates = false;

	/**
	 * 旋转速率（度/秒）
	 * 仅在 bRotates=true 时生效
	 * 默认 45 度/秒
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Movement")
	float RotationRate = 45.f;

	/**
	 * 是否启用正弦波浮动动画
	 * true：Actor 在 Z 轴上做正弦波上下浮动
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Movement")
	bool bSinusoidalMovement = false;

	/**
	 * 启动正弦波浮动动画（蓝图可调用）
	 * 记录初始位置，开始每帧计算正弦波偏移
	 */
	UFUNCTION(BlueprintCallable)
	void StartSinusoidalMovement();

	/**
	 * 启动旋转动画（蓝图可调用）
	 * 设置 bRotates=true，开始每帧旋转
	 */
	UFUNCTION(BlueprintCallable)
	void StartRotation();
	
	/**
	 * 正弦波振幅（单位：cm）
	 * 控制浮动动画的上下幅度
	 * 默认 1 cm
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Movement")
	float SineAmplitude = 1.f;

	/**
	 * 正弦波周期常数
	 * 控制浮动动画的速度（值越大浮动越快）
	 * 默认 1.0
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Movement")
	float SinePeriodConstant = 1.f; 

	/** 初始位置（StartSinusoidalMovement 时记录，用于计算正弦波偏移的基准） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup Movement")
	FVector InitialLocation;

	/**
	 * 将 GameplayEffect 应用到目标（蓝图可调用）
	 * 获取目标的 ASC，创建 GE 规格并应用
	 * 对于 Infinite GE，将句柄存储到 ActiveEffectHandles 以便后续移除
	 * @param TargetActor          目标 Actor（必须有 ASC）
	 * @param GameplayEffectClass  要应用的 GE 类
	 */
	UFUNCTION(BlueprintCallable)
	void ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass);

	/**
	 * 目标进入重叠区域时调用（蓝图可调用）
	 * 根据各 GE 的应用策略，决定是否应用对应的 GE
	 * @param TargetActor 进入重叠区域的 Actor
	 */
	UFUNCTION(BlueprintCallable)
	void OnOverlap(AActor* TargetActor);

	/**
	 * 目标离开重叠区域时调用（蓝图可调用）
	 * 根据各 GE 的应用/移除策略，决定是否应用或移除对应的 GE
	 * @param TargetActor 离开重叠区域的 Actor
	 */
	UFUNCTION(BlueprintCallable)
	void OnEndOverlap(AActor* TargetActor);

	/**
	 * 应用 GE 后是否销毁此 Actor
	 * true：应用 GE 后立即销毁（适用于一次性拾取物，如药水）
	 * false：保持存在（适用于持续增益区域）
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	bool bDestroyOnEffectApplication = false;

	/**
	 * 是否对敌人也应用效果
	 * false（默认）：只对玩家应用效果
	 * true：对所有角色（包括敌人）应用效果（适用于陷阱等）
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	bool bApplyEffectsToEnemies = false;
	
	/**
	 * 即时 GameplayEffect 类（Instant Duration）
	 * 立即修改属性值（如立即恢复生命值），不持续
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TSubclassOf<UGameplayEffect> InstantGameplayEffectClass;

	/**
	 * 即时 GE 的应用策略
	 * 默认 DoNotApply（不自动应用）
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	EEffectApplicationPolicy InstantEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;

	/**
	 * 持续 GameplayEffect 类（Has Duration）
	 * 在指定时间内持续修改属性（如短暂加速），时间到后自动移除
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TSubclassOf<UGameplayEffect> DurationGameplayEffectClass;

	/**
	 * 持续 GE 的应用策略
	 * 默认 DoNotApply
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	EEffectApplicationPolicy DurationEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;

	/**
	 * 无限 GameplayEffect 类（Infinite Duration）
	 * 持续修改属性直到被手动移除（如持续增益区域的效果）
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	TSubclassOf<UGameplayEffect> InfiniteGameplayEffectClass;

	/**
	 * 无限 GE 的应用策略
	 * 默认 DoNotApply
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	EEffectApplicationPolicy InfiniteEffectApplicationPolicy = EEffectApplicationPolicy::DoNotApply;

	/**
	 * 无限 GE 的移除策略
	 * 默认 RemoveOnEndOverlap（目标离开时移除）
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Applied Effects")
	EEffectRemovalPolicy InfiniteEffectRemovalPolicy = EEffectRemovalPolicy::RemoveOnEndOverlap;

	/**
	 * 已激活的无限 GE 句柄映射表
	 * Key: 激活的 GE 句柄（用于后续移除）
	 * Value: 目标的 ASC（用于调用 RemoveActiveGameplayEffect）
	 * 当目标离开重叠区域时，通过此映射找到对应的 GE 并移除
	 */
	TMap<FActiveGameplayEffectHandle, UAbilitySystemComponent*> ActiveEffectHandles;

	/**
	 * Actor 等级（影响 GE 的 ScalableFloat 曲线取值）
	 * 默认 1.0，可在 Details 面板中调整以改变效果强度
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Applied Effects")
	float ActorLevel = 1.f;

private:
	/** 累计运行时间（用于正弦波动画的时间参数） */
	float RunningTime = 0.f;

	/**
	 * 每帧更新物品移动（浮动和旋转动画）
	 * 计算正弦波偏移和旋转角度，更新 CalculatedLocation 和 CalculatedRotation
	 * @param DeltaTime 帧时间间隔
	 */
	void ItemMovement(float DeltaTime);
};