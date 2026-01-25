// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "AbilitySystem/Data/AbilityConfigData.h"
#include "ConfigurableAbility.generated.h"

/**
 * 可配置技能类
 * 
 * 通过数据资产配置技能行为，无需编写C++代码
 * 支持多种动作类型的组合和序列执行
 */
UCLASS()
class AURA_API UConfigurableAbility : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()
	
public:
	UConfigurableAbility();
	
	// 技能配置数据
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Config")
	TObjectPtr<UAbilityConfigData> AbilityConfig;
	
	// 获取技能描述
	virtual FString GetDescription(int32 Level) override;
	virtual FString GetNextLevelDescription(int32 Level) override;
	
protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData
	) override;
	
	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled
	) override;
	
private:
	// 执行技能动作序列
	void ExecuteNextAction();
	
	// 执行单个动作
	void ExecuteAction(const FAbilityActionConfig& ActionConfig);
	
	// 具体动作实现
	void ExecuteSpawnProjectile(const FAbilityActionConfig& ActionConfig);
	void ExecuteApplyEffect(const FAbilityActionConfig& ActionConfig);
	void ExecutePlayMontage(const FAbilityActionConfig& ActionConfig);
	void ExecuteSpawnBeam(const FAbilityActionConfig& ActionConfig);
	void ExecuteAreaOfEffect(const FAbilityActionConfig& ActionConfig);
	void ExecuteTeleport(const FAbilityActionConfig& ActionConfig);
	void ExecuteWaitForEvent(const FAbilityActionConfig& ActionConfig);
	
	// 播放视觉效果
	void PlayVisualEffect(const FVisualEffectConfig& VisualConfig, const FVector& Location, const FRotator& Rotation);
	
	// 获取目标位置
	FVector GetTargetLocation() const;
	
	// 当前执行的动作索引
	int32 CurrentActionIndex;
	
	// 动作定时器
	FTimerHandle ActionTimerHandle;
	
	// 目标位置（从鼠标光标获取）
	FVector CachedTargetLocation;
	
	// 是否已缓存目标位置
	bool bHasCachedTarget;
};
