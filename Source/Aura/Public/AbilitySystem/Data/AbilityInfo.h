// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "AbilityInfo.generated.h"

class UGameplayAbility;

/**
 * 单个技能的 UI 信息结构体
 * 存储技能在菜单中显示所需的所有 UI 数据
 *
 * 使用说明：
 * - AbilityTag/CooldownTag/AbilityType 在数据资产中配置（EditDefaultsOnly）
 * - InputTag/StatusTag 在运行时由 ASC 填充（BlueprintReadOnly，无 EditDefaultsOnly）
 * - 通过 UAbilityInfo::FindAbilityInfoForTag 按 AbilityTag 查找
 */
USTRUCT(BlueprintType)
struct FAuraAbilityInfo
{
	GENERATED_BODY()

	/** 技能唯一标识标签（如 Abilities.Fire.FireBolt），用于查找和匹配 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityTag = FGameplayTag();

	/**
	 * 技能绑定的输入标签（运行时填充）
	 * 由 ASC 在广播技能信息时填入当前装备的槽位标签
	 * 用于技能栏 Widget 显示技能绑定的按键
	 */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag InputTag = FGameplayTag();

	/**
	 * 技能当前状态标签（运行时填充）
	 * 由 ASC 在广播技能信息时填入（Locked/Eligible/Unlocked/Equipped）
	 * 用于技能菜单 Widget 显示技能的解锁状态
	 */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag StatusTag = FGameplayTag();

	/** 冷却 GameplayTag（如 Cooldown.Fire.FireBolt），用于监听冷却状态 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag CooldownTag = FGameplayTag();

	/** 技能类型标签（Abilities.Type.Offensive/Passive），决定装备到哪类槽位 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag AbilityType = FGameplayTag();

	/** 技能图标纹理（在技能球和技能栏中显示） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UTexture2D> Icon = nullptr;

	/** 技能背景材质（技能球的背景颜色/材质，区分不同技能类型） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<const UMaterialInterface> BackgroundMaterial = nullptr;

	/** 解锁所需等级（玩家达到此等级后技能状态从 Locked 变为 Eligible） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 LevelRequirement = 1;

	/** 技能类（用于在 ASC 中查找对应的技能 Spec） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayAbility> Ability;
};

/**
 * 技能信息数据资产
 *
 * 存储游戏中所有技能的 UI 显示信息
 * 在 GameMode 的 Details 面板中指定，通过 UAuraAbilitySystemLibrary::GetAbilityInfo 全局访问
 *
 * 使用方式：
 *   // 根据技能标签查找技能信息
 *   FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
 *   // 使用信息更新 UI
 *   AbilityIcon->SetBrushFromTexture(Info.Icon);
 */
UCLASS()
class AURA_API UAbilityInfo : public UDataAsset
{
	GENERATED_BODY()
public:

	/** 所有技能的信息数组（在 Details 面板中配置每个技能的 UI 数据） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AbilityInformation")
	TArray<FAuraAbilityInfo> AbilityInformation;

	/**
	 * 根据技能标签查找对应的技能信息
	 * @param AbilityTag   要查找的技能标签
	 * @param bLogNotFound 未找到时是否输出警告日志
	 * @return 对应的 FAuraAbilityInfo 结构体（未找到则返回空结构体）
	 */
	FAuraAbilityInfo FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound = false) const;
};
