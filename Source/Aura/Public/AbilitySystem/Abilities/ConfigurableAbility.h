// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "AbilitySystem/Data/AbilityConfigData.h"
#include "ConfigurableAbility.generated.h"

/**
 * 可配置技能类
 *
 * 通过数据资产（UAbilityConfigData）配置技能行为，无需编写 C++ 代码
 * 支持多种动作类型的组合和序列执行，是一个数据驱动的技能系统
 *
 * 支持的动作类型（FAbilityActionConfig.ActionType）：
 * - SpawnProjectile：生成投射物
 * - ApplyEffect：应用 GameplayEffect
 * - PlayMontage：播放动画蒙太奇
 * - SpawnBeam：生成光束特效
 * - AreaOfEffect：范围效果
 * - Teleport：传送
 * - WaitForEvent：等待 GameplayEvent
 *
 * 使用方式：
 *   1. 创建 UAbilityConfigData 数据资产，配置动作序列
 *   2. 创建 UConfigurableAbility 的蓝图子类
 *   3. 将数据资产赋值给 AbilityConfig
 *   4. 技能激活时自动按顺序执行配置的动作
 */
UCLASS()
class AURA_API UConfigurableAbility : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()
	
public:
	UConfigurableAbility();
	
	/**
	 * 技能配置数据资产
	 * 定义技能的动作序列、视觉效果、伤害参数等
	 * 在 Details 面板中指定对应的 UAbilityConfigData 资产
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Config")
	TObjectPtr<UAbilityConfigData> AbilityConfig;
	
	/**
	 * 获取当前等级的技能描述（重写基类）
	 * 从 AbilityConfig 中读取描述模板，填入当前等级的数值
	 * @param Level 技能等级
	 * @return 格式化的描述字符串
	 */
	virtual FString GetDescription(int32 Level) override;

	/**
	 * 获取下一等级的技能描述（重写基类）
	 * @param Level 当前等级
	 * @return 下一等级的描述字符串
	 */
	virtual FString GetNextLevelDescription(int32 Level) override;
	
protected:
	/**
	 * 技能激活（重写基类）
	 * 初始化动作索引，获取鼠标目标位置，开始执行动作序列
	 */
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
	
	/**
	 * 技能结束（重写基类）
	 * 清理定时器，重置状态
	 */
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;
	
private:
	/**
	 * 执行下一个动作
	 * 按顺序从 AbilityConfig.Actions 数组中取出下一个动作并执行
	 * 如果所有动作执行完毕，结束技能
	 */
	void ExecuteNextAction();
	
	/**
	 * 执行单个动作
	 * 根据 ActionConfig.ActionType 分发到对应的具体执行函数
	 * @param ActionConfig 要执行的动作配置
	 */
	void ExecuteAction(const FAbilityActionConfig& ActionConfig);
	
	/** 执行生成投射物动作 */
	void ExecuteSpawnProjectile(const FAbilityActionConfig& ActionConfig);

	/** 执行应用 GameplayEffect 动作 */
	void ExecuteApplyEffect(const FAbilityActionConfig& ActionConfig);

	/** 执行播放蒙太奇动作 */
	void ExecutePlayMontage(const FAbilityActionConfig& ActionConfig);

	/** 执行生成光束特效动作 */
	void ExecuteSpawnBeam(const FAbilityActionConfig& ActionConfig);

	/** 执行范围效果动作 */
	void ExecuteAreaOfEffect(const FAbilityActionConfig& ActionConfig);

	/** 执行传送动作 */
	void ExecuteTeleport(const FAbilityActionConfig& ActionConfig);

	/** 执行等待 GameplayEvent 动作 */
	void ExecuteWaitForEvent(const FAbilityActionConfig& ActionConfig);
	
	/**
	 * 播放视觉效果（粒子特效、音效等）
	 * @param VisualConfig 视觉效果配置
	 * @param Location     播放位置
	 * @param Rotation     播放旋转
	 */
	void PlayVisualEffect(const FVisualEffectConfig& VisualConfig, const FVector& Location, const FRotator& Rotation);
	
	/**
	 * 获取目标位置
	 * 优先使用缓存的鼠标目标位置，如果没有则使用施法者前方位置
	 * @return 目标世界坐标
	 */
	FVector GetTargetLocation() const;
	
	/** 当前执行的动作索引（从 0 开始，每执行一个动作递增） */
	int32 CurrentActionIndex;
	
	/** 动作间延迟定时器句柄（用于实现动作之间的时间间隔） */
	FTimerHandle ActionTimerHandle;
	
	/** 缓存的目标位置（从鼠标光标射线检测获取，在技能激活时记录） */
	FVector CachedTargetLocation;
	
	/** 是否已缓存目标位置（防止重复射线检测） */
	bool bHasCachedTarget;
};
