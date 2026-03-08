// Copyright Druid Mechanics


#include "AbilitySystem/AuraAbilitySystemGlobals.h"

#include "AuraAbilityTypes.h"

/**
 * 分配自定义 GameplayEffectContext（重写基类）
 * 
 * 实现流程：
 * 1. 创建并返回 FAuraGameplayEffectContext（自定义的 Context 类）
 * 
 * @return 分配的 GameplayEffectContext 指针
 * 
 * 使用场景：
 * - GAS 需要创建 EffectContext 时自动调用
 * 
 * 注意：
 * - FAuraGameplayEffectContext 扩展了标准 Context，添加了自定义数据（如伤害类型、Debuff 信息等）
 * - 此函数确保所有 GE 都使用自定义 Context
 */
FGameplayEffectContext* UAuraAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FAuraGameplayEffectContext();
}
