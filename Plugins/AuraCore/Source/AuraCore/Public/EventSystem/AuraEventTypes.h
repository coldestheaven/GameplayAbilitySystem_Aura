// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "AuraEventTypes.generated.h"

/**
 * 事件基类 - 所有事件必须继承此类
 */
USTRUCT(BlueprintType)
struct AURACORE_API FAuraEvent
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Event")
	FGameplayTag EventTag;
	
	UPROPERTY(BlueprintReadOnly, Category = "Event")
	float Timestamp;
	
	FAuraEvent()
		: Timestamp(0.f)
	{}
	
	virtual ~FAuraEvent() = default;
};

/**
 * 属性变化事件
 */
USTRUCT(BlueprintType)
struct AURACORE_API FAttributeChangedEvent : public FAuraEvent
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	FGameplayTag AttributeTag;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	float OldValue;
	
	UPROPERTY(BlueprintReadOnly, Category = "Attribute")
	float NewValue;
	
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
 */
USTRUCT(BlueprintType)
struct AURACORE_API FAbilityActivatedEvent : public FAuraEvent
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayTag AbilityTag;
	
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	AActor* Instigator;
	
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	AActor* Target;

	FAbilityActivatedEvent()
		: Instigator(nullptr)
		, Target(nullptr)
	{}
};

/**
 * 伤害事件
 */
USTRUCT(BlueprintType)
struct AURACORE_API FAuraDamageEvent : public FAuraEvent
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	float Damage;
	
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	bool bCriticalHit;
	
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	bool bBlockedHit;
	
	UPROPERTY(BlueprintReadOnly, Category = "Damage")
	AActor* Attacker;
	
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
 */
USTRUCT(BlueprintType)
struct AURACORE_API FXPGainedEvent : public FAuraEvent
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "XP")
	int32 XPAmount;
	
	UPROPERTY(BlueprintReadOnly, Category = "XP")
	AActor* Source;

	FXPGainedEvent()
		: XPAmount(0)
		, Source(nullptr)
	{}
};

/**
 * 等级提升事件
 */
USTRUCT(BlueprintType)
struct AURACORE_API FLevelUpEvent : public FAuraEvent
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Level")
	int32 NewLevel;
	
	UPROPERTY(BlueprintReadOnly, Category = "Level")
	int32 AttributePointsGained;
	
	UPROPERTY(BlueprintReadOnly, Category = "Level")
	int32 SpellPointsGained;
	
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
 */
USTRUCT(BlueprintType)
struct AURACORE_API FCharacterDeathEvent : public FAuraEvent
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Death")
	AActor* DeadCharacter;
	
	UPROPERTY(BlueprintReadOnly, Category = "Death")
	AActor* Killer;

	FCharacterDeathEvent()
		: DeadCharacter(nullptr)
		, Killer(nullptr)
	{}
};

/**
 * 技能装备事件
 */
USTRUCT(BlueprintType)
struct AURACORE_API FAbilityEquippedEvent : public FAuraEvent
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayTag AbilityTag;
	
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayTag SlotTag;
	
	UPROPERTY(BlueprintReadOnly, Category = "Ability")
	FGameplayTag PreviousSlotTag;

	FAbilityEquippedEvent()
	{}
};
