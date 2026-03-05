// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "AuraFireBlast.generated.h"

class AAuraFireBall;

/**
 * 火焰爆炸技能
 *
 * 向四周均匀发射多个火球（AAuraFireBall），火球飞出后返回施法者
 * 去程和回程均可造成伤害，是一个高伤害的范围技能
 *
 * 特性：
 * - 向四周均匀发射 NumFireBalls 个火球（360度均匀分布）
 * - 火球飞出一段距离后自动返回施法者（通过蓝图时间轴控制）
 * - 去程命中敌人造成伤害，回程命中也造成伤害
 * - 升级时增加火球数量和伤害
 *
 * 使用流程：
 *   ActivateAbility → SpawnFireBalls → 火球飞出 → 返回 → 结束技能
 */
UCLASS()
class AURA_API UAuraFireBlast : public UAuraDamageGameplayAbility
{
	GENERATED_BODY()
public:
	/**
	 * 获取当前等级的技能描述（重写基类）
	 * 包含：火球数量、伤害值、法力消耗、冷却时间
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
	 * 生成所有火球（蓝图可调用）
	 * 在施法者周围均匀生成 NumFireBalls 个火球，设置伤害参数后发射
	 * @return 生成的所有火球 Actor 数组（蓝图可用于后续控制，如触发返回）
	 */
	UFUNCTION(BlueprintCallable)
	TArray<AAuraFireBall*> SpawnFireBalls();

protected:
	/**
	 * 火球数量
	 * 每次激活技能时生成的火球数量，均匀分布在 360 度范围内
	 * 默认 12 个
	 */
	UPROPERTY(EditDefaultsOnly, Category = "FireBlast")
	int32 NumFireBalls = 12;

private:
	/**
	 * 火球 Actor 类
	 * 在 Details 面板中指定要生成的火球类型（通常为 BP_AuraFireBall）
	 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AAuraFireBall> FireBallClass;
};
