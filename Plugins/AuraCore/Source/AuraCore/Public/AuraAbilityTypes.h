#pragma once

#include "GameplayEffectTypes.h"
#include "AuraAbilityTypes.generated.h"

class UGameplayEffect;

/**
 * 伤害效果参数结构体
 *
 * 封装了应用一次伤害所需的所有参数
 * 由 UAuraDamageGameplayAbility::MakeDamageEffectParamsFromClassDefaults 构建
 * 由 UAuraAbilitySystemLibrary::ApplyDamageEffect 消费并应用
 *
 * 参数分类：
 * 1. 基础参数：WorldContextObject、DamageGameplayEffectClass、来源/目标 ASC
 * 2. 伤害参数：BaseDamage、AbilityLevel、DamageType
 * 3. Debuff 参数：DebuffChance、DebuffDamage、DebuffDuration、DebuffFrequency
 * 4. 物理参数：DeathImpulseMagnitude/DeathImpulse（死亡冲量）、KnockbackForceMagnitude/KnockbackChance/KnockbackForce（击退）
 * 5. 范围伤害参数：bIsRadialDamage、RadialDamageInnerRadius、RadialDamageOuterRadius、RadialDamageOrigin
 */
USTRUCT(BlueprintType)
struct FDamageEffectParams
{
	GENERATED_BODY()

	FDamageEffectParams(){}

	/** 世界上下文对象（用于获取 World 指针） */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UObject> WorldContextObject = nullptr;

	/** 伤害 GameplayEffect 类（触发 ExecCalc_Damage 计算最终伤害） */
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UGameplayEffect> DamageGameplayEffectClass = nullptr;

	/** 伤害来源的 AbilitySystemComponent（施法者的 ASC） */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> SourceAbilitySystemComponent;

	/** 伤害目标的 AbilitySystemComponent（受伤者的 ASC） */
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> TargetAbilitySystemComponent;

	/** 基础伤害值（在 ExecCalc_Damage 中经过护甲、暴击等计算后得到最终伤害） */
	UPROPERTY(BlueprintReadWrite)
	float BaseDamage = 0.f;

	/** 技能等级（影响 ScalableFloat 曲线取值） */
	UPROPERTY(BlueprintReadWrite)
	float AbilityLevel = 1.f;

	/** 伤害类型标签（火焰/闪电/奥术/物理，决定使用哪种抗性减伤和触发哪种 Debuff） */
	UPROPERTY(BlueprintReadWrite)
	FGameplayTag DamageType = FGameplayTag();

	/** Debuff 触发概率（百分比，0~100） */
	UPROPERTY(BlueprintReadWrite)
	float DebuffChance = 0.f;

	/** Debuff 每次触发的伤害值 */
	UPROPERTY(BlueprintReadWrite)
	float DebuffDamage = 0.f;

	/** Debuff 总持续时间（秒） */
	UPROPERTY(BlueprintReadWrite)
	float DebuffDuration = 0.f;

	/** Debuff 触发频率（秒/次） */
	UPROPERTY(BlueprintReadWrite)
	float DebuffFrequency = 0.f;

	/** 死亡冲量大小（单位：N·s，击杀时施加的物理冲量大小） */
	UPROPERTY(BlueprintReadWrite)
	float DeathImpulseMagnitude = 0.f;

	/** 死亡冲量向量（方向 * 大小，由 MakeDamageEffectParamsFromClassDefaults 计算） */
	UPROPERTY(BlueprintReadWrite)
	FVector DeathImpulse = FVector::ZeroVector;

	/** 击退力大小（单位：N·s，命中时施加的击退力大小） */
	UPROPERTY(BlueprintReadWrite)
	float KnockbackForceMagnitude = 0.f;

	/** 击退触发概率（百分比，0~100） */
	UPROPERTY(BlueprintReadWrite)
	float KnockbackChance = 0.f;

	/** 击退力向量（方向 * 大小，由 MakeDamageEffectParamsFromClassDefaults 计算） */
	UPROPERTY(BlueprintReadWrite)
	FVector KnockbackForce = FVector::ZeroVector;

	/** 是否为范围伤害（true 时使用内外半径进行伤害衰减） */
	UPROPERTY(BlueprintReadWrite)
	bool bIsRadialDamage = false;

