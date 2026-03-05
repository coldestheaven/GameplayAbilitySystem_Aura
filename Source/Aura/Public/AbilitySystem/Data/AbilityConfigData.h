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
 * 定义 UConfigurableAbility 支持的所有动作类型
 * 每种类型对应 FAbilityActionConfig 中不同的配置字段
 */
UENUM(BlueprintType)
enum class EAbilityActionType : uint8
{
	None				UMETA(DisplayName = "无"),				// 无动作（占位符）
	SpawnProjectile		UMETA(DisplayName = "生成投射物"),		// 在指定插槽生成投射物 Actor
	ApplyEffect			UMETA(DisplayName = "应用效果"),			// 对自身或目标应用 GameplayEffect
	SpawnActor			UMETA(DisplayName = "生成Actor"),			// 在目标位置生成 Actor
	PlayMontage			UMETA(DisplayName = "播放动画"),			// 播放动画蒙太奇
	SpawnBeam			UMETA(DisplayName = "生成光束"),			// 生成光束特效（如电击链）
	Teleport			UMETA(DisplayName = "传送"),				// 将施法者传送到目标位置
	AreaOfEffect		UMETA(DisplayName = "范围效果"),			// 对范围内所有目标应用效果
	WaitForEvent		UMETA(DisplayName = "等待事件")			// 等待指定 GameplayEvent 后继续执行
};

/**
 * 投射物配置结构体
 * 用于 EAbilityActionType::SpawnProjectile 动作类型
 * 定义生成投射物的所有参数
 */
USTRUCT(BlueprintType)
struct FProjectileConfig
{
	GENERATED_BODY()
	
	/** 投射物 Actor 类（如 BP_FireBolt、BP_LightningBall 等） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<AActor> ProjectileClass;
	
	/** 同时生成的投射物数量（多发技能如 FireBolt 可配置多个） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	int32 NumProjectiles = 1;
	
	/** 多发投射物的扩散角度（度），0 表示所有投射物朝同一方向 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	float SpreadAngle = 0.f;
	
	/** 是否为追踪投射物（true 时投射物会追踪目标） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bHomingProjectile = false;
	
	/** 追踪加速度（单位：cm/s²，仅 bHomingProjectile=true 时有效） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (EditCondition = "bHomingProjectile"))
	float HomingAccelerationMagnitude = 1600.f;
	
	/** 生成位置插槽标签（如 CombatSocket.Weapon，决定投射物从哪个骨骼插槽生成） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	FGameplayTag SocketTag;
	
	/** 是否覆盖投射物的俯仰角（用于调整投射物的飞行角度） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	bool bOverridePitch = false;
	
	/** 俯仰角覆盖值（度，仅 bOverridePitch=true 时有效，负值表示向下） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile", meta = (EditCondition = "bOverridePitch"))
	float PitchOverride = 0.f;
};

/**
 * GameplayEffect 应用配置结构体
 * 用于 EAbilityActionType::ApplyEffect 动作类型
 * 定义要应用的 GE 及其目标
 */
USTRUCT(BlueprintType)
struct FEffectConfig
{
	GENERATED_BODY()
	
	/** 要应用的 GameplayEffect 类 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	TSubclassOf<UGameplayEffect> EffectClass;
	
	/** GE 应用等级（影响 ScalableFloat 曲线取值） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	float EffectLevel = 1.f;
	
	/** 是否将 GE 应用到施法者自身（如自我增益技能） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	bool bApplyToSelf = false;
	
	/** 是否将 GE 应用到目标（如对敌人施加 Debuff） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	bool bApplyToTarget = true;
};

/**
 * 动画蒙太奇配置结构体
 * 用于 EAbilityActionType::PlayMontage 动作类型
 * 定义要播放的动画及其参数
 */
USTRUCT(BlueprintType)
struct FMontageConfig
{
	GENERATED_BODY()
	
	/** 要播放的动画蒙太奇资产 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> Montage;
	
	/** 播放速率（1.0 为正常速度，2.0 为两倍速） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	float PlayRate = 1.f;
	
	/** 起始段落名称（NAME_None 表示从头播放） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	FName StartSectionName = NAME_None;
};

/**
 * 范围效果配置结构体
 * 用于 EAbilityActionType::AreaOfEffect 动作类型
 * 定义范围效果的作用范围和目标筛选规则
 */
USTRUCT(BlueprintType)
struct FAOEConfig
{
	GENERATED_BODY()
	
	/** 范围效果半径（单位：cm） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
	float Radius = 500.f;
	
	/** 是否影响友军（同阵营角色） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
	bool bAffectAllies = false;
	
	/** 是否影响敌人（不同阵营角色） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
	bool bAffectEnemies = true;
	
	/** 最大目标数量（0 表示无限制，影响范围内所有符合条件的目标） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
	int32 MaxTargets = 0;
	
	/** 对范围内目标应用的 GameplayEffect 配置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AOE")
	FEffectConfig Effect;
};

/**
 * 视觉效果配置结构体
 * 定义技能的粒子特效和音效参数
 * 可用于施法时特效（CastVisualEffect）和命中时特效（ImpactVisualEffect）
 */
USTRUCT(BlueprintType)
struct FVisualEffectConfig
{
	GENERATED_BODY()
	
