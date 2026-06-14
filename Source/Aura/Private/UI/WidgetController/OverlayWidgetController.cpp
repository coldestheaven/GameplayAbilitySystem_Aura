// Copyright Druid Mechanics


#include "UI/WidgetController/OverlayWidgetController.h"

#include "GameplayTags/AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Player/AuraPlayerState.h"

/**
 * 广播初始值（重写基类）
 * 
 * 实现流程：
 * 1. 广播生命值初始值
 * 2. 广播最大生命值初始值
 * 3. 广播法力值初始值
 * 4. 广播最大法力值初始值
 * 
 * 使用场景：
 * - WidgetController 创建后调用，确保 UI 显示正确的初始状态
 * - 在绑定回调之前调用，避免初始值丢失
 * 
 * 注意：
 * - 必须在 BindCallbacksToDependencies 之前调用
 * - 确保 UI 在绑定回调前显示正确的初始值
 */
void UOverlayWidgetController::BroadcastInitialValues()
{
	OnHealthChanged.Broadcast(GetAuraAS()->GetHealth());
	OnMaxHealthChanged.Broadcast(GetAuraAS()->GetMaxHealth());
	OnManaChanged.Broadcast(GetAuraAS()->GetMana());
	OnMaxManaChanged.Broadcast(GetAuraAS()->GetMaxMana());
}

/**
 * 绑定回调到依赖项（重写基类）
 * 
 * 实现流程：
 * 1. 绑定 PlayerState 的 XP 和等级变化委托
 * 2. 绑定属性变化委托（生命值、最大生命值、法力值、最大法力值）
 * 3. 绑定技能装备委托
 * 4. 如果技能已赋予，立即广播技能信息；否则等待技能赋予完成
 * 5. 绑定 EffectAssetTags 委托（用于显示消息提示，如使用药水）
 * 
 * 使用场景：
 * - WidgetController 创建后调用
 * - 确保所有 UI 更新回调都已绑定
 * 
 * 注意：
 * - 属性变化委托会在属性值变化时自动触发
 * - EffectAssetTags 用于显示 GameplayEffect 相关的 UI 消息（如药水使用提示）
 */
void UOverlayWidgetController::BindCallbacksToDependencies()
{
	// 绑定 PlayerState 的 XP 和等级变化委托
	GetAuraPS()->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);
	GetAuraPS()->OnLevelChangedDelegate.AddLambda(
		[this](int32 NewLevel, bool bLevelUp)
		{
			OnPlayerLevelChangedDelegate.Broadcast(NewLevel, bLevelUp);
		}
	);
	
	// 绑定属性变化委托（C1 重构 · 2026-06-14：折叠 4 段重复 Lambda 为基类模板调用）
	// 模板内部统一执行：ASC->GetGameplayAttributeValueChangeDelegate(Attr).AddLambda(...Broadcast(NewValue))
	BindAttributeValueChange(GetAuraAS()->GetHealthAttribute(),    OnHealthChanged);
	BindAttributeValueChange(GetAuraAS()->GetMaxHealthAttribute(), OnMaxHealthChanged);
	BindAttributeValueChange(GetAuraAS()->GetManaAttribute(),      OnManaChanged);
	BindAttributeValueChange(GetAuraAS()->GetMaxManaAttribute(),   OnMaxManaChanged);

	// 绑定技能和消息相关委托
	if (GetAuraASC())
	{
		// 绑定技能装备委托
		GetAuraASC()->AbilityEquipped.AddUObject(this, &UOverlayWidgetController::OnAbilityEquipped);
		
		// 如果技能已赋予，立即广播；否则等待技能赋予完成
		if (GetAuraASC()->bStartupAbilitiesGiven)
		{
			BroadcastAbilityInfo();
		}
		else
		{
			GetAuraASC()->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::BroadcastAbilityInfo);
		}

		// 绑定 EffectAssetTags 委托（用于显示消息提示，如使用药水）
		GetAuraASC()->EffectAssetTags.AddLambda(
			[this](const FGameplayTagContainer& AssetTags)
			{
				for (const FGameplayTag& Tag : AssetTags)
				{
					// 检查标签是否匹配 "Message" 父标签
					// 例如：Tag = "Message.HealthPotion"
					// "Message.HealthPotion".MatchesTag("Message") 返回 True
					FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(FName("Message"));
					if (Tag.MatchesTag(MessageTag))
					{
						const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
						MessageWidgetRowDelegate.Broadcast(*Row);
					}
				}
			}
		);
	}
}

/**
 * XP 变化回调处理
 * 
 * 实现流程：
 * 1. 获取 LevelUpInfo 数据资产
 * 2. 根据当前 XP 查找对应等级
 * 3. 计算经验条百分比：
 *    - 当前等级所需 XP = LevelUpRequirement[Level]
 *    - 上一等级所需 XP = LevelUpRequirement[Level - 1]
 *    - 当前等级进度 = NewXP - 上一等级所需 XP
 *    - 当前等级总需求 = 当前等级所需 XP - 上一等级所需 XP
 *    - 经验条百分比 = 当前等级进度 / 当前等级总需求
 * 4. 广播经验条百分比变化
 * 
 * @param NewXP 新的 XP 值
 * 
 * 使用场景：
 * - PlayerState 的 XP 变化时自动调用
 * - 更新 UI 经验条显示
 * 
 * 注意：
 * - 经验条百分比是相对于当前等级的进度（0.0 - 1.0）
 * - 当达到升级阈值时，百分比会重置为 0（进入下一级）
 */
void UOverlayWidgetController::OnXPChanged(int32 NewXP)
{
	const ULevelUpInfo* LevelUpInfo = GetAuraPS()->LevelUpInfo;
	checkf(LevelUpInfo, TEXT("Unabled to find LevelUpInfo. Please fill out AuraPlayerState Blueprint"));

	const int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
	const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();

	if (Level <= MaxLevel && Level > 0)
	{
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;

		const int32 DeltaLevelRequirement = LevelUpRequirement - PreviousLevelUpRequirement;
		const int32 XPForThisLevel = NewXP - PreviousLevelUpRequirement;

		const float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelRequirement);

		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	}
}

void UOverlayWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot) const
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	
	FAuraAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PreviousSlot;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
	// Broadcast empty info if PreviousSlot is a valid slot. Only if equipping an already-equipped spell
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = Status;
	Info.InputTag = Slot;
	AbilityInfoDelegate.Broadcast(Info);
}
