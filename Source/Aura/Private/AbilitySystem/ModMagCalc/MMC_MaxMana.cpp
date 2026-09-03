// Copyright Druid Mechanics


#include "AbilitySystem/ModMagCalc/MMC_MaxMana.h"

#include "AbilitySystem/AuraAttributeSet.h"
#include "Interaction/CombatInterface.h"

/**
 * 构造函数：注册需要捕获的属性
 * 
 * 实现流程：
 * 1. 配置 Intelligence（智力）属性的捕获定义：
 *    - 属性：Intelligence
 *    - 来源：Target（目标，即应用 GE 的角色）
 *    - 快照：false（不捕获快照值）
 * 2. 添加到 RelevantAttributesToCapture
 * 
 * 使用场景：
 * - ModMagCalc 创建时自动调用
 * 
 * 注意：
 * - 最大法力值 = 50 + 2.5 * Intelligence + 15 * Level
 * - Intelligence 从目标捕获（因为 GE 应用到目标自身）
 */
UMMC_MaxMana::UMMC_MaxMana()
{
	IntDef.AttributeToCapture = UAuraAttributeSet::GetIntelligenceAttribute();
	IntDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
	IntDef.bSnapshot = false;

	RelevantAttributesToCapture.Add(IntDef);
}

/**
 * 计算基础数值（重写基类）
 * 
 * 实现流程：
 * 1. 获取源和目标标签（用于属性捕获评估）
 * 2. 设置评估参数
 * 3. 捕获 Intelligence 属性值（从目标）
 * 4. 确保 Intelligence >= 0
 * 5. 获取角色等级（从 SourceObject，通过 CombatInterface）
 * 6. 计算最大法力值：50 + 2.5 * Intelligence + 15 * Level
 * 
 * @param Spec GameplayEffect 规格（包含捕获的属性值）
 * @return 计算的最大法力值
 * 
 * 使用场景：
 * - GE 应用时自动调用，计算最大法力值的修改量
 * 
 * 注意：
 * - 公式：基础值(50) + Intelligence加成(2.5*Intelligence) + 等级加成(15*Level)
 * - Intelligence 从目标捕获（因为 GE 应用到目标自身）
 * - 等级从源对象获取（通常是角色自身）
 */
float UMMC_MaxMana::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	// 获取源和目标标签（用于属性捕获评估）
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// 捕获 Intelligence 属性值（从目标）
	float Int = 0.f;
	GetCapturedAttributeMagnitude(IntDef, Spec, EvaluationParameters, Int);
	Int = FMath::Max<float>(Int, 0.f);

	// 获取角色等级（从源对象）
	int32 PlayerLevel = 1;
	if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>())
	{
		PlayerLevel = ICombatInterface::Execute_GetPlayerLevel(Spec.GetContext().GetSourceObject());
	}
	
	// 计算最大法力值：BaseValue + IntelligenceCoefficient * Intelligence + LevelCoefficient * Level
	// 系数已数据化（EditDefaultsOnly），可在蓝图 MMC 子类的默认值中调整
	return BaseValue + IntelligenceCoefficient * Int + LevelCoefficient * PlayerLevel;
}
