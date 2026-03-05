// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameplayTags/AuraGameplayTags.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "GameplayTagContainer.h"
#include "SpellMenuWidgetController.generated.h"

/**
 * 技能球选中时广播的委托
 * 携带按钮状态和技能描述信息，用于更新技能菜单的操作按钮和描述面板
 * @param bSpendPointsButtonEnabled  "消耗技能点"按钮是否可用
 * @param bEquipButtonEnabled        "装备"按钮是否可用
 * @param DescriptionString          当前等级技能描述文本
 * @param NextLevelDescriptionString 下一等级技能描述文本
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSpellGlobeSelectedSignature, bool, bSpendPointsButtonEnabled, bool, bEquipButtonEnabled, FString, DescriptionString, FString, NextLevelDescriptionString);

/**
 * 等待装备槽位选择时广播的委托
 * 当玩家点击"装备"按钮后，进入等待选择槽位的状态，广播此委托通知 Widget 高亮可用槽位
 * @param AbilityType 要装备的技能类型（主动/被动），用于过滤可用槽位
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitForEquipSelectionSignature, const FGameplayTag&, AbilityType);

/**
 * 技能球重新分配时广播的委托
 * 当技能被移动到新槽位后，通知原槽位的技能球更新显示
 * @param AbilityTag 被重新分配的技能标签
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpellGlobeReassignedSignature, const FGameplayTag&, AbilityTag);

/**
 * 当前选中技能的状态结构体
 * 记录技能菜单中当前选中的技能及其状态
 */
struct FSelectedAbility
{
	/** 当前选中的技能标签（默认为 Abilities_None） */
	FGameplayTag Ability = FGameplayTag();

	/** 当前选中技能的状态标签（默认为 Abilities_Status_Locked） */
	FGameplayTag Status = FGameplayTag();
};

/**
 * 技能菜单 Widget 控制器
 *
 * 职责：
 * - 管理技能菜单的交互逻辑（选中、消耗技能点、装备、取消选中）
 * - 维护当前选中技能的状态（SelectedAbility）
 * - 处理技能装备流程（点击装备按钮 → 等待选择槽位 → 确认装备）
 * - 广播按钮状态和技能描述给 Widget
 *
 * 技能装备流程：
 *   1. 玩家点击技能球 → SpellGlobeSelected → 广播按钮状态和描述
 *   2. 玩家点击"装备"按钮 → EquipButtonPressed → 进入等待槽位选择状态
 *   3. 玩家点击槽位 → SpellRowGlobePressed → 服务端装备技能
 *   4. 服务端确认 → OnAbilityEquipped → 广播更新 UI
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API USpellMenuWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()
public:
	/**
	 * 广播初始技能信息（重写基类）
	 * 广播所有已解锁技能的信息和当前技能点数量
	 */
	virtual void BroadcastInitialValues() override;

	/**
	 * 绑定回调到依赖项（重写基类）
	 * 绑定 ASC 技能状态变化、技能装备等事件的回调
	 */
	virtual void BindCallbacksToDependencies() override;

	/**
	 * 技能点数量变化时广播（技能点显示 Widget 绑定此委托）
	 */
	UPROPERTY(BlueprintAssignable)
	FOnPlayerStatChangedSignature SpellPointsChanged;

	/**
	 * 技能球被选中时广播
	 * 携带按钮可用状态和技能描述，Widget 根据此更新操作面板
	 */
	UPROPERTY(BlueprintAssignable)
	FSpellGlobeSelectedSignature SpellGlobeSelectedDelegate;

	/**
	 * 进入等待装备槽位选择状态时广播
	 * Widget 收到此委托后高亮对应类型的槽位行
	 */
	UPROPERTY(BlueprintAssignable)
	FWaitForEquipSelectionSignature WaitForEquipDelegate;

	/**
	 * 退出等待装备槽位选择状态时广播
	 * Widget 收到此委托后取消槽位高亮
	 */
	UPROPERTY(BlueprintAssignable)
	FWaitForEquipSelectionSignature StopWaitingForEquipDelegate;

	/**
	 * 技能被重新分配到新槽位时广播
	 * 通知原槽位的技能球清除显示
	 */
	UPROPERTY(BlueprintAssignable)
	FSpellGlobeReassignedSignature SpellGlobeReassignedDelegate;

	/**
	 * 玩家选中技能球时调用（蓝图可调用）
	 * 更新 SelectedAbility，计算按钮状态，广播技能描述
	 * @param AbilityTag 被选中的技能标签
	 */
	UFUNCTION(BlueprintCallable)
	void SpellGlobeSelected(const FGameplayTag& AbilityTag);

	/**
	 * 玩家点击"消耗技能点"按钮时调用（蓝图可调用）
	 * 向服务端发送 ServerSpendSpellPoint RPC
	 */
	UFUNCTION(BlueprintCallable)
	void SpendPointButtonPressed();

	/**
	 * 玩家取消选中技能球时调用（蓝图可调用）
	 * 清空 SelectedAbility，重置按钮状态
	 */
	UFUNCTION(BlueprintCallable)
	void GlobeDeselect();

	/**
	 * 玩家点击"装备"按钮时调用（蓝图可调用）
	 * 进入等待槽位选择状态，广播 WaitForEquipDelegate
	 */
	UFUNCTION(BlueprintCallable)
	void EquipButtonPressed();

	/**
	 * 玩家点击技能栏槽位时调用（蓝图可调用）
	 * 向服务端发送 ServerEquipAbility RPC，将选中技能装备到此槽位
	 * @param SlotTag     被点击的槽位标签
	 * @param AbilityType 槽位对应的技能类型（主动/被动）
	 */
	UFUNCTION(BlueprintCallable)
	void SpellRowGlobePressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityType);

	/**
	 * 技能装备完成回调（绑定到 ASC 的 AbilityEquipped 委托）
	 * 退出等待槽位选择状态，广播 SpellGlobeReassignedDelegate 更新 UI
	 * @param AbilityTag   已装备的技能标签
	 * @param Status       技能当前状态
	 * @param Slot         新槽位
	 * @param PreviousSlot 原槽位
	 */
	void OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot);

private:
	/**
	 * 根据技能状态和技能点数量计算按钮可用状态
	 * @param AbilityStatus                技能当前状态标签
	 * @param SpellPoints                  当前可用技能点数量
	 * @param bShouldEnableSpellPointsButton 输出：是否启用"消耗技能点"按钮
	 * @param bShouldEnableEquipButton       输出：是否启用"装备"按钮
	 */
	static void ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints, bool& bShouldEnableSpellPointsButton, bool& bShouldEnableEquipButton);

	/** 当前选中的技能（标签和状态），默认为未选中状态 */
	FSelectedAbility SelectedAbility = { FAuraGameplayTags::Get().Abilities_None, FAuraGameplayTags::Get().Abilities_Status_Locked };

	/** 当前可用技能点数量（从 PlayerState 同步，用于计算按钮状态） */
	int32 CurrentSpellPoints = 0;

	/** 是否正在等待玩家选择装备槽位（点击"装备"按钮后进入此状态） */
	bool bWaitingForEquipSelection = false;

	/** 当前选中的槽位标签（在等待装备状态下记录玩家选择的槽位） */
	FGameplayTag SelectedSlot;
};

