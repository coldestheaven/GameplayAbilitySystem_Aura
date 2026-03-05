// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraGameplayAbility.h"
#include "AuraPassiveAbility.generated.h"

/**
 * 被动技能基类
 *
 * 所有被动技能（如保护光环、生命虹吸、法力虹吸）均继承自此类
 *
 * 与主动技能的区别：
 * - 被动技能在赋予后立即激活（InstancingPolicy = InstancedPerActor）
 * - 激活后持续生效，不会自动结束
 * - 通过 ASC 的 DeactivatePassiveAbility 委托接收停用通知
 * - 停用时调用 EndAbility 结束技能
 *
 * 被动技能激活流程：
 *   AddCharacterPassiveAbilities → TryActivateAbility → ActivateAbility
 *   → 绑定 DeactivatePassiveAbility 委托 → 持续生效
 *   → ReceiveDeactivate（收到停用通知）→ EndAbility
 *
 * 被动特效同步：
 *   激活时通过 ASC.MulticastActivatePassiveEffect(Tag, true) 通知所有客户端开启特效
 *   停用时通过 ASC.MulticastActivatePassiveEffect(Tag, false) 通知所有客户端关闭特效
 */
UCLASS()
class AURA_API UAuraPassiveAbility : public UAuraGameplayAbility
{
	GENERATED_BODY()
public:
	/**
	 * 技能激活（重写基类）
	 * 绑定 ASC 的 DeactivatePassiveAbility 委托，监听停用通知
	 * 通知所有客户端开启对应的被动特效（PassiveNiagaraComponent）
	 */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	/**
	 * 接收停用通知
	 * 当 ASC 广播 DeactivatePassiveAbility 委托时调用
	 * 如果 AbilityTag 匹配此技能，则调用 EndAbility 结束技能
	 * @param AbilityTag 要停用的技能标签
	 */
	void ReceiveDeactivate(const FGameplayTag& AbilityTag);
};