	/** Niagara 粒子特效资产（软引用，按需加载） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TSoftObjectPtr<UNiagaraSystem> ParticleSystem;
	
	/** 音效资产（软引用，按需加载） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TSoftObjectPtr<USoundBase> Sound;
	
	/** 是否将特效附加到角色骨骼上（true 时特效跟随角色移动） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	bool bAttachToCharacter = false;
	
	/** 附加的骨骼插槽标签（仅 bAttachToCharacter=true 时有效） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual", meta = (EditCondition = "bAttachToCharacter"))
	FGameplayTag AttachSocketTag;
	
	/** 特效位置偏移（相对于附加点或世界坐标的偏移） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FVector LocationOffset = FVector::ZeroVector;
	
	/** 特效旋转偏移（相对于附加点的旋转偏移） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FRotator RotationOffset = FRotator::ZeroRotator;
	
	/** 特效缩放（1.0 为原始大小） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	FVector Scale = FVector::OneVector;
};

/**
 * 技能单个动作配置结构体
 * 定义技能序列中一个动作的完整参数
 * UAbilityConfigData.Actions 数组中的每个元素都是一个 FAbilityActionConfig
 */
USTRUCT(BlueprintType)
struct FAbilityActionConfig
{
	GENERATED_BODY()
	
	/** 动作类型（决定使用下面哪个配置字段） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	EAbilityActionType ActionType = EAbilityActionType::None;
	
	/** 延迟执行时间（秒），0 表示立即执行，>0 表示等待指定时间后执行 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	float Delay = 0.f;
	
	/** 投射物配置（仅 ActionType == SpawnProjectile 时显示和生效） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action", meta = (EditCondition = "ActionType == EAbilityActionType::SpawnProjectile", EditConditionHides))
	FProjectileConfig ProjectileConfig;
	
	/** 效果配置（仅 ActionType == ApplyEffect 时显示和生效） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action", meta = (EditCondition = "ActionType == EAbilityActionType::ApplyEffect", EditConditionHides))
	FEffectConfig EffectConfig;
	
	/** 动画配置（仅 ActionType == PlayMontage 时显示和生效） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action", meta = (EditCondition = "ActionType == EAbilityActionType::PlayMontage", EditConditionHides))
	FMontageConfig MontageConfig;
	
	/** 范围效果配置（仅 ActionType == AreaOfEffect 时显示和生效） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action", meta = (EditCondition = "ActionType == EAbilityActionType::AreaOfEffect", EditConditionHides))
	FAOEConfig AOEConfig;
	
	/** 视觉效果配置（所有动作类型均可配置，在动作执行时播放） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FVisualEffectConfig VisualEffect;
	
	/** 等待的事件标签（仅 ActionType == WaitForEvent 时显示和生效） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action", meta = (EditCondition = "ActionType == EAbilityActionType::WaitForEvent", EditConditionHides))
	FGameplayTag EventTag;
};

/**
 * 技能配置数据资产
 *
 * 数据驱动的技能配置系统，通过此数据资产配置技能的所有行为，无需编写 C++ 代码
 * 与 UConfigurableAbility 配合使用，实现完全数据驱动的技能设计
 *
 * 使用方式：
 *   1. 在内容浏览器中创建 UAbilityConfigData 数据资产
 *   2. 配置技能标签、名称、描述、图标等基础信息
 *   3. 在 Actions 数组中添加动作序列（如先播放动画，再生成投射物）
 *   4. 将此数据资产赋值给 UConfigurableAbility 的 AbilityConfig 字段
 */
UCLASS(BlueprintType)
class AURA_API UAbilityConfigData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
	public:
	/** 技能唯一标识标签（如 Abilities.Fire.FireBolt），用于 ASC 查找和状态管理 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FGameplayTag AbilityTag;
	
	/** 技能显示名称（在技能菜单和技能球 Tooltip 中显示） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	FText AbilityName;
	
	/** 技能描述文本（支持多行，在技能菜单的描述面板中显示） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability", meta = (MultiLine = true))
	FText AbilityDescription;
	
	/** 技能图标纹理（在技能球和技能栏中显示） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TObjectPtr<UTexture2D> AbilityIcon;
	
	/**
	 * 技能动作序列
	 * 按数组顺序依次执行每个动作，支持延迟（Delay 字段）
	 * 例如：[PlayMontage(0s), SpawnProjectile(0.3s)] 表示先播放动画，0.3秒后生成投射物
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	TArray<FAbilityActionConfig> Actions;
	
	/**
	 * 伤害类型和数值映射（按等级缩放）
	 * Key: 伤害类型标签（如 Damage.Type.Fire）
	 * Value: 该类型的伤害值（ScalableFloat，支持按等级缩放）
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Damage")
	TMap<FGameplayTag, FScalableFloat> DamageTypes;
	
	/** 冷却时间（秒，ScalableFloat 支持按等级缩放，高等级技能冷却可以更短） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cost")
	FScalableFloat CooldownDuration;
	
	/** 法力消耗（ScalableFloat 支持按等级缩放，高等级技能消耗可以更多） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Cost")
	FScalableFloat ManaCost;
	
	/** 施法时的视觉效果（粒子特效和音效，在技能激活时播放） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	FVisualEffectConfig CastVisualEffect;
	
	/** 命中时的视觉效果（粒子特效和音效，在投射物命中目标时播放） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	FVisualEffectConfig ImpactVisualEffect;
	
	/** 是否可以在移动时施放（false 时施法会打断移动） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	bool bCanCastWhileMoving = true;
	
	/** 是否需要目标（true 时需要鼠标指向有效目标才能施放） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	bool bRequiresTarget = true;
	
	/** 最大施法距离（单位：cm，超出此距离无法施放） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	float MaxCastRange = 2000.f;
	
	/** 施法时间（秒，0 表示瞬发，>0 表示需要引导） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	float CastTime = 0.f;
	
	/** 是否可以被打断（true 时受击或眩晕会中断施法） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability")
	bool bCanBeInterrupted = true;
};
