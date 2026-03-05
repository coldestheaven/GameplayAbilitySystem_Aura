// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraBeamSpell.h"
#include "Electrocute.generated.h"

/**
 * 电击技能（链式闪电）
 *
 * 继承自 UAuraBeamSpell，是闪电系法师的核心技能
 * 向主目标发射闪电光束，并可以链式跳转到附近的其他敌人
 *
 * 特性：
 * - 对主目标持续造成闪电伤害
 * - 闪电可以跳转到主目标附近的最多 MaxNumShockTargets 个敌人
 * - 命中目标有概率触发眩晕 Debuff（Stun）
 * - 升级时增加伤害和跳转目标数量
 *
 * 与 UAuraBeamSpell 的关系：
 * - 继承了所有光束技能的基础逻辑（目标追踪、链式跳转）
 * - 只重写了描述文本函数，其余逻辑在基类中实现
 */
UCLASS()
class AURA_API UElectrocute : public UAuraBeamSpell
{
	GENERATED_BODY()
public:
	/**
	 * 获取当前等级的技能描述（重写基类）
	 * 包含：伤害值、跳转目标数、法力消耗、冷却时间
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
};
