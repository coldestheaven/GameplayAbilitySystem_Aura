// Copyright Druid Mechanics


#include "UI/WidgetController/SpellMenuWidgetController.h"

#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/AuraPlayerState.h"

/**
 * 广播初始值（重写基类）
 * 
 * 实现流程：
 * 1. 广播所有技能信息（BroadcastAbilityInfo）
 * 2. 广播技能点数量
 * 
 * 使用场景：
 * - WidgetController 创建后调用，确保 UI 显示正确的初始状态
 */
void USpellMenuWidgetController::BroadcastInitialValues()
{
	BroadcastAbilityInfo();
	SpellPointsChanged.Broadcast(GetAuraPS()->GetSpellPoints());
}

/**
 * 绑定回调到依赖项（重写基类）
 * 
 * 实现流程：
 * 1. 绑定技能状态变化回调：
 *    - 如果变化的是当前选中技能，更新选中技能状态并刷新按钮状态
 *    - 广播技能信息更新（更新技能列表中的显示）
 * 2. 绑定技能装备回调（OnAbilityEquipped）
 * 3. 绑定技能点变化回调：
 *    - 广播技能点数量变化
 *    - 更新当前选中技能的按钮状态
 * 
 * 使用场景：
 * - WidgetController 创建后调用
 * 
 * 注意：
 * - 技能状态变化会影响按钮的启用状态（升级按钮、装备按钮）
 */
void USpellMenuWidgetController::BindCallbacksToDependencies()
{
	// 绑定技能状态变化回调
	GetAuraASC()->AbilityStatusChanged.AddLambda([this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 NewLevel)
	{
		// 如果变化的是当前选中技能，更新选中技能状态并刷新按钮
		if (SelectedAbility.Ability.MatchesTagExact(AbilityTag))
		{
			SelectedAbility.Status = StatusTag;
			bool bEnableSpendPoints = false;
			bool bEnableEquip = false;
			ShouldEnableButtons(StatusTag, CurrentSpellPoints, bEnableSpendPoints, bEnableEquip);
			FString Description;
			FString NextLevelDescription;
			GetAuraASC()->GetDescriptionsByAbilityTag(AbilityTag, Description, NextLevelDescription);
			SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip, Description, NextLevelDescription);
		}
		
		// 广播技能信息更新（更新技能列表中的显示）
		if (AbilityInfo)
		{
			FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
			Info.StatusTag = StatusTag;
			AbilityInfoDelegate.Broadcast(Info);
		}
	});

	// 绑定技能装备回调
	GetAuraASC()->AbilityEquipped.AddUObject(this, &USpellMenuWidgetController::OnAbilityEquipped);

	// 绑定技能点变化回调
	GetAuraPS()->OnSpellPointsChangedDelegate.AddLambda([this](int32 SpellPoints)
	{
		SpellPointsChanged.Broadcast(SpellPoints);
		CurrentSpellPoints = SpellPoints;

		// 更新当前选中技能的按钮状态
		bool bEnableSpendPoints = false;
		bool bEnableEquip = false;
		ShouldEnableButtons(SelectedAbility.Status, CurrentSpellPoints, bEnableSpendPoints, bEnableEquip);
		FString Description;
		FString NextLevelDescription;
		GetAuraASC()->GetDescriptionsByAbilityTag(SelectedAbility.Ability, Description, NextLevelDescription);
		SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip, Description, NextLevelDescription);
	});
}

void USpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
	if (bWaitingForEquipSelection)
	{
		const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType;
		StopWaitingForEquipDelegate.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}
	
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();	
	const int32 SpellPoints = GetAuraPS()->GetSpellPoints();
	FGameplayTag AbilityStatus;	
	
	const bool bTagValid = AbilityTag.IsValid();
	const bool bTagNone = AbilityTag.MatchesTag(GameplayTags.Abilities_None);
	const FGameplayAbilitySpec* AbilitySpec = GetAuraASC()->GetSpecFromAbilityTag(AbilityTag);
	const bool bSpecValid = AbilitySpec != nullptr;
	if (!bTagValid || bTagNone || !bSpecValid)
	{
		AbilityStatus = GameplayTags.Abilities_Status_Locked;
	}
	else
	{
		AbilityStatus = GetAuraASC()->GetStatusFromSpec(*AbilitySpec);
	}

	SelectedAbility.Ability = AbilityTag;
	SelectedAbility.Status = AbilityStatus;
	bool bEnableSpendPoints = false;
	bool bEnableEquip = false;
	ShouldEnableButtons(AbilityStatus, SpellPoints, bEnableSpendPoints, bEnableEquip);
	FString Description;
	FString NextLevelDescription;
	GetAuraASC()->GetDescriptionsByAbilityTag(AbilityTag, Description, NextLevelDescription);
	SpellGlobeSelectedDelegate.Broadcast(bEnableSpendPoints, bEnableEquip, Description, NextLevelDescription);
}

/**
 * 升级按钮按下（消耗技能点升级技能）
 * 
 * 实现流程：
 * 1. 调用 ASC 的 ServerSpendSpellPoint（服务端 RPC）
 * 
 * 使用场景：
 * - 玩家点击技能菜单中的升级按钮时调用
 * 
 * 网络同步说明：
 * - 通过 Server RPC 在服务端执行，确保权威性
 */
void USpellMenuWidgetController::SpendPointButtonPressed()
{
	if (GetAuraASC())
	{
		GetAuraASC()->ServerSpendSpellPoint(SelectedAbility.Ability);
	}
}

/**
 * 取消选中技能球体
 * 
 * 实现流程：
 * 1. 如果正在等待装备选择，停止等待
 * 2. 重置选中技能为 None 和 Locked 状态
 * 3. 广播取消选中委托（禁用所有按钮，清空描述）
 * 
 * 使用场景：
 * - 玩家点击空白区域或关闭技能菜单时调用
 */
void USpellMenuWidgetController::GlobeDeselect()
{
	if (bWaitingForEquipSelection)
	{
		const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
		StopWaitingForEquipDelegate.Broadcast(SelectedAbilityType);
		bWaitingForEquipSelection = false;
	}
	
	SelectedAbility.Ability = FAuraGameplayTags::Get().Abilities_None;
	SelectedAbility.Status = FAuraGameplayTags::Get().Abilities_Status_Locked;

	SpellGlobeSelectedDelegate.Broadcast(false, false, FString(), FString());
}

/**
 * 装备按钮按下（开始装备流程）
 * 
 * 实现流程：
 * 1. 获取技能类型（攻击/防御/被动）
 * 2. 广播等待装备委托（高亮对应类型的技能槽）
 * 3. 设置等待装备选择标志
 * 4. 如果技能已装备，记录当前槽位（用于重新装备）
 * 
 * 使用场景：
 * - 玩家点击技能菜单中的装备按钮时调用
 * 
 * 注意：
 * - 装备流程需要玩家点击技能槽来完成
 * - 不同类型的技能只能装备到对应类型的槽位
 */
void USpellMenuWidgetController::EquipButtonPressed()
{
	const FGameplayTag AbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;

	WaitForEquipDelegate.Broadcast(AbilityType);
	bWaitingForEquipSelection = true;

	// 如果技能已装备，记录当前槽位
	const FGameplayTag SelectedStatus = GetAuraASC()->GetStatusFromAbilityTag(SelectedAbility.Ability);
	if (SelectedStatus.MatchesTagExact(FAuraGameplayTags::Get().Abilities_Status_Equipped))
	{
		SelectedSlot = GetAuraASC()->GetSlotFromAbilityTag(SelectedAbility.Ability);
	}
}

