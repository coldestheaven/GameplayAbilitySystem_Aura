// Copyright Druid Mechanics


#include "AbilitySystem/ModMagCalc/MMC_MaxHealth.h"

#include "AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"

/**
 * 构造函数：注册需要捕获的属性
 * 
 * 实现流程：
 * 1. 配置 Vigor（活力）属性的捕获定义：
 *    - 属性：Vigor
 *    - 来源：Target（目标，即应用 GE 的角色）
 *    - 快照：false（不捕获快照值）
 * 2. 添加到 RelevantAttributesToCapture
 * 
 * 使用场景：
 * - ModMagCalc 创建时自动调用
 * 
 * 注意：
 * - 最大生命值 = 80 + 2.5 * Vigor + 10 * Level
 * - Vigor 从目标捕获（因为 GE 应用到目标自身）
 */
UMMC_MaxHealth::UMMC_MaxHealth()
{
	VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();
	VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	VigorDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(VigorDef);
}

/**
 * 计算基础数值（重写基类）
 * 
 * 实现流程：
 * 1. 获取源和目标标签（用于属性捕获评估）
 * 2. 设置评估参数
 * 3. 捕获 Vigor 属性值（从目标）
 * 4. 确保 Vigor >= 0
 * 5. 获取角色等级（从 SourceObject，通过 CombatInterface）
 * 6. 计算最大生命值：80 + 2.5 * Vigor + 10 * Level
 * 
 * @param Spec GameplayEffect 规格（包含捕获的属性值）
 * @return 计算的最大生命值
 * 
 * 使用场景：
 * - GE 应用时自动调用，计算最大生命值的修改量
 * 
 * 注意：
 * - 公式：基础值(80) + Vigor加成(2.5*Vigor) + 等级加成(10*Level)
 * - Vigor 从目标捕获（因为 GE 应用到目标自身）
 * - 等级从源对象获取（通常是角色自身）
 */
float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// 获取源和目标标签（用于属性捕获评估）
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// 捕获 Vigor 属性值（从目标）
	float Vigor = 0.f;
	GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluationParameters, Vigor);
	Vigor = FMath::Max<float>(Vigor, 0.f);

	// 获取角色等级（从源对象）
	int32 PlayerLevel = 1;
	if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>())
	{
		PlayerLevel = ICombatInterface::Execute_GetPlayerLevel(Spec.GetContext().GetSourceObject());
	}

	// 计算最大生命值：BaseValue + VigorCoefficient * Vigor + LevelCoefficient * Level
	// 系数已数据化（EditDefaultsOnly），可在蓝图 MMC 子类的默认值中调整
	return BaseValue + VigorCoefficient * Vigor + LevelCoefficient * PlayerLevel;
}
