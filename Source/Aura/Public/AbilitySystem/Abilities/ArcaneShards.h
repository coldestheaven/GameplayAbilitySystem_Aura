// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "ArcaneShards.generated.h"

/**
 * 奥术碎片技能
 *
 * 在目标位置召唤多根奥术尖刺从地面刺出，造成范围伤害
 * 是奥术系法师的核心输出技能
 *
 * 特性：
 * - 在鼠标点击位置周围生成多个奥术尖刺（最多 MaxNumShards 个）
 * - 使用 PointCollection Actor 计算均匀分布的生成位置
 * - 每个尖刺独立造成伤害
 * - 升级时增加尖刺数量和伤害
 */
UCLASS()
class AURA_API UArcaneShards : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()
public:
	/**
	 * 获取当前等级的技能描述（重写基类）
	 * 包含：尖刺数量、伤害值、法力消耗、冷却时间
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

	/**
	 * 最大奥术尖刺数量
	 * 技能等级达到最大时生成此数量的尖刺
	 * 默认 11 个
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 MaxNumShards = 11;
};
