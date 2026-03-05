// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AuraAbilitySystemComponent.generated.h"

class ULoadScreenSaveGame;

/** 当 GE 被应用时广播，携带该 GE 的所有 AssetTags（用于触发 UI 消息提示） */
DECLARE_MULTICAST_DELEGATE_OneParam(FEffectAssetTags, const FGameplayTagContainer& /*AssetTags*/);

/** 初始技能赋予完成时广播（用于通知 UI 技能列表已就绪） */
DECLARE_MULTICAST_DELEGATE(FAbilitiesGiven);

/** 遍历所有技能时的回调委托（单播，用于 ForEachAbility 函数） */
DECLARE_DELEGATE_OneParam(FForEachAbility, const FGameplayAbilitySpec&);

/** 技能状态变化时广播（解锁、升级等，用于更新技能菜单 UI） */
DECLARE_MULTICAST_DELEGATE_ThreeParams(FAbilityStatusChanged, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /*StatusTag*/, int32 /*AbilityLevel*/);

/** 技能装备到槽位时广播（用于更新技能栏 UI） */
DECLARE_MULTICAST_DELEGATE_FourParams(FAbilityEquipped, const FGameplayTag& /*AbilityTag*/, const FGameplayTag& /*Status*/, const FGameplayTag& /*Slot*/, const FGameplayTag& /*PrevSlot*/);

/** 停用被动技能时广播（用于关闭对应的被动特效） */
DECLARE_MULTICAST_DELEGATE_OneParam(FDeactivatePassiveAbility, const FGameplayTag& /*AbilityTag*/);

/** 激活/停用被动特效时广播（用于控制 PassiveNiagaraComponent 的开关） */
DECLARE_MULTICAST_DELEGATE_TwoParams(FActivatePassiveEffect, const FGameplayTag& /*AbilityTag*/, bool /*bActivate*/);

/**
 * Aura 游戏的能力系统组件（ASC）
 *
 * 扩展了 UAbilitySystemComponent，添加了以下功能：
 * 1. 技能输入处理（Pressed/Held/Released 三种输入状态）
 * 2. 技能状态管理（锁定/可解锁/已解锁/已装备）
 * 3. 技能槽位管理（将技能分配到输入槽）
 * 4. 存档数据加载（从 SaveGame 恢复技能状态）
 * 5. 属性升级（通过 Server RPC 安全地升级属性）
 * 6. 被动技能特效同步（通过 Multicast RPC 同步特效状态）
 *
 * 使用示例：
 *   // 绑定技能输入
 *   AuraASC->AbilityInputTagPressed(InputTag);
 *   // 升级属性
 *   AuraASC->UpgradeAttribute(StrengthTag);
 *   // 遍历所有技能
 *   AuraASC->ForEachAbility([](const FGameplayAbilitySpec& Spec){ ... });
 */
