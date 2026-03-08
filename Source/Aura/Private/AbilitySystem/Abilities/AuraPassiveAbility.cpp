// Copyright Druid Mechanics


#include "AbilitySystem/Abilities/AuraPassiveAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"

/**
 * 激活被动技能（重写基类）
 * 
 * 实现流程：
 * 1. 调用父类 ActivateAbility
 * 2. 获取 AuraASC
 * 3. 绑定被动技能停用委托（DeactivatePassiveAbility）
 * 
 * 使用场景：
 * - 被动技能激活时自动调用
 * 
 * 注意：
 * - 被动技能激活后会一直运行，直到被停用
 * - 停用委托用于在技能卸载时停用被动效果
 */
void UAuraPassiveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (UAuraAbilitySystemComponent* AuraASC = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo())))
	{
		AuraASC->DeactivatePassiveAbility.AddUObject(this, &UAuraPassiveAbility::ReceiveDeactivate);
	}
}

/**
 * 接收停用信号
 * 
 * 实现流程：
 * 1. 检查停用的技能标签是否匹配当前技能
 * 2. 如果匹配，结束技能（EndAbility）
 * 
 * @param AbilityTag 要停用的技能标签
 * 
 * 使用场景：
 * - 被动技能被卸载时由 ASC 调用
 * 
 * 注意：
 * - EndAbility 的最后一个参数为 true，表示技能被取消
 */
void UAuraPassiveAbility::ReceiveDeactivate(const FGameplayTag& AbilityTag)
{
	if (AbilityTags.HasTagExact(AbilityTag))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}
