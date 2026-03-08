// Copyright Druid Mechanics


#include "AbilitySystem/Abilities/AuraGameplayAbility.h"

#include "AbilitySystem/AuraAttributeSet.h"

/**
 * 获取技能描述（基类默认实现，子类应重写）
 * 
 * @param Level 技能等级
 * @return 技能描述字符串（包含富文本格式）
 * 
 * 使用场景：
 * - 在技能菜单中显示技能描述
 * - 由 AuraASC::GetDescriptionsByAbilityTag 调用
 * 
 * 注意：
 * - 这是基类默认实现，返回占位文本
 * - 子类应重写此函数返回实际的技能描述
 */
FString UAuraGameplayAbility::GetDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>%s, </><Level>%d</>"), L"Default Ability Name - LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum LoremIpsum", Level);
}

/**
 * 获取下一等级描述（基类默认实现，子类应重写）
 * 
 * @param Level 下一等级
 * @return 下一等级描述字符串（包含富文本格式）
 * 
 * 使用场景：
 * - 在技能菜单中显示下一等级效果
 * - 由 AuraASC::GetDescriptionsByAbilityTag 调用
 * 
 * 注意：
 * - 这是基类默认实现，返回占位文本
 * - 子类应重写此函数返回实际的下一等级描述
 */
FString UAuraGameplayAbility::GetNextLevelDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>Next Level: </><Level>%d</> \n<Default>Causes much more damage. </>"), Level);
}

/**
 * 获取锁定描述（技能未解锁时的提示）
 * 
 * @param Level 解锁所需等级
 * @return 锁定描述字符串（包含富文本格式）
 * 
 * 使用场景：
 * - 在技能菜单中显示未解锁技能的提示
 * - 由 AuraASC::GetDescriptionsByAbilityTag 调用
 */
FString UAuraGameplayAbility::GetLockedDescription(int32 Level)
{
	return FString::Printf(TEXT("<Default>Spell Locked Until Level: %d</>"), Level);
}

/**
 * 获取技能的法力消耗
 * 
 * 实现流程：
 * 1. 获取技能的 Cost GE（消耗效果）
 * 2. 遍历 Cost GE 的所有 Modifier
 * 3. 找到法力值属性的 Modifier
 * 4. 从 ModifierMagnitude 获取静态数值（根据等级）
 * 
 * @param InLevel 技能等级
 * @return 法力消耗值
 * 
 * 使用场景：
 * - 在技能菜单中显示技能消耗
 * - 判断是否可以释放技能
 * 
 * 注意：
 * - 法力消耗在 Cost GE 中配置（Modifier 模式为 Override）
 */
float UAuraGameplayAbility::GetManaCost(float InLevel) const
{
	float ManaCost = 0.f;
	if (const UGameplayEffect* CostEffect = GetCostGameplayEffect())
	{
		for (FGameplayModifierInfo Mod : CostEffect->Modifiers)
		{
			if (Mod.Attribute == UAuraAttributeSet::GetManaAttribute())
			{
				Mod.ModifierMagnitude.GetStaticMagnitudeIfPossible(InLevel, ManaCost);
				break;
			}
		}
	}
	return ManaCost;
}

/**
 * 获取技能的冷却时间
 * 
 * 实现流程：
 * 1. 获取技能的 Cooldown GE（冷却效果）
 * 2. 从 Cooldown GE 的 DurationMagnitude 获取静态数值（根据等级）
 * 
 * @param InLevel 技能等级
 * @return 冷却时间（秒）
 * 
 * 使用场景：
 * - 在技能菜单中显示技能冷却时间
 * - 判断技能是否在冷却中
 * 
 * 注意：
 * - 冷却时间在 Cooldown GE 的 Duration 中配置
 */
float UAuraGameplayAbility::GetCooldown(float InLevel) const
{
	float Cooldown = 0.f;
	if (const UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect())
	{
		CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(InLevel, Cooldown);
	}
	return Cooldown;
}