	/** 范围伤害内半径（单位：cm，内半径内受到全额伤害） */
	UPROPERTY(BlueprintReadWrite)
	float RadialDamageInnerRadius = 0.f;

	/** 范围伤害外半径（单位：cm，外半径外不受伤害） */
	UPROPERTY(BlueprintReadWrite)
	float RadialDamageOuterRadius = 0.f;

	/** 范围伤害爆炸中心位置（世界坐标） */
	UPROPERTY(BlueprintReadWrite)
	FVector RadialDamageOrigin = FVector::ZeroVector;
};

/**
 * Aura 自定义 GameplayEffect 上下文
 *
 * 扩展了 FGameplayEffectContext，添加了 Aura 游戏特有的伤害信息字段
 * 这些字段在 ExecCalc_Damage 中写入，在 AttributeSet.PostGameplayEffectExecute 中读取
 *
 * 自定义字段：
 * - bIsBlockedHit：是否被格挡（伤害减半）
 * - bIsCriticalHit：是否暴击（伤害翻倍）
 * - bIsSuccessfulDebuff：是否成功触发 Debuff
 * - DebuffDamage/Duration/Frequency：Debuff 参数
 * - DamageType：伤害类型标签（用于创建对应的 Debuff GE）
 * - DeathImpulse：死亡冲量向量
 * - KnockbackForce：击退力向量
 * - 范围伤害参数：bIsRadialDamage、内外半径、爆炸中心
 *
 * 网络序列化：
 * - 通过 NetSerialize 自定义序列化，确保所有字段正确同步到客户端
 * - TStructOpsTypeTraits 声明 WithNetSerializer=true 启用自定义序列化
 */
USTRUCT(BlueprintType)
struct AURACORE_API FAuraGameplayEffectContext : public FGameplayEffectContext
{
	GENERATED_BODY()

public:
	/** 获取是否为暴击 */
	bool IsCriticalHit() const { return bIsCriticalHit; }

	/** 获取是否被格挡 */
	bool IsBlockedHit () const { return bIsBlockedHit; }

	/** 获取是否成功触发 Debuff */
	bool IsSuccessfulDebuff() const { return bIsSuccessfulDebuff; }

	/** 获取 Debuff 伤害值 */
	float GetDebuffDamage() const { return DebuffDamage; }

	/** 获取 Debuff 持续时间 */
	float GetDebuffDuration() const { return DebuffDuration; }

	/** 获取 Debuff 触发频率 */
	float GetDebuffFrequency() const { return DebuffFrequency; }

	/** 获取伤害类型标签（SharedPtr，避免拷贝开销） */
	TSharedPtr<FGameplayTag> GetDamageType() const { return DamageType; }

	/** 获取死亡冲量向量 */
	FVector GetDeathImpulse() const { return DeathImpulse; }

	/** 获取击退力向量 */
	FVector GetKnockbackForce() const { return KnockbackForce; }

	/** 获取是否为范围伤害 */
	bool IsRadialDamage() const { return bIsRadialDamage; }

	/** 获取范围伤害内半径 */
	float GetRadialDamageInnerRadius() const { return RadialDamageInnerRadius; }

	/** 获取范围伤害外半径 */
	float GetRadialDamageOuterRadius() const { return RadialDamageOuterRadius; }

	/** 获取范围伤害爆炸中心位置 */
	FVector GetRadialDamageOrigin() const { return RadialDamageOrigin; }

	/** 设置是否为暴击 */
	void SetIsCriticalHit(bool bInIsCriticalHit) { bIsCriticalHit = bInIsCriticalHit; }

	/** 设置是否被格挡 */
	void SetIsBlockedHit(bool bInIsBlockedHit) { bIsBlockedHit = bInIsBlockedHit; }

	/** 设置是否成功触发 Debuff */
	void SetIsSuccessfulDebuff(bool bInIsDebuff) { bIsSuccessfulDebuff = bInIsDebuff; }

	/** 设置 Debuff 伤害值 */
	void SetDebuffDamage(float InDamage) { DebuffDamage = InDamage; }

	/** 设置 Debuff 持续时间 */
	void SetDebuffDuration(float InDuration) { DebuffDuration = InDuration; }

	/** 设置 Debuff 触发频率 */
	void SetDebuffFrequency(float InFrequency) { DebuffFrequency = InFrequency; }

