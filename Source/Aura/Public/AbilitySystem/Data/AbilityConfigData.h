// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "Curves/CurveFloat.h"
#include "ScalableFloat.h"
#include "AbilityConfigData.generated.h"

class UGameplayEffect;
class UAnimMontage;
class UNiagaraSystem;
class USoundBase;

/**
 * 技能动作类型枚举
 */
UENUM(BlueprintType)
enum class EAbilityActionType : uint8
{
	None				UMETA(DisplayName = "无"),
	SpawnProjectile		UMETA(DisplayName = "生成投射物"),
	ApplyEffect			UMETA(DisplayName = "应用效果"),
	SpawnActor			UMETA(DisplayName = "生成Actor"),
	PlayMontage			UMETA(DisplayName = "播放动画"),
	SpawnBeam			UMETA(DisplayName = "生成光束"),
	Teleport			UMETA(DisplayName = "传送"),
	AreaOfEffect		UMETA(DisplayName = "范围效果"),
	WaitForEvent		UMETA(DisplayName = "等待事件")
};

/**
 * 投射物配置
 */
USTRUCT(BlueprintType)
struct FProjectileConfig
{
	GENERATED_BODY()
	
	// 投射物类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<AActor> ProjectileClass;
	
	// 投射物数量
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	int32 NumProjectiles = 1;
	
	// 扩散角度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float SpreadAngle = 0.f;
	
	// 是否追踪目标
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bHomingProjectile = false;
	
	// 追踪加速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (EditCondition = "bHomingProjectile"))
	float HomingAccelerationMagnitude = 1600.f;
	
	// 生成位置标签
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FGameplayTag SocketTag;
	
	// 是否覆盖俯仰角
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bOverridePitch = false;
	
	// 俯仰角覆盖值
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (EditCondition = "bOverridePitch"))
	float PitchOverride = 0.f;
};

/**
 * 效果配置
 */
USTRUCT(BlueprintType)
struct FEffectConfig
{
	GENERATED_BODY()
	
	// 效果类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TSubclassOf<UGameplayEffect> EffectClass;
	
	// 效果等级
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float EffectLevel = 1.f;
	
	// 是否应用到自己
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	bool bApplyToSelf = false;
	
	// 是否应用到目标
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	bool bApplyToTarget = true;
};

/**
 * 动画配置
 */
USTRUCT(BlueprintType)
struct FMontageConfig
{
	GENERATED_BODY()
	
	// 动画蒙太奇
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> Montage;
	
	// 播放速率
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float PlayRate = 1.f;
	
	// 起始位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FName StartSectionName = NAME_None;
};

/**
 * 范围效果配置
 */
USTRUCT(BlueprintType)
struct FAOEConfig
{
	GENERATED_BODY()
	
	// 范围半径
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
	float Radius = 500.f;
	
	// 是否影响友军
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
	bool bAffectAllies = false;
	
	// 是否影响敌人
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
	bool bAffectEnemies = true;
	
	// 最大目标数量（0表示无限制）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
	int32 MaxTargets = 0;
	
	// 应用的效果
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
	FEffectConfig Effect;
};

/**
 * 视觉效果配置
 */
USTRUCT(BlueprintType)
struct FVisualEffectConfig
{
	GENERATED_BODY()
	
	// 粒子效果
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TSoftObjectPtr<UNiagaraSystem> ParticleSystem;
	
	// 音效
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TSoftObjectPtr<USoundBase> Sound;
	
	// 是否附加到角色
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	bool bAttachToCharacter = false;
	
	// 附加位置标签
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (EditCondition = "bAttachToCharacter"))
	FGameplayTag AttachSocketTag;
	
	// 位置偏移
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FVector LocationOffset = FVector::ZeroVector;
	
	// 旋转偏移
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FRotator RotationOffset = FRotator::ZeroRotator;
	
	// 缩放
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FVector Scale = FVector::OneVector;
};

/**
 * 技能动作配置
 */
USTRUCT(BlueprintType)
struct FAbilityActionConfig
{
	GENERATED_BODY()
	
	// 动作类型
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	EAbilityActionType ActionType = EAbilityActionType::None;
	
	// 延迟执行时间
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	float Delay = 0.f;
	
	// 投射物配置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action", meta = (EditCondition = "ActionType == EAbilityActionType::SpawnProjectile", EditConditionHides))
	FProjectileConfig ProjectileConfig;
	
	// 效果配置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action", meta = (EditCondition = "ActionType == EAbilityActionType::ApplyEffect", EditConditionHides))
	FEffectConfig EffectConfig;
	
	// 动画配置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action", meta = (EditCondition = "ActionType == EAbilityActionType::PlayMontage", EditConditionHides))
	FMontageConfig MontageConfig;
	
	// 范围效果配置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action", meta = (EditCondition = "ActionType == EAbilityActionType::AreaOfEffect", EditConditionHides))
	FAOEConfig AOEConfig;
	
	// 视觉效果配置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FVisualEffectConfig VisualEffect;
	
	// 等待的事件标签（用于WaitForEvent类型）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action", meta = (EditCondition = "ActionType == EAbilityActionType::WaitForEvent", EditConditionHides))
	FGameplayTag EventTag;
};

/**
 * 技能配置数据资产
 * 
 * 用于配置技能的所有行为，无需编写C++代码
 */
UCLASS(BlueprintType)
class AURA_API UAbilityConfigData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	// 技能标签
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FGameplayTag AbilityTag;
	
	// 技能名称
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FText AbilityName;
	
	// 技能描述
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability", meta = (MultiLine = true))
	FText AbilityDescription;
	
	// 技能图标
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TObjectPtr<UTexture2D> AbilityIcon;
	
	// 技能动作序列
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TArray<FAbilityActionConfig> Actions;
	
	// 伤害类型和数值（按等级缩放）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TMap<FGameplayTag, FScalableFloat> DamageTypes;
	
	// 冷却时间（按等级缩放）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cost")
	FScalableFloat CooldownDuration;
	
	// 法力消耗（按等级缩放）
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cost")
	FScalableFloat ManaCost;
	
	// 施法时视觉效果
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	FVisualEffectConfig CastVisualEffect;
	
	// 命中时视觉效果
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	FVisualEffectConfig ImpactVisualEffect;
	
	// 是否可以在移动时施放
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	bool bCanCastWhileMoving = true;
	
	// 是否需要目标
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	bool bRequiresTarget = true;
	
	// 最大施法距离
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	float MaxCastRange = 2000.f;
	
	// 施法时间
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	float CastTime = 0.f;
	
	// 是否可以被打断
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	bool bCanBeInterrupted = true;
};
