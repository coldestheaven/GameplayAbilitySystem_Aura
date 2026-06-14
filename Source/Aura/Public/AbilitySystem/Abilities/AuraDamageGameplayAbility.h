// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AuraAbilityTypes.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "Interaction/CombatInterface.h"
#include "ScalableFloat.h"
#include "AuraDamageGameplayAbility.generated.h"

/**
 * 伤害基础参数（伤害 GE / 伤害类型 / 基础伤害曲线）
 *
 * 所有伤害型技能必填的核心字段聚合
 */
USTRUCT(BlueprintType)
struct FAuraDamageBase
{
	GENERATED_BODY()

	/**
	 * 伤害 GameplayEffect 类
	 * 此 GE 会触发 ExecCalc_Damage 计算最终伤害并写入 IncomingDamage 元属性
	 * 通常为 GE_Damage
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/**
	 * 伤害类型标签（火焰/闪电/奥术/物理）
	 * 决定使用哪种抗性属性来减伤，以及触发哪种 Debuff
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag DamageType;

	/**
	 * 基础伤害值（ScalableFloat，支持按等级缩放）
	 * 在 CurveTable 中配置每个等级对应的伤害值
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FScalableFloat Damage;
};

/**
 * Debuff 参数（命中时的负面状态触发配置）
 */
USTRUCT(BlueprintType)
struct FAuraDebuffParams
{
	GENERATED_BODY()

	/** Debuff 触发概率（百分比，0~100） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Chance = 20.f;

	/** Debuff 每次触发的伤害值 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Damage = 5.f;

	/** Debuff 触发频率（秒/次） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Frequency = 1.f;

	/** Debuff 持续时间（秒） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float Duration = 5.f;
};

/**
 * 击退/死亡冲量参数（命中/击杀时的物理反馈配置）
 */
USTRUCT(BlueprintType)
struct FAuraImpactParams
{
	GENERATED_BODY()

	/** 死亡冲量大小（单位：N·s）：击杀时施加给目标的物理冲量 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float DeathImpulseMagnitude = 1000.f;

	/** 击退力大小（单位：N·s）：命中目标时施加的击退力（未死亡时生效） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float KnockbackForceMagnitude = 1000.f;

	/** 击退触发概率（百分比，0~100） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float KnockbackChance = 0.f;
};

/**
 * 范围伤害参数（圆形 AOE 衰减配置）
 */
USTRUCT(BlueprintType)
struct FAuraRadialDamageParams
{
	GENERATED_BODY()

	/**
	 * 是否为范围伤害
	 * true：使用内外半径进行伤害衰减计算
	 * false：忽略此结构体内其它字段
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bIsRadialDamage = false;

	/** 范围伤害内半径（单位：cm）：内半径内的目标受到全额伤害 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float InnerRadius = 0.f;

	/** 范围伤害外半径（单位：cm）：外半径外的目标不受伤害，内外之间线性衰减 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float OuterRadius = 0.f;
};

/**
 * Aura 伤害型技能基类
 *
 * 所有造成伤害的技能（火焰箭、火焰爆炸、电击等）均继承自此类
 *
 * 职责：
 * - 封装伤害应用逻辑（CauseDamage）
 * - 提供伤害参数构建（MakeDamageEffectParamsFromClassDefaults）
 * - 通过 4 个聚合结构体（DamageBase/Debuff/Impact/RadialDamage）定义可配置参数
 *
 * 字段聚合（重构 v2）：
 *   DamageBase    : DamageEffectClass / DamageType / Damage 曲线
 *   DebuffParams  : Chance / Damage / Frequency / Duration
 *   ImpactParams  : DeathImpulseMagnitude / KnockbackForceMagnitude / KnockbackChance
 *   RadialDamage  : bIsRadialDamage / InnerRadius / OuterRadius
 *
 * 伤害计算流程：
 *   CauseDamage → MakeDamageEffectParamsFromClassDefaults → ApplyDamageEffect
 *   → ExecCalc_Damage（计算最终伤害）→ AttributeSet.PostGameplayEffectExecute
 */
UCLASS()
class AURA_API UAuraDamageGameplayAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()
public:

	/**
	 * 对目标造成伤害（蓝图可调用）
	 * 使用默认参数构建 FDamageEffectParams 并应用伤害 GE
	 * @param TargetActor 受伤目标 Actor
	 */
	UFUNCTION(BlueprintCallable)
	void CauseDamage(AActor* TargetActor);

	/**
	 * 从类默认值构建伤害效果参数（蓝图纯函数）
	 * 将此类的所有伤害属性打包成 FDamageEffectParams 结构体
	 *
	 * @param TargetActor                  目标 Actor（nullptr 表示不指定目标）
	 * @param InRadialDamageOrigin         范围伤害中心（仅 bIsRadialDamage=true 时有效）
	 * @param bOverrideKnockbackDirection  是否覆盖击退方向
	 * @param KnockbackDirectionOverride   覆盖的击退方向
	 * @param bOverrideDeathImpulse        是否覆盖死亡冲量方向
	 * @param DeathImpulseDirectionOverride 覆盖的死亡冲量方向
	 * @param bOverridePitch               是否覆盖俯仰角
	 * @param PitchOverride                覆盖的俯仰角（度）
	 * @return 构建好的伤害效果参数结构体
	 */
	UFUNCTION(BlueprintPure)
	FDamageEffectParams MakeDamageEffectParamsFromClassDefaults(
		AActor* TargetActor = nullptr,
		FVector InRadialDamageOrigin = FVector::ZeroVector,
		bool bOverrideKnockbackDirection = false,
		FVector KnockbackDirectionOverride = FVector::ZeroVector,
		bool bOverrideDeathImpulse = false,
		FVector DeathImpulseDirectionOverride = FVector::ZeroVector,
		bool bOverridePitch = false,
		float PitchOverride = 0.f) const;

	/**
	 * 获取当前技能等级对应的伤害值（蓝图纯函数）
	 * @return 当前等级的基础伤害值
	 */
	UFUNCTION(BlueprintPure)
	float GetDamageAtLevel() const;

	/** 获取伤害类型标签（蓝图纯函数，便于子类描述函数访问） */
	UFUNCTION(BlueprintPure, Category = "Damage")
	FGameplayTag GetDamageType() const { return DamageBase.DamageType; }

	/** 获取伤害曲线（蓝图纯函数，便于子类按等级取值） */
	const FScalableFloat& GetDamageScalable() const { return DamageBase.Damage; }

protected:
	/** 伤害基础参数（伤害 GE / 类型 / 曲线） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage", meta = (ShowOnlyInnerProperties))
	FAuraDamageBase DamageBase;

	/** Debuff 参数（命中时触发的负面状态配置） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage|Debuff", meta = (ShowOnlyInnerProperties))
	FAuraDebuffParams DebuffParams;

	/** 击退/死亡冲量参数（物理反馈配置） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage|Impact", meta = (ShowOnlyInnerProperties))
	FAuraImpactParams ImpactParams;

	/** 范围伤害参数（仅 bIsRadialDamage=true 时使用其它字段） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage|Radial", meta = (ShowOnlyInnerProperties))
	FAuraRadialDamageParams RadialDamage;

	/**
	 * 从带标签蒙太奇数组中随机获取一个（蓝图纯函数）
	 * @param TaggedMontages 带标签的蒙太奇数组
	 * @return 随机选中的 FTaggedMontage 结构体
	 */
	UFUNCTION(BlueprintPure)
	FTaggedMontage GetRandomTaggedMontageFromArray(const TArray<FTaggedMontage>& TaggedMontages) const;
};
