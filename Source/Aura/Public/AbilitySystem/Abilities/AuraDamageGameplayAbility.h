// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AuraAbilityTypes.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "Interaction/CombatInterface.h"
#include "AuraDamageGameplayAbility.generated.h"

/**
 * Aura 伤害型技能基类
 *
 * 所有造成伤害的技能（火焰箭、火焰爆炸、电击等）均继承自此类
 *
 * 职责：
 * - 封装伤害应用逻辑（CauseDamage）
 * - 提供伤害参数构建（MakeDamageEffectParamsFromClassDefaults）
 * - 定义通用伤害属性（伤害类型、基础伤害、Debuff 参数、击退参数等）
 * - 支持范围伤害（RadialDamage）和方向性击退/死亡冲量
 *
 * 伤害计算流程：
 *   CauseDamage → MakeDamageEffectParamsFromClassDefaults → ApplyDamageEffect
 *   → ExecCalc_Damage（计算最终伤害）→ AttributeSet.PostGameplayEffectExecute
 *
 * 使用示例（子类中）：
 *   // 对目标造成伤害
 *   CauseDamage(TargetActor);
 *   // 或者先构建参数再应用（用于需要自定义参数的情况）
 *   FDamageEffectParams Params = MakeDamageEffectParamsFromClassDefaults(TargetActor);
 *   UAuraAbilitySystemLibrary::ApplyDamageEffect(Params);
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
	 * 支持多种参数覆盖，用于需要动态修改伤害方向的情况（如追踪导弹）
	 *
	 * @param TargetActor                  目标 Actor（nullptr 表示不指定目标）
	 * @param InRadialDamageOrigin         范围伤害中心（仅 bIsRadialDamage=true 时有效）
	 * @param bOverrideKnockbackDirection  是否覆盖击退方向
	 * @param KnockbackDirectionOverride   覆盖的击退方向（需 bOverrideKnockbackDirection=true）
	 * @param bOverrideDeathImpulse        是否覆盖死亡冲量方向
	 * @param DeathImpulseDirectionOverride 覆盖的死亡冲量方向（需 bOverrideDeathImpulse=true）
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
	 * 从 Damage ScalableFloat 曲线中读取当前等级的伤害值
	 * @return 当前等级的基础伤害值
	 */
	UFUNCTION(BlueprintPure)
	float GetDamageAtLevel() const;

protected:
	/**
	 * 伤害 GameplayEffect 类
	 * 此 GE 会触发 ExecCalc_Damage 计算最终伤害并写入 IncomingDamage 元属性
	 * 在 Details 面板中指定（通常为 GE_Damage）
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	/**
	 * 伤害类型标签（火焰/闪电/奥术/物理）
	 * 决定使用哪种抗性属性来减伤，以及触发哪种 Debuff
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	FGameplayTag DamageType;

	/**
	 * 基础伤害值（ScalableFloat，支持按等级缩放）
	 * 在 CurveTable 中配置每个等级对应的伤害值
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	FScalableFloat Damage;

	/**
	 * Debuff 触发概率（百分比，0~100）
	 * 每次命中时，有此概率触发对应伤害类型的 Debuff
	 * 默认 20%
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DebuffChance = 20.f;

	/**
	 * Debuff 每次触发的伤害值
	 * 默认 5 点伤害/次
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DebuffDamage = 5.f;

	/**
	 * Debuff 触发频率（秒/次）
	 * 每隔此时间触发一次 Debuff 伤害
	 * 默认 1 秒/次
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DebuffFrequency = 1.f;

	/**
	 * Debuff 持续时间（秒）
	 * Debuff 效果的总持续时间
	 * 默认 5 秒
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DebuffDuration = 5.f;

	/**
	 * 死亡冲量大小（单位：N·s）
	 * 击杀目标时施加的物理冲量，使目标产生击飞效果
	 * 默认 1000
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DeathImpulseMagnitude = 1000.f;

	/**
	 * 击退力大小（单位：N·s）
	 * 命中目标时施加的击退力（目标未死亡时生效）
	 * 默认 1000
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float KnockbackForceMagnitude = 1000.f;

	/**
	 * 击退触发概率（百分比，0~100）
	 * 每次命中时，有此概率触发击退效果
	 * 默认 0%（不触发击退）
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float KnockbackChance = 0.f;

	/**
	 * 是否为范围伤害
	 * true：使用内外半径进行伤害衰减计算
	 * false：对单个目标造成固定伤害
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	bool bIsRadialDamage = false;

	/**
	 * 范围伤害内半径（单位：cm）
	 * 内半径内的目标受到全额伤害
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float RadialDamageInnerRadius = 0.f;

	/**
	 * 范围伤害外半径（单位：cm）
	 * 外半径外的目标不受伤害，内外半径之间按距离线性衰减
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	float RadialDamageOuterRadius = 0.f;
	
	/**
	 * 从带标签蒙太奇数组中随机获取一个（蓝图纯函数）
	 * 用于随机选择攻击动画（如近战攻击有多种动画随机播放）
	 * @param TaggedMontages 带标签的蒙太奇数组
	 * @return 随机选中的 FTaggedMontage 结构体
	 */
	UFUNCTION(BlueprintPure)
	FTaggedMontage GetRandomTaggedMontageFromArray(const TArray<FTaggedMontage>& TaggedMontages) const;
};
