// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ExecCalc_Damage.generated.h"

/**
 * 伤害执行计算类
 *
 * GAS 的 GameplayEffect 执行计算（Execution Calculation）
 * 负责计算最终伤害值并写入 IncomingDamage 元属性
 *
 * 计算流程（Execute_Implementation）：
 * 1. 捕获来源和目标的属性（护甲、穿甲、暴击率等）
 * 2. 从 EffectContext 读取伤害类型标签
 * 3. 根据伤害类型查找目标的对应抗性属性
 * 4. 计算格挡（BlockChance）：有概率将伤害减半
 * 5. 计算护甲穿透（ArmorPenetration）：忽略目标护甲的百分比
 * 6. 计算有效护甲（Armor * (1 - ArmorPenetration)）：减少伤害
 * 7. 计算暴击（CriticalHitChance）：有概率触发暴击，伤害翻倍+额外伤害
 * 8. 应用抗性减伤（Resistance）：按百分比减少对应类型伤害
 * 9. 调用 DetermineDebuff 判断是否触发 Debuff
 * 10. 将最终伤害写入 IncomingDamage 元属性
 *
 * 系数来源：
 * - 所有计算系数从 CharacterClassInfo.DamageCalculationCoefficients 曲线表中读取
 * - 按角色等级缩放，高等级角色的护甲等属性效果更强
 */
UCLASS()
class AURA_API UExecCalc_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
public:
	UExecCalc_Damage();

	/**
	 * 判断并应用 Debuff 效果
	 * 根据 EffectContext 中的 Debuff 参数（类型、概率、伤害、持续时间、频率）
	 * 随机决定是否触发 Debuff，并将结果写回 EffectContext
	 * @param ExecutionParams    GE 执行参数（包含来源和目标的 ASC）
	 * @param Spec               GE 规格（包含 EffectContext）
	 * @param EvaluationParameters 属性评估参数
	 * @param InTagsToDefs       Tag 到属性捕获定义的映射表
	 */
	void DetermineDebuff(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	                     const FGameplayEffectSpec& Spec,
	                     FAggregatorEvaluateParameters EvaluationParameters,
	                     const TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition>& InTagsToDefs) const;

	/**
	 * 执行伤害计算（重写基类）
	 * 按照上述流程计算最终伤害，并通过 OutExecutionOutput 输出修改量
	 * @param ExecutionParams    GE 执行参数
	 * @param OutExecutionOutput 输出：属性修改量（IncomingDamage 的修改值）
	 */
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