UCLASS()
class AURA_API UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	/**
	 * 初始化 AbilityActorInfo 后调用
	 * 绑定 GE 应用回调（ClientEffectApplied），用于广播 EffectAssetTags
	 */
	void AbilityActorInfoSet();

	/** GE 应用时广播其 AssetTags（客户端监听，用于显示消息 Widget） */
	FEffectAssetTags EffectAssetTags;

	/** 初始技能赋予完成时广播（UI 可监听此委托来初始化技能列表） */
	FAbilitiesGiven AbilitiesGivenDelegate;

	/** 技能状态变化时广播（技能菜单 UI 监听此委托更新显示） */
	FAbilityStatusChanged AbilityStatusChanged;

	/** 技能装备到槽位时广播（技能栏 UI 监听此委托更新图标） */
	FAbilityEquipped AbilityEquipped;

	/** 停用被动技能时广播（PassiveNiagaraComponent 监听此委托关闭特效） */
	FDeactivatePassiveAbility DeactivatePassiveAbility;

	/** 激活/停用被动特效时广播（PassiveNiagaraComponent 监听此委托开关特效） */
	FActivatePassiveEffect ActivatePassiveEffect;

	/**
	 * 从存档数据加载并赋予技能
	 * 恢复技能的状态（已解锁/已装备）、等级和槽位绑定
	 * @param SaveData 存档数据对象
	 */
	void AddCharacterAbilitiesFromSaveData(ULoadScreenSaveGame* SaveData);

	/**
	 * 赋予角色初始主动技能列表
	 * @param StartupAbilities 要赋予的技能类数组
	 */
	void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);

	/**
	 * 赋予角色初始被动技能列表（赋予后立即激活）
	 * @param StartupPassiveAbilities 要赋予的被动技能类数组
	 */
	void AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities);

	/** 初始技能是否已赋予完成（防止重复赋予） */
	bool bStartupAbilitiesGiven = false;

	/**
	 * 处理技能输入按下事件
	 * 对匹配 InputTag 的技能调用 TryActivateAbility（如果技能未激活）
	 * @param InputTag 按下的输入标签
	 */
	void AbilityInputTagPressed(const FGameplayTag& InputTag);

	/**
	 * 处理技能输入持续按住事件
	 * 对匹配 InputTag 的已激活技能调用 InputPressed
	 * @param InputTag 持续按住的输入标签
	 */
	void AbilityInputTagHeld(const FGameplayTag& InputTag);

	/**
	 * 处理技能输入释放事件
	 * 对匹配 InputTag 的已激活技能调用 InputReleased
	 * @param InputTag 释放的输入标签
	 */
	void AbilityInputTagReleased(const FGameplayTag& InputTag);

	/**
	 * 遍历所有已赋予的技能并执行回调
	 * 线程安全地锁定技能列表后遍历，避免遍历过程中列表被修改
	 * @param Delegate 对每个技能 Spec 执行的回调函数
	 */
	void ForEachAbility(const FForEachAbility& Delegate);

	/**
	 * 从技能 Spec 中获取技能标签（Abilities.xxx 类型的标签）
	 * @param AbilitySpec 技能规格
	 * @return 技能的 GameplayTag，未找到则返回空 Tag
	 */
	static FGameplayTag GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);

	/**
	 * 从技能 Spec 中获取输入标签（InputTag.xxx 类型的标签）
	 * @param AbilitySpec 技能规格
	 * @return 技能绑定的输入 Tag，未找到则返回空 Tag
	 */
	static FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec);

	/**
	 * 从技能 Spec 中获取状态标签（Abilities.Status.xxx 类型的标签）
	 * @param AbilitySpec 技能规格
	 * @return 技能当前状态 Tag（Locked/Eligible/Unlocked/Equipped）
	 */
	static FGameplayTag GetStatusFromSpec(const FGameplayAbilitySpec& AbilitySpec);

	/**
	 * 根据技能标签查找对应的状态标签
	 * @param AbilityTag 要查询的技能标签
	 * @return 技能当前状态 Tag
	 */
	FGameplayTag GetStatusFromAbilityTag(const FGameplayTag& AbilityTag);

	/**
	 * 根据技能标签查找当前装备的槽位标签
	 * @param AbilityTag 要查询的技能标签
	 * @return 技能当前所在槽位的 Tag，未装备则返回空 Tag
	 */
	FGameplayTag GetSlotFromAbilityTag(const FGameplayTag& AbilityTag);

	/**
	 * 检查指定槽位是否为空（没有技能装备在此槽位）
	 * @param Slot 要检查的槽位 Tag
	 * @return true 表示槽位为空
	 */
	bool SlotIsEmpty(const FGameplayTag& Slot);

	/**
	 * 检查技能 Spec 是否装备在指定槽位
	 * @param Spec 技能规格
	 * @param Slot 槽位 Tag
	 */
	static bool AbilityHasSlot(const FGameplayAbilitySpec& Spec, const FGameplayTag& Slot);

	/**
	 * 检查技能 Spec 是否装备在任意槽位
	 * @param Spec 技能规格
	 * @return true 表示已装备到某个槽位
	 */
	static bool AbilityHasAnySlot(const FGameplayAbilitySpec& Spec);

	/**
	 * 获取装备在指定槽位的技能 Spec 指针
	 * @param Slot 槽位 Tag
	 * @return 对应的技能 Spec 指针，未找到则返回 nullptr
	 */
	FGameplayAbilitySpec* GetSpecWithSlot(const FGameplayTag& Slot);

	/**
	 * 判断技能是否为被动技能（通过检查技能的 AbilityType 标签）
	 * @param Spec 技能规格
	 * @return true 表示是被动技能
	 */
	bool IsPassiveAbility(const FGameplayAbilitySpec& Spec) const;

	/**
	 * 将技能分配到指定槽位（修改 Spec 的 DynamicAbilityTags）
	 * @param Spec 要修改的技能规格（引用）
	 * @param Slot 目标槽位 Tag
	 */
	static void AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot);

	/**
	 * 多播 RPC：在所有客户端激活/停用被动特效
	 * 由服务端调用，确保所有客户端的被动特效状态同步
	 * @param AbilityTag 被动技能标签
	 * @param bActivate  true 激活特效，false 停用特效
	 */
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastActivatePassiveEffect(const FGameplayTag& AbilityTag, bool bActivate);

	/**
	 * 根据技能标签查找技能 Spec 指针
	 * @param AbilityTag 要查找的技能标签
	 * @return 对应的技能 Spec 指针，未找到则返回 nullptr
	 */
	FGameplayAbilitySpec* GetSpecFromAbilityTag(const FGameplayTag& AbilityTag);

	/**
	 * 升级指定属性（本地调用，内部转发到 Server RPC）
	 * @param AttributeTag 要升级的属性标签（如 Attributes.Primary.Strength）
	 */
	void UpgradeAttribute(const FGameplayTag& AttributeTag);

	/**
	 * 服务端 RPC：安全地升级属性
	 * 验证玩家有足够的属性点后，应用属性升级 GE 并扣除属性点
	 * @param AttributeTag 要升级的属性标签
	 */
	UFUNCTION(Server, Reliable)
	void ServerUpgradeAttribute(const FGameplayTag& AttributeTag);

	/**
	 * 根据角色等级更新所有技能的解锁状态
	 * 遍历所有技能，将满足等级要求的 Locked 技能升级为 Eligible 状态
	 * @param Level 当前角色等级
	 */
	void UpdateAbilityStatuses(int32 Level);

	/**
	 * 服务端 RPC：消耗技能点解锁/升级技能
	 * 验证玩家有足够的技能点后，将技能状态从 Eligible 升级为 Unlocked，或提升技能等级
	 * @param AbilityTag 要解锁/升级的技能标签
	 */
	UFUNCTION(Server, Reliable)
	void ServerSpendSpellPoint(const FGameplayTag& AbilityTag);

	/**
	 * 服务端 RPC：将技能装备到指定槽位
	 * 处理槽位冲突（将原槽位技能移除或交换），然后将技能分配到新槽位
	 * @param AbilityTag 要装备的技能标签
	 * @param Slot       目标槽位 Tag
	 */
	UFUNCTION(Server, Reliable)
	void ServerEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Slot);

	/**
	 * 客户端 RPC：通知客户端技能装备完成
	 * 广播 AbilityEquipped 委托，更新技能栏 UI
	 * @param AbilityTag    已装备的技能标签
	 * @param Status        技能当前状态
	 * @param Slot          新槽位
	 * @param PreviousSlot  原槽位（用于清除原槽位 UI）
	 */
	UFUNCTION(Client, Reliable)
	void ClientEquipAbility(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot);

	/**
	 * 获取技能的描述文本（当前等级和下一等级）
	 * @param AbilityTag              要查询的技能标签
	 * @param OutDescription          输出：当前等级描述
	 * @param OutNextLevelDescription 输出：下一等级描述
	 * @return true 表示成功获取描述
	 */
	bool GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription, FString& OutNextLevelDescription);

	/**
	 * 清除技能 Spec 的槽位绑定（移除所有 InputTag 类型的动态标签）
	 * @param Spec 要清除槽位的技能 Spec 指针
	 */
	static void ClearSlot(FGameplayAbilitySpec* Spec);

	/**
	 * 清除指定槽位上的所有技能绑定
	 * 遍历所有技能，将装备在此槽位的技能的槽位标签移除
	 * @param Slot 要清除的槽位 Tag
	 */
	void ClearAbilitiesOfSlot(const FGameplayTag& Slot);

	/**
	 * 检查技能 Spec 指针是否装备在指定槽位（指针版本重载）
	 * @param Spec 技能 Spec 指针
	 * @param Slot 槽位 Tag
	 */
	static bool AbilityHasSlot(FGameplayAbilitySpec* Spec, const FGameplayTag& Slot);

protected:
	/**
	 * 技能激活状态同步回调
	 * 当技能列表同步到客户端后调用，广播 AbilitiesGivenDelegate 通知 UI
	 */
	virtual void OnRep_ActivateAbilities() override;

	/**
	 * 客户端 RPC：GE 被应用时通知客户端
	 * 广播 EffectAssetTags 委托，触发 UI 消息显示（如拾取物品提示）
	 * @param AbilitySystemComponent 应用 GE 的 ASC
	 * @param EffectSpec             被应用的 GE 规格
	 * @param ActiveEffectHandle     激活的 GE 句柄
	 */
	UFUNCTION(Client, Reliable)
	void ClientEffectApplied(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayEffectSpec& EffectSpec, FActiveGameplayEffectHandle ActiveEffectHandle);

	/**
	 * 客户端 RPC：通知客户端技能状态已更新
	 * 广播 AbilityStatusChanged 委托，更新技能菜单 UI
	 * @param AbilityTag  状态变化的技能标签
	 * @param StatusTag   新的状态标签
	 * @param AbilityLevel 技能当前等级
	 */
	UFUNCTION(Client, Reliable)
	void ClientUpdateAbilityStatus(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel);
};
