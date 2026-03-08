// Copyright Druid Mechanics


#include "AbilitySystem/Passive/PassiveNiagaraComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTags/AuraGameplayTags.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"

/**
 * 构造函数：初始化被动技能 Niagara 组件
 * 
 * 实现流程：
 * 1. 禁用自动激活（等待被动技能装备时激活）
 * 
 * 使用场景：
 * - 组件创建时自动调用
 */
UPassiveNiagaraComponent::UPassiveNiagaraComponent()
{
	bAutoActivate = false;
}

/**
 * 游戏开始时初始化
 * 
 * 实现流程：
 * 1. 调用父类 BeginPlay
 * 2. 尝试获取 AuraASC：
 *    - 如果 ASC 已存在，绑定被动效果激活委托并检查是否已装备
 *    - 如果 ASC 未存在但 Owner 实现了 CombatInterface，等待 ASC 注册后再绑定
 * 
 * 使用场景：
 * - 组件初始化时调用
 * 
 * 注意：
 * - 如果 ASC 还未注册，会等待 ASC 注册完成
 * - 初始化时会检查被动技能是否已装备（如果技能已赋予）
 */
void UPassiveNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();

	// 如果 ASC 已存在，直接绑定委托
	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner())))
	{
		AuraASC->ActivatePassiveEffect.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveActivate);
		ActivateIfEquipped(AuraASC);
	}
	// 如果 ASC 未存在，等待 ASC 注册
	else if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner()))
	{
		CombatInterface->GetOnASCRegisteredDelegate().AddLambda([this](UAbilitySystemComponent* ASC)
		{
			if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner())))
			{
				AuraASC->ActivatePassiveEffect.AddUObject(this, &UPassiveNiagaraComponent::OnPassiveActivate);
				ActivateIfEquipped(AuraASC);
			}
		});
	}
}

/**
 * 如果被动技能已装备，激活特效
 * 
 * 实现流程：
 * 1. 检查技能是否已赋予（bStartupAbilitiesGiven）
 * 2. 如果已赋予，检查被动技能状态是否为 Equipped
 * 3. 如果已装备，激活 Niagara 特效
 * 
 * @param AuraASC Aura AbilitySystemComponent
 * 
 * 使用场景：
 * - 组件初始化时调用（如果技能已赋予）
 * - 确保已装备的被动技能特效正确显示
 */
void UPassiveNiagaraComponent::ActivateIfEquipped(UAuraAbilitySystemComponent* AuraASC)
{
	const bool bStartupAbilitiesGiven = AuraASC->bStartupAbilitiesGiven;
	if (bStartupAbilitiesGiven)
	{
		if (AuraASC->GetStatusFromAbilityTag(PassiveSpellTag) == FAuraGameplayTags::Get().Abilities_Status_Equipped)
		{
			Activate();
		}
	}
}

/**
 * 被动技能激活回调
 * 
 * 实现流程：
 * 1. 检查是否为对应的被动技能标签
 * 2. 如果激活且当前未激活，激活特效
 * 3. 否则停用特效
 * 
 * @param AbilityTag 被动技能标签
 * @param bActivate 是否激活
 * 
 * 使用场景：
 * - 被动技能装备/卸载时调用
 * - 由 AuraASC::ActivatePassiveEffect 委托触发
 */
void UPassiveNiagaraComponent::OnPassiveActivate(const FGameplayTag& AbilityTag, bool bActivate)
{
	if (AbilityTag.MatchesTagExact(PassiveSpellTag))
	{
		if (bActivate && !IsActive())
		{
			Activate();
		}
		else
		{
			Deactivate();
		}
	}
}