/**
 * 技能槽球体按下（完成装备）
 * 
 * 实现流程：
 * 1. 检查是否正在等待装备选择
 * 2. 检查选中技能类型是否匹配槽位类型（防止将攻击技能装备到被动槽）
 * 3. 调用 ASC 的 ServerEquipAbility（服务端 RPC）
 * 
 * @param SlotTag 技能槽标签（如 InputTag_LMB、InputTag_RMB 等）
 * @param AbilityType 技能槽类型（攻击/防御/被动）
 * 
 * 使用场景：
 * - 玩家在等待装备选择时点击技能槽时调用
 * 
 * 注意：
 * - 技能类型必须匹配槽位类型，否则无法装备
 */
void USpellMenuWidgetController::SpellRowGlobePressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityType)
{
	if (!bWaitingForEquipSelection) return;
	
	// 检查选中技能类型是否匹配槽位类型（防止将攻击技能装备到被动槽）
	const FGameplayTag& SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
	if (!SelectedAbilityType.MatchesTagExact(AbilityType)) return;

	GetAuraASC()->ServerEquipAbility(SelectedAbility.Ability, SlotTag);
}

/**
 * 技能装备完成回调
 * 
 * 实现流程：
 * 1. 停止等待装备选择
 * 2. 如果技能之前已装备在其他槽位：
 *    - 广播旧槽位信息（状态为 Unlocked，技能为 None）
 * 3. 广播新装备的技能信息（更新技能槽显示）
 * 4. 停止等待装备委托（取消高亮）
 * 5. 广播技能重新分配委托（更新技能列表）
 * 6. 取消选中（清空选中状态）
 * 
 * @param AbilityTag 被装备的技能标签
 * @param Status 技能状态（通常为 Equipped）
 * @param Slot 新槽位标签
 * @param PreviousSlot 旧槽位标签（如果之前已装备）
 * 
 * 使用场景：
 * - 技能装备完成后由 ASC 调用
 */
void USpellMenuWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot)
{
	bWaitingForEquipSelection = false;

	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	
	// 如果技能之前已装备在其他槽位，广播旧槽位信息（清空旧槽位）
	FAuraAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = GameplayTags.Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PreviousSlot;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	// 广播新装备的技能信息（更新新槽位显示）
	FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = Status;
	Info.InputTag = Slot;
	AbilityInfoDelegate.Broadcast(Info);

	StopWaitingForEquipDelegate.Broadcast(AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType);
	SpellGlobeReassignedDelegate.Broadcast(AbilityTag);
	GlobeDeselect();
}

/**
 * 判断按钮是否应该启用
 * 
 * 实现流程：
 * 1. 初始化两个按钮为禁用状态
 * 2. 根据技能状态判断：
 *    - Equipped（已装备）：装备按钮启用，如果有技能点则升级按钮也启用
 *    - Eligible（可升级）：如果有技能点则升级按钮启用
 *    - Unlocked（已解锁）：装备按钮启用，如果有技能点则升级按钮也启用
 *    - Locked（锁定）：两个按钮都禁用
 * 
 * @param AbilityStatus 技能状态标签
 * @param SpellPoints 当前技能点数量
 * @param bShouldEnableSpellPointsButton 输出的升级按钮启用状态
 * @param bShouldEnableEquipButton 输出的装备按钮启用状态
 * 
 * 使用场景：
 * - 技能选中时判断按钮状态
 * - 技能状态或技能点变化时更新按钮状态
 */
void USpellMenuWidgetController::ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints, bool& bShouldEnableSpellPointsButton, bool& bShouldEnableEquipButton)
{
	const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();

	bShouldEnableSpellPointsButton = false;
	bShouldEnableEquipButton = false;
	
	if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Equipped))
	{
		// 已装备：可以重新装备，可以升级
		bShouldEnableEquipButton = true;
		if (SpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
	{
		// 可升级：只能升级
		if (SpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
	{
		// 已解锁：可以装备，可以升级
		bShouldEnableEquipButton = true;
		if (SpellPoints > 0)
		{
			bShouldEnableSpellPointsButton = true;
		}
	}
	// Locked 状态：两个按钮都禁用（默认值）
}
