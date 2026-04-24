// Copyright Druid Mechanics


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "AuraAbilityTypes.h"
#include "GameplayTags/AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/AuraEffectContextLibrary.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Camera/CameraShakeSourceActor.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/GameplayStatics.h"

// 属性捕获定义结构体：声明并定义所有需要在伤害计算中捕获的属性
struct AuraDamageStatics
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage);
	
	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(LightningResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArcaneResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance);
	
	AuraDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, CriticalHitDamage, Source, false);

		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, FireResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, LightningResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, ArcaneResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, PhysicalResistance, Target, false);
	}
};

// 单例访问函数：返回全局唯一的 AuraDamageStatics 实例
static const AuraDamageStatics& DamageStatics()
{
	static AuraDamageStatics DStatics;
	return DStatics;
}

/**
 * 构造函数：注册需要捕获的属性
 *
 * 实现流程：
 * 1. 注册目标属性：护甲、格挡率、暴击抗性、各元素抗性
 * 2. 注册源属性：护甲穿透、暴击率、暴击伤害
 *
 * 注意：
 * - 这些属性会在 Execute_Implementation 中被捕获并用于伤害计算
 * - 目标属性用于防御计算，源属性用于攻击计算
 */
UExecCalc_Damage::UExecCalc_Damage()
{
	// 目标属性（防御相关）
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitResistanceDef);
	
	// 源属性（攻击相关）
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitDamageDef);

	// 目标抗性属性（元素抗性）
	RelevantAttributesToCapture.Add(DamageStatics().FireResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().LightningResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArcaneResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalResistanceDef);
}

/**
 * 判定 Debuff（负面效果）是否触发
 *
 * 实现流程：
 * 1. 遍历所有伤害类型与对应 Debuff 的映射
 * 2. 获取该伤害类型的实际伤害值，若无此类型伤害则跳过
 * 3. 计算有效 Debuff 触发率：源触发率 * (100 - 目标抗性) / 100
 * 4. 随机判定是否触发，若触发则将 Debuff 信息写入 EffectContext
 *
 * @param ExecutionParams  执行参数（用于捕获目标抗性属性）
 * @param Spec             GameplayEffect 规格（用于读取 SetByCaller 数值）
 * @param EvaluationParameters 属性评估参数（源/目标标签）
 * @param InTagsToDefs     标签到属性捕获定义的映射表
 */
void UExecCalc_Damage::DetermineDebuff(const FGameplayEffectCustomExecutionParameters& ExecutionParams, const FGameplayEffectSpec& Spec, FAggregatorEvaluateParameters EvaluationParameters,
						 const TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition>& InTagsToDefs) const
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

	for (TTuple<FGameplayTag, FGameplayTag> Pair : GameplayTags.DamageTypesToDebuffs)
	{
		const FGameplayTag& DamageType = Pair.Key;
		const FGameplayTag& DebuffType = Pair.Value;
		const float TypeDamage = Spec.GetSetByCallerMagnitude(DamageType, false, -1.f);
		if (TypeDamage > -.5f) // 浮点精度容差 0.5，避免误判
		{
			// 判定是否触发 Debuff
			const float SourceDebuffChance = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Chance, false, -1.f);

			float TargetDebuffResistance = 0.f;
			const FGameplayTag& ResistanceTag = GameplayTags.DamageTypesToResistances[DamageType];
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(InTagsToDefs[ResistanceTag], EvaluationParameters, TargetDebuffResistance);
			TargetDebuffResistance = FMath::Max<float>(TargetDebuffResistance, 0.f);
			const float EffectiveDebuffChance = SourceDebuffChance * ( 100 - TargetDebuffResistance ) / 100.f;
			const bool bDebuff = FMath::RandRange(1, 100) < EffectiveDebuffChance;
			if (bDebuff)
			{
				FGameplayEffectContextHandle ContextHandle = Spec.GetContext();

				UAuraEffectContextLibrary::SetIsSuccessfulDebuff(ContextHandle, true);
				UAuraEffectContextLibrary::SetDamageType(ContextHandle, DamageType);

				const float DebuffDamage = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Damage, false, -1.f);
				const float DebuffDuration = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Duration, false, -1.f);
				const float DebuffFrequency = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Frequency, false, -1.f);

				UAuraEffectContextLibrary::SetDebuffDamage(ContextHandle, DebuffDamage);
				UAuraEffectContextLibrary::SetDebuffDuration(ContextHandle, DebuffDuration);
				UAuraEffectContextLibrary::SetDebuffFrequency(ContextHandle, DebuffFrequency);
			}
		}
	}
}

