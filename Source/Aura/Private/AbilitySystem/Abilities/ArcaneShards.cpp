// Copyright Druid Mechanics


#include "AbilitySystem/Abilities/ArcaneShards.h"

/**
 * 获取技能描述（重写基类）
 * 
 * 实现流程：
 * 1. 计算等级对应的伤害值、法力消耗、冷却时间
 * 2. 根据等级返回不同格式的描述：
 *    - 等级 1：单发奥术碎片描述
 *    - 等级 >1：多发奥术碎片描述（数量 = Min(等级, MaxNumShards)）
 * 
 * @param Level 技能等级
 * @return 技能描述字符串（包含富文本格式）
 * 
 * 使用场景：
 * - 在技能菜单中显示技能描述
 * 
 * 注意：
 * - 奥术碎片会在目标位置造成范围伤害
 * - 碎片数量受 MaxNumShards 限制
 */
FString UArcaneShards::GetDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(
			// Title
			"<Title>ARCANE SHARDS</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
			
			"<Default>Summon a shard of arcane energy, "
			"causing radial arcane damage of  </>"

			// Damage
			"<Damage>%d</><Default> at the shard origin.</>"),

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
			"<Title>ARCANE SHARDS</>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Addition Number of Shock Targets
			"<Default>Summon %d shards of arcane energy, causing radial arcane damage of </>"

			// Damage
			"<Damage>%d</><Default> at the shard origins.</>"),

			// Values
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(Level, MaxNumShards),
			ScaledDamage);		
	}
}

/**
 * 获取下一等级描述（重写基类）
 * 
 * 实现流程：
 * 1. 计算下一等级的伤害值、法力消耗、冷却时间
 * 2. 返回下一等级的技能描述（包含碎片数量和伤害）
 * 
 * @param Level 下一等级
 * @return 下一等级描述字符串（包含富文本格式）
 */
FString UArcaneShards::GetNextLevelDescription(int32 Level)
{
	const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float Cooldown = GetCooldown(Level);
	
	return FString::Printf(TEXT(
			// Title
			"<Title>NEXT LEVEL: </>\n\n"

			// Level
			"<Small>Level: </><Level>%d</>\n"
			// ManaCost
			"<Small>ManaCost: </><ManaCost>%.1f</>\n"
			// Cooldown
			"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"

			// Addition Number of Shock Targets
			"<Default>Summon %d shards of arcane energy, causing radial arcane damage of </>"

			// Damage
			"<Damage>%d</><Default> at the shard origins.</>"),

			// Values
			Level,
			ManaCost,
			Cooldown,
			FMath::Min(Level, MaxNumShards),
			ScaledDamage);	
}