	/** 设置伤害类型标签 */
	void SetDamageType(TSharedPtr<FGameplayTag> InDamageType) { DamageType = InDamageType; }

	/** 设置死亡冲量向量 */
	void SetDeathImpulse(const FVector& InImpulse) { DeathImpulse = InImpulse; }

	/** 设置击退力向量 */
	void SetKnockbackForce(const FVector& InForce) { KnockbackForce = InForce; }

	/** 设置是否为范围伤害 */
	void SetIsRadialDamage(bool bInIsRadialDamage) { bIsRadialDamage = bInIsRadialDamage; }

	/** 设置范围伤害内半径 */
	void SetRadialDamageInnerRadius(float InRadialDamageInnerRadius) { RadialDamageInnerRadius = InRadialDamageInnerRadius; }

	/** 设置范围伤害外半径 */
	void SetRadialDamageOuterRadius(float InRadialDamageOuterRadius) { RadialDamageOuterRadius = InRadialDamageOuterRadius; }

	/** 设置范围伤害爆炸中心位置 */
	void SetRadialDamageOrigin(const FVector& InRadialDamageOrigin) { RadialDamageOrigin = InRadialDamageOrigin; }
	
	/**
	 * 返回实际使用的结构体类型（子类必须重写）
	 * 用于 GAS 内部的类型识别和序列化
	 */
	virtual UScriptStruct* GetScriptStruct() const
	{
		return FGameplayEffectContext::StaticStruct();
	}

	/**
	 * 创建此上下文的深拷贝（用于 GE 预测和延迟应用）
	 * @return 新分配的上下文副本
	 */
	virtual FGameplayEffectContext* Duplicate() const
	{
		FGameplayEffectContext* NewContext = new FGameplayEffectContext();
		*NewContext = *this;
		if (GetHitResult())
		{
			// 深拷贝命中结果
			NewContext->AddHitResult(*GetHitResult(), true);
		}
		return NewContext;
	}

	/**
	 * 自定义网络序列化（子类必须重写）
	 * 将所有自定义字段序列化到网络数据包，确保客户端收到完整的上下文信息
	 */
	virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess);
	
protected:
	/** 是否被格挡（格挡时伤害减半） */
	UPROPERTY()
	bool bIsBlockedHit = false;
	
	/** 是否为暴击（暴击时伤害翻倍） */
	UPROPERTY()
	bool bIsCriticalHit = false;

	/** 是否成功触发 Debuff（由 DebuffChance 概率决定） */
	UPROPERTY()
	bool bIsSuccessfulDebuff = false;

	/** Debuff 每次触发的伤害值 */
	UPROPERTY()
	float DebuffDamage = 0.f;

	/** Debuff 总持续时间（秒） */
	UPROPERTY()
	float DebuffDuration = 0.f;

	/** Debuff 触发频率（秒/次） */
	UPROPERTY()
	float DebuffFrequency = 0.f;

	/** 伤害类型标签（使用 SharedPtr 避免拷贝，不参与网络序列化，通过 NetSerialize 手动处理） */
	TSharedPtr<FGameplayTag> DamageType;

	/** 死亡冲量向量（击杀时施加的物理冲量） */
	UPROPERTY()
	FVector DeathImpulse = FVector::ZeroVector;

	/** 击退力向量（命中时施加的击退力） */
	UPROPERTY()
	FVector KnockbackForce = FVector::ZeroVector;

	/** 是否为范围伤害 */
	UPROPERTY()
	bool bIsRadialDamage = false;

	/** 范围伤害内半径（单位：cm） */
	UPROPERTY()
	float RadialDamageInnerRadius = 0.f;

	/** 范围伤害外半径（单位：cm） */
	UPROPERTY()
	float RadialDamageOuterRadius = 0.f;

	/** 范围伤害爆炸中心位置（世界坐标） */
	UPROPERTY()
	FVector RadialDamageOrigin = FVector::ZeroVector;
};

/**
 * 类型特性声明
 * 启用自定义网络序列化（WithNetSerializer=true）
 * 启用拷贝构造（WithCopy=true）
 */
template<>
struct TStructOpsTypeTraits<FAuraGameplayEffectContext> : public TStructOpsTypeTraitsBase2<FAuraGameplayEffectContext>
{
	enum
	{
		WithNetSerializer = true,
		WithCopy = true
	};
};