/**
 * 伤害计算执行函数（GAS 自定义执行的核心）
 *
 * 实现流程：
 * 1. 构建标签到属性捕获定义的映射表
 * 2. 获取源和目标 ASC、Avatar Actor、等级
 * 3. 获取 EffectSpec 和 ContextHandle
 * 4. 设置评估参数（源/目标标签）
 * 5. 判定 Debuff（DetermineDebuff）
 * 6. 计算基础伤害（应用抗性减免，范围伤害走 ApplyRadialDamageWithFalloff 并通过委托回调获取衰减后的实际值）
 * 7. 判定格挡（BlockChance），格挡成功则伤害减半
 * 8. 计算护甲减免（考虑护甲穿透系数）
 * 9. 判定暴击（CriticalHit），暴击则伤害翻倍并叠加暴击伤害加成
 * 10. 将最终伤害写入 IncomingDamage 中间属性（由 PostGameplayEffectExecute 转换到 Health）
 *
 * @param ExecutionParams    执行参数（包含源/目标 ASC、属性捕获等）
 * @param OutExecutionOutput 输出参数（最终伤害值写入此处）
 *
 * 伤害计算公式：
 * 1. 基础伤害     = SetByCaller 伤害值 * (100 - 抗性) / 100
 * 2. 格挡后伤害   = 基础伤害 / 2（格挡成功时）
 * 3. 有效护甲     = 目标护甲 * (100 - 护甲穿透 * 穿透系数) / 100
 * 4. 护甲减免伤害 = 伤害 * (100 - 有效护甲 * 护甲系数) / 100
 * 5. 最终伤害     = 2 × 护甲减免后伤害 + 暴击伤害加成（暴击时）
 *
 * 注意：
 * - 此函数在服务端执行，确保伤害计算的权威性
 * - 所有属性捕获均在此函数中进行
 * - 伤害值最终通过 OutExecutionOutput 应用到 IncomingDamage 属性
 */
