// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "AuraAbilitySystemGlobals.generated.h"

/**
 * Aura 能力系统全局配置类
 *
 * 继承自 UAbilitySystemGlobals，重写 AllocGameplayEffectContext
 * 使 GAS 在创建 GE 上下文时使用 FAuraGameplayEffectContext（自定义上下文）
 * 而不是默认的 FGameplayEffectContext
 *
 * 配置方式（DefaultGame.ini）：
 *   [/Script/GameplayAbilities.AbilitySystemGlobals]
 *   AbilitySystemGlobalsClassName=/Script/Aura.AuraAbilitySystemGlobals
 *
 * 重要性：
 * - 如果不配置此类，GAS 将使用默认的 FGameplayEffectContext
 * - 默认上下文不包含 Aura 自定义的字段（暴击、格挡、Debuff 参数等）
 * - 配置后，所有 GE 上下文都会使用 FAuraGameplayEffectContext
 */
UCLASS()
class AURA_API UAuraAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

	/**
	 * 分配 GameplayEffect 上下文（重写基类）
	 * 返回 FAuraGameplayEffectContext 实例（而非默认的 FGameplayEffectContext）
	 * GAS 在每次需要创建 GE 上下文时调用此函数
	 * @return 新分配的 FAuraGameplayEffectContext 指针（调用者负责管理生命周期）
	 */
	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
