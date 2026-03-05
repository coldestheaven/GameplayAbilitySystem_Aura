// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "AuraMeleeAttack.generated.h"

/**
 * 近战攻击技能
 *
 * 敌人 AI 使用的近战攻击技能基类
 * 继承自 UAuraDamageGameplayAbility，使用攻击蒙太奇触发伤害
 *
 * 工作原理：
 * - 激活时播放随机攻击蒙太奇（从 AttackMontages 数组中随机选择）
 * - 蒙太奇中的 AnimNotify 触发 GameplayEvent（如 Event.Montage.Attack.Weapon）
 * - 在事件回调中对目标应用 DamageEffectParams 中定义的伤害
 * - 蒙太奇播放完毕后自动结束技能
 *
 * 注意：此类目前没有额外的成员，所有逻辑在基类中实现
 * 子类可以重写以添加特殊的近战攻击效果（如毒素、击退等）
 */
UCLASS()
class AURA_API UAuraMeleeAttack : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()
	
};