void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                              FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	// 构建标签到属性捕获定义的映射表（静态变量，只初始化一次）
	static TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;
	if (TagsToCaptureDefs.IsEmpty())
	{
		const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_Armor, DamageStatics().ArmorDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_BlockChance, DamageStatics().BlockChanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_ArmorPenetration, DamageStatics().ArmorPenetrationDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitChance, DamageStatics().CriticalHitChanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitResistance, DamageStatics().CriticalHitResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Secondary_CriticalHitDamage, DamageStatics().CriticalHitDamageDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Arcane, DamageStatics().ArcaneResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Fire, DamageStatics().FireResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Lightning, DamageStatics().LightningResistanceDef);
		TagsToCaptureDefs.Add(Tags.Attributes_Resistance_Physical, DamageStatics().PhysicalResistanceDef);
	}
	const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
	
	// 获取源和目标 ASC 和 Avatar Actor
	const UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	AActor* SourceAvatar = SourceASC ? SourceASC->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetASC ? TargetASC->GetAvatarActor() : nullptr;

	// 获取源和目标等级（用于曲线计算）
	int32 SourcePlayerLevel = 1;
	if (SourceAvatar->Implements<UCombatInterface>())
	{
		SourcePlayerLevel = ICombatInterface::Execute_GetPlayerLevel(SourceAvatar);
	}
	int32 TargetPlayerLevel = 1;
	if (TargetAvatar->Implements<UCombatInterface>())
	{
		TargetPlayerLevel = ICombatInterface::Execute_GetPlayerLevel(TargetAvatar);
	}

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();

	// 获取源和目标标签（用于属性捕获评估）
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// 步骤 1：判定 Debuff（负面效果）
	DetermineDebuff(ExecutionParams, Spec, EvaluationParameters, TagsToCaptureDefs);

	// 步骤 2：计算基础伤害（应用抗性减免）
	float Damage = 0.f;
	for (const TTuple<FGameplayTag, FGameplayTag>& Pair  : FAuraGameplayTags::Get().DamageTypesToResistances)
	{
		const FGameplayTag DamageTypeTag = Pair.Key;
		const FGameplayTag ResistanceTag = Pair.Value;
		
		checkf(TagsToCaptureDefs.Contains(ResistanceTag), TEXT("TagsToCaptureDefs doesn't contain Tag: [%s] in ExecCalc_Damage"), *ResistanceTag.ToString());
		const FGameplayEffectAttributeCaptureDefinition CaptureDef = TagsToCaptureDefs[ResistanceTag];

		// 获取该类型伤害的基础值（从 SetByCaller 获取）
		float DamageTypeValue = Spec.GetSetByCallerMagnitude(Pair.Key, false);
		if (DamageTypeValue <= 0.f)
		{
			continue;
		}
		
		// 捕获目标对应抗性值
		float Resistance = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CaptureDef, EvaluationParameters, Resistance);
		Resistance = FMath::Clamp(Resistance, 0.f, 100.f);

		// 应用抗性减免：伤害 = 基础伤害 * (100 - 抗性) / 100
		DamageTypeValue *= ( 100.f - Resistance ) / 100.f;

		// 若为范围伤害：通过 ApplyRadialDamageWithFalloff 应用衰减，
		// 并经由 TakeDamage 回调将实际受到的伤害值写回 DamageTypeValue
		if (UAuraEffectContextLibrary::IsRadialDamage(EffectContextHandle))
		{
			// 绑定委托：在 TakeDamage 回调中捕获经衰减后的实际伤害值
			if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(TargetAvatar))
			{
				CombatInterface->GetOnDamageSignature().AddLambda([&](float DamageAmount)
				{
					DamageTypeValue = DamageAmount;
				});
			}
			// 触发范围伤害（内部调用目标的 TakeDamage，进而触发上方委托回调）
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				TargetAvatar,
				DamageTypeValue,
				0.f,
				UAuraEffectContextLibrary::GetRadialDamageOrigin(EffectContextHandle),
				UAuraEffectContextLibrary::GetRadialDamageInnerRadius(EffectContextHandle),
				UAuraEffectContextLibrary::GetRadialDamageOuterRadius(EffectContextHandle),
				1.f,
				UDamageType::StaticClass(),
				TArray<AActor*>(),
				SourceAvatar,
				nullptr);
		}
		
		Damage += DamageTypeValue;
	}

	// 步骤 3：判定格挡（Block）
	float TargetBlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluationParameters, TargetBlockChance);
	TargetBlockChance = FMath::Max<float>(TargetBlockChance, 0.f);

	// 随机判定是否格挡成功
	const bool bBlocked = FMath::RandRange(1, 100) < TargetBlockChance;
	
	// 在 Context 中标记格挡状态（用于 UI 显示）
	UAuraEffectContextLibrary::SetIsBlockedHit(EffectContextHandle, bBlocked);

	// 如果格挡成功，伤害减半
	Damage = bBlocked ? Damage / 2.f : Damage;
	
	// 步骤 4：计算护甲减免
	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, TargetArmor);
	TargetArmor = FMath::Max<float>(TargetArmor, 0.f);

	float SourceArmorPenetration = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, EvaluationParameters, SourceArmorPenetration);
	SourceArmorPenetration = FMath::Max<float>(SourceArmorPenetration, 0.f);

	// 获取护甲穿透系数（根据源等级从曲线读取）
	const UCharacterClassInfo* CharacterClassInfo = UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);
	const FRealCurve* ArmorPenetrationCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("ArmorPenetration"), FString());
	const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(SourcePlayerLevel);
	
	// 计算有效护甲：目标护甲 * (100 - 护甲穿透 * 系数) / 100
	const float EffectiveArmor = TargetArmor * ( 100 - SourceArmorPenetration * ArmorPenetrationCoefficient ) / 100.f;

	// 获取有效护甲系数（根据目标等级从曲线读取）
	const FRealCurve* EffectiveArmorCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("EffectiveArmor"), FString());
	const float EffectiveArmorCoefficient = EffectiveArmorCurve->Eval(TargetPlayerLevel);
	
	// 应用护甲减免：伤害 = 伤害 * (100 - 有效护甲 * 系数) / 100
	Damage *= ( 100 - EffectiveArmor * EffectiveArmorCoefficient ) / 100.f;

	// 步骤 5：判定暴击（Critical Hit）
	float SourceCriticalHitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef, EvaluationParameters, SourceCriticalHitChance);
	SourceCriticalHitChance = FMath::Max<float>(SourceCriticalHitChance, 0.f);
	
	float TargetCriticalHitResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitResistanceDef, EvaluationParameters, TargetCriticalHitResistance);
	TargetCriticalHitResistance = FMath::Max<float>(TargetCriticalHitResistance, 0.f);

	float SourceCriticalHitDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitDamageDef, EvaluationParameters, SourceCriticalHitDamage);
	SourceCriticalHitDamage = FMath::Max<float>(SourceCriticalHitDamage, 0.f);

	// 获取暴击抗性系数（根据目标等级从曲线读取）
	const FRealCurve* CriticalHitResistanceCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("CriticalHitResistance"), FString());
	const float CriticalHitResistanceCoefficient = CriticalHitResistanceCurve->Eval(TargetPlayerLevel);

	// 计算有效暴击率：源暴击率 - 目标暴击抗性 * 系数
	const float EffectiveCriticalHitChance = SourceCriticalHitChance - TargetCriticalHitResistance * CriticalHitResistanceCoefficient;
	const bool bCriticalHit = FMath::RandRange(1, 100) < EffectiveCriticalHitChance;

	// 在 Context 中标记暴击状态（用于 UI 显示）
	UAuraEffectContextLibrary::SetIsCriticalHit(EffectContextHandle, bCriticalHit);

	// 如果暴击，伤害 = 2 * 伤害 + 暴击伤害加成
	Damage = bCriticalHit ? 2.f * Damage + SourceCriticalHitDamage : Damage;
	
	// 步骤 6：将最终伤害写入 IncomingDamage 中间属性（由 PostGameplayEffectExecute 转换到 Health）
	const FGameplayModifierEvaluatedData EvaluatedData(UAuraAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);
	OutExecutionOutput.AddOutputModifier(EvaluatedData);
}
