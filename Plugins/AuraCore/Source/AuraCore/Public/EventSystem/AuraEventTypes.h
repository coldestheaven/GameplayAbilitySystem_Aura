// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AuraEventTypes.generated.h"

/**
 * 事件基类
 * 所有 Aura 事件结构体必须继承自此类
 * 提供通用的事件标签和时间戳字段
 */
USTRUCT(BlueprintType)
struct AURACORE_API FAuraEvent
{
	GENERATED_BODY()
	
	/** 事件类型标签（用于事件分类和过滤） */
	UPROPERTY(BlueprintReadOnly, Category = "Event")
	FGameplayTag EventTag;
	
	/** 事件发生的游戏时间戳（秒） */
	UPROPERTY(BlueprintReadOnly, Category = "Event")
	float Timestamp;
	
	FAuraEvent()
		: Timestamp(0.f)
	{}
	
	virtual ~FAuraEvent() = default;
};

/**
 * 属性变化事件
 * 当角色的 GameplayAttribute（生命值、法力值等）发生变化时发布
 */
USTRUCT(BlueprintType)
struct AURACORE_API FAttributeChangedEvent : public FAuraEvent
{
	GENERATED_BODY()
	
	/** 变化的属性标签（如 Attributes.Vital.Health） */
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayTag AttributeTag;
	
	/** 变化前的属性值 */
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	float OldValue;
	
	/** 变化后的属性值 */
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	float NewValue;
	
	/** 触发属性变化的来源 Actor（如造成伤害的攻击者） */
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	AActor* Instigator;

	FAttributeChangedEvent()
		: OldValue(0.f)
		, NewValue(0.f)
		, Instigator(nullptr)
	{}
};

/**
 * 技能激活事件
 * 当角色成功激活一个 GameplayAbility 时发布
 */
USTRUCT(BlueprintType)
struct AURACORE_API FAbilityActivatedEvent : public FAuraEvent
{
	GENERATED_BODY()
	
	/** 激活的技能标签（如 Abilities.Fire.FireBolt） */
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayTag AbilityTag;
	
	/** 激活技能的角色 */
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	AActor* Instigator;
	
	/** 技能的目标 Actor（可为 nullptr） */
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	AActor* Target;

	FAbilityActivatedEvent()
		: Instigator(nullptr)
		, Target(nullptr)
	{}
};

/**
 * 伤害事件
 * 当角色受到伤害时发布（在 AttributeSet.PostGameplayEffectExecute 中触发）
 */
USTRUCT(BlueprintType)
struct AURACORE_API FAuraDamageEvent : public FAuraEvent
{
	GENERATED_BODY()
	
	/** 最终伤害数值（经过护甲、格挡、暴击计算后的值） */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float Damage;
	
	/** 是否为暴击（伤害翻倍） */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	bool bCriticalHit;
	
	/** 是否被格挡（伤害减半） */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	bool bBlockedHit;
	
	/** 造成伤害的攻击者 Actor */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	AActor* Attacker;
	
	/** 受到伤害的目标 Actor */
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	AActor* Victim;

	FAuraDamageEvent()
		: Damage(0.f)
		, bCriticalHit(false)
		, bBlockedHit(false)
		, Attacker(nullptr)
		, Victim(nullptr)
	{}
};

/**
 * 经验值获得事件
 * 当玩家击杀敌人获得 XP 时发布
 */
USTRUCT(BlueprintType)
struct AURACORE_API FXPGainedEvent : public FAuraEvent
{
	GENERATED_BODY()
	
	/** 获得的 XP 数量 */
	UPROPERTY(BlueprintReadOnly, Category = "XP")
	int32 XPAmount;
	
	/** XP 来源（被击杀的敌人 Actor） */
	UPROPERTY(BlueprintReadOnly, Category = "XP")
	AActor* Source;

	FXPGainedEvent()
		: XPAmount(0)
		, Source(nullptr)
	{}
};

/**
 * 等级提升事件
 * 当玩家 XP 达到阈值触发升级时发布
 */
USTRUCT(BlueprintType)
struct AURACORE_API FLevelUpEvent : public FAuraEvent
{
	GENERATED_BODY()
	
	/** 升级后的新等级 */
	UPROPERTY(BlueprintReadOnly, Category = "Level")
	int32 NewLevel;
	
	/** 本次升级获得的属性点数量 */
	UPROPERTY(BlueprintReadOnly, Category = "Level")
	int32 AttributePointsGained;
	
	/** 本次升级获得的技能点数量 */
	UPROPERTY(BlueprintReadOnly, Category = "Level")
	int32 SpellPointsGained;
	
	/** 升级的角色 Actor */
	UPROPERTY(BlueprintReadOnly, Category = "Level")
	AActor* Character;

	FLevelUpEvent()
		: NewLevel(0)
		, AttributePointsGained(0)
		, SpellPointsGained(0)
		, Character(nullptr)
	{}
};

/**
 * 角色死亡事件
 * 当角色生命值降为 0 触发死亡时发布
 */
USTRUCT(BlueprintType)
struct AURACORE_API FCharacterDeathEvent : public FAuraEvent
{
	GENERATED_BODY()
	
	/** 死亡的角色 Actor */
	UPROPERTY(BlueprintReadOnly, Category = "Death")
	AActor* DeadCharacter;
	
	/** 造成致命伤害的攻击者 Actor（可为 nullptr，如环境伤害） */
	UPROPERTY(BlueprintReadOnly, Category = "Death")
	AActor* Killer;

	FCharacterDeathEvent()
		: DeadCharacter(nullptr)
		, Killer(nullptr)
	{}
};

/**
 * 技能装备事件
 * 当玩家将技能装备到输入槽位时发布
 */
USTRUCT(BlueprintType)
struct AURACORE_API FAbilityEquippedEvent : public FAuraEvent
{
	GENERATED_BODY()
	
	/** 被装备的技能标签 */
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayTag AbilityTag;
	
	/** 新的槽位标签（技能被装备到的槽位） */
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayTag SlotTag;
	
	/** 原来的槽位标签（技能从哪个槽位移动过来，空 Tag 表示首次装备） */
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayTag PreviousSlotTag;

	FAbilityEquippedEvent()
	{}
};
