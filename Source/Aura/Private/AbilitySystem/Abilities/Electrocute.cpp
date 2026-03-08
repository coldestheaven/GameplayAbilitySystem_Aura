// Copyright Druid Mechanics


#include "AbilitySystem/Abilities/Electrocute.h"

/**
 * 获取技能描述（重写基类）
 * 
 * 实现流程：
 * 1. 计算等级对应的伤害值、法力消耗、冷却时间
 * 2. 根据等级返回不同格式的描述：
 *    - 等级 1：单目标闪电束描述
 *    - 等级 >1：多目标闪电束描述（可传播到额外目标）
 * 
 * @param Level 技能等级
 * @return 技能描述字符串（包含富文本格式）
 * 
 * 使用场景：
 * - 在技能菜单中显示技能描述
 * 
 * 注意：
 * - 伤害值通过 Damage 曲线根据等级计算
 * - 额外目标数量受 MaxNumShockTargets 限制（不会超过配置的最大数量）
 */
FString UElectrocute::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>ELECTROCUTE</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
			
			"<Default>Emits a beam of lightning, "
			"connecting with the target, repeatedly causing </>"

			// Damage
			"<Damage>%d</><Default> lightning damage with"
			" a chance to stun</>"),

			// Values
			Level,
			ManaCost,
			Cooldown,
			ScaledDamage);
	}
	else
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>ELECTROCUTE</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Addition Number of Shock Targets
			"<Default>Emits a beam of lightning, "
			"propagating to %d additional targets nearby, causing </>"

			// Damage
			"<Damage>%d</><Default> lightning damage with"
			" a chance to stun</>"),

			// Values
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(Level, MaxNumShockTargets - 1),
			ScaledDamage);		
	}
}

/**
 * 获取下一等级描述（重写基类）
 * 
 * 实现流程：
 * 1. 计算下一等级的伤害值、法力消耗、冷却时间
 * 2. 返回下一等级的技能描述（包含额外目标数量和伤害）
 * 
 * @param Level 下一等级
 * @return 下一等级描述字符串（包含富文本格式）
 * 
 * 使用场景：
 * - 在技能菜单中显示下一等级效果
 */
FString UElectrocute::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	return FString::Printf(TEXT(
			// Title
			"<Title>NEXT LEVEL:</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Addition Number of Shock Targets
			"<Default>Emits a beam of lightning, "
			"propagating to %d additional targets nearby, causing </>"

			// Damage
			"<Damage>%d</><Default> lightning damage with"
			" a chance to stun</>"),

			// Values
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(Level, MaxNumShockTargets - 1),
			ScaledDamage);	
}
