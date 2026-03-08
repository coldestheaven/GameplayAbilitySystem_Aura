// Copyright Druid Mechanics


#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

/**
 * 对目标造成伤害
 * 
 * 实现流程：
 * 1. 创建伤害 GE Spec（使用 DamageEffectClass）
 * 2. 计算等级对应的伤害值（从 Damage 曲线获取）
 * 3. 使用 SetByCaller 设置伤害值（通过 DamageType 标签）
 * 4. 应用伤害 GE 到目标 ASC
 * 
 * @param TargetActor 目标 Actor（要造成伤害的目标）
 * 
 * 使用场景：
 * - 近战攻击、直接伤害技能等需要直接造成伤害时调用
 * 
 * 注意：
 * - 伤害值通过 SetByCaller 模式设置，在 ExecCalc_Damage 中读取
 * - DamageType 标签用于区分伤害类型（火焰、闪电、物理等）
 */
void UAuraDamageGameplayAbility::CauseDamage(AActor* TargetActor)
{
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, 1.f);
	const float ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel());
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, DamageType, ScaledDamage);
	GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
}

/**
 * 从类默认值创建伤害效果参数
 * 
 * 实现流程：
 * 1. 填充基础参数（WorldContext、GE 类、ASC、伤害值、等级、伤害类型等）
 * 2. 填充 Debuff 参数（概率、伤害、持续时间、频率）
 * 3. 填充物理参数（死亡冲量、击退力度、击退概率）
 * 4. 如果目标有效，计算默认方向：
 *    - 从源位置指向目标位置
 *    - 如果未覆盖，使用此方向作为击退和死亡冲量方向
 * 5. 如果覆盖击退方向，使用覆盖值（可指定 Pitch）
 * 6. 如果覆盖死亡冲量方向，使用覆盖值（可指定 Pitch）
 * 7. 如果是范围伤害，填充范围参数
 * 
 * @param TargetActor 目标 Actor（用于计算默认方向）
 * @param InRadialDamageOrigin 范围伤害原点（如果是范围伤害）
 * @param bOverrideKnockbackDirection 是否覆盖击退方向
 * @param KnockbackDirectionOverride 击退方向覆盖值
 * @param bOverrideDeathImpulse 是否覆盖死亡冲量方向
 * @param DeathImpulseDirectionOverride 死亡冲量方向覆盖值
 * @param bOverridePitch 是否覆盖 Pitch 角度
 * @param PitchOverride Pitch 角度覆盖值
 * @return 填充完成的伤害效果参数结构体
 * 
 * 使用场景：
 * - 投射物技能生成投射物时设置伤害参数
 * - 范围技能设置范围伤害参数
 * 
 * 注意：
 * - 如果未覆盖方向，默认方向为从源指向目标
 * - Pitch 覆盖用于调整击退/冲量的垂直角度（如向上击飞）
 */
FDamageEffectParams UAuraDamageGameplayAbility::MakeDamageEffectParamsFromClassDefaults(AActor* TargetActor,
	FVector InRadialDamageOrigin, bool bOverrideKnockbackDirection, FVector KnockbackDirectionOverride,
	bool bOverrideDeathImpulse, FVector DeathImpulseDirectionOverride, bool bOverridePitch, float PitchOverride) const
{
	FDamageEffectParams Params;
	
	// 填充基础参数
	Params.WorldContextObject = GetAvatarActorFromActorInfo();
	Params.DamageGameplayEffectClass = DamageEffectClass;
	Params.SourceAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	Params.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	Params.BaseDamage = Damage.GetValueAtLevel(GetAbilityLevel());
	Params.AbilityLevel = GetAbilityLevel();
	Params.DamageType = DamageType;
	
	// 填充 Debuff 参数
	Params.DebuffChance = DebuffChance;
	Params.DebuffDamage = DebuffDamage;
	Params.DebuffDuration = DebuffDuration;
	Params.DebuffFrequency = DebuffFrequency;
	
	// 填充物理参数
	Params.DeathImpulseMagnitude = DeathImpulseMagnitude;
	Params.KnockbackForceMagnitude = KnockbackForceMagnitude;
	Params.KnockbackChance = KnockbackChance;

	// 如果目标有效，计算默认方向（从源指向目标）
	if (IsValid(TargetActor))
	{
		FRotator Rotation = (TargetActor->GetActorLocation() - GetAvatarActorFromActorInfo()->GetActorLocation()).Rotation();
		if (bOverridePitch)
		{
			Rotation.Pitch = PitchOverride;
		}
		const FVector ToTarget = Rotation.Vector();
		
		// 如果未覆盖，使用默认方向
		if (!bOverrideKnockbackDirection)
		{
			Params.KnockbackForce = ToTarget * KnockbackForceMagnitude;
		}
		if (!bOverrideDeathImpulse)
		{
			Params.DeathImpulse = ToTarget * DeathImpulseMagnitude;
		}
	}
	
	// 如果覆盖击退方向，使用覆盖值
	if (bOverrideKnockbackDirection)
	{
		KnockbackDirectionOverride.Normalize();
		Params.KnockbackForce = KnockbackDirectionOverride * KnockbackForceMagnitude;
		if (bOverridePitch)
		{
			FRotator KnockbackRotation = KnockbackDirectionOverride.Rotation();
			KnockbackRotation.Pitch = PitchOverride;
			Params.KnockbackForce = KnockbackRotation.Vector() * KnockbackForceMagnitude;
		}
	}

	// 如果覆盖死亡冲量方向，使用覆盖值
	if (bOverrideDeathImpulse)
	{
		DeathImpulseDirectionOverride.Normalize();
		Params.DeathImpulse = DeathImpulseDirectionOverride * DeathImpulseMagnitude;
		if (bOverridePitch)
		{
			FRotator DeathImpulseRotation = DeathImpulseDirectionOverride.Rotation();
			DeathImpulseRotation.Pitch = PitchOverride;
			Params.DeathImpulse = DeathImpulseRotation.Vector() * DeathImpulseMagnitude;
		}
	}
	
	// 如果是范围伤害，填充范围参数
	if (bIsRadialDamage)
	{
		Params.bIsRadialDamage = bIsRadialDamage;
		Params.RadialDamageOrigin = InRadialDamageOrigin;
		Params.RadialDamageInnerRadius = RadialDamageInnerRadius;
		Params.RadialDamageOuterRadius = RadialDamageOuterRadius;
	}
	return Params;
}

float UAuraDamageGameplayAbility::GetDamageAtLevel() const
{
	return Damage.GetValueAtLevel(GetAbilityLevel());
}

FTaggedMontage UAuraDamageGameplayAbility::GetRandomTaggedMontageFromArray(const TArray<FTaggedMontage>& TaggedMontages) const
{
	if (TaggedMontages.Num() > 0)
	{
		const int32 Selection = FMath::RandRange(0, TaggedMontages.Num() - 1);
		return TaggedMontages[Selection];
	}

	return FTaggedMontage();
}
