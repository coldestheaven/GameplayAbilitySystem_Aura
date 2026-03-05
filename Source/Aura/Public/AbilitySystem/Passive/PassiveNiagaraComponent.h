// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "GameplayTagContainer.h"
#include "PassiveNiagaraComponent.generated.h"

class UAuraAbilitySystemComponent;

/**
 * 被动技能特效 Niagara 组件
 *
 * 挂载在角色上，负责在对应被动技能激活/停用时自动开关粒子特效
 *
 * 支持的被动技能（通过 PassiveSpellTag 配置）：
 * - 保护光环（Abilities.Passive.HaloOfProtection）：防护光环特效
 * - 生命虹吸（Abilities.Passive.LifeSiphon）：生命吸取特效
 * - 法力虹吸（Abilities.Passive.ManaSiphon）：法力吸取特效
 *
 * 工作原理：
 * - BeginPlay 时从 Avatar Actor 获取 ASC
 * - 绑定 ASC 的 ActivatePassiveEffect 委托
 * - 收到激活通知时开启粒子特效
 * - 收到停用通知时关闭粒子特效
 * - 如果技能已经装备（ASC 初始化完成前），在 ActivateIfEquipped 中补充激活
 *
 * 网络说明：
 * - 通过 ASC 的 MulticastActivatePassiveEffect RPC 同步到所有客户端
 * - 确保所有玩家都能看到被动技能特效
 */
UCLASS()
class AURA_API UPassiveNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()
public:
	UPassiveNiagaraComponent();

	/**
	 * 此组件对应的被动技能标签
	 * 在 Details 面板中配置（如 Abilities.Passive.HaloOfProtection）
	 * 当收到对应标签的激活/停用通知时，自动开关粒子特效
	 */
	UPROPERTY(EditDefaultsOnly)
	FGameplayTag PassiveSpellTag;

protected:
	/** 初始化：从 Avatar Actor 获取 ASC，绑定委托，检查是否已装备 */
	virtual void BeginPlay() override;

	/**
	 * 被动技能激活/停用回调
	 * 绑定到 ASC 的 ActivatePassiveEffect 委托
	 * @param AbilityTag 激活/停用的技能标签
	 * @param bActivate  true 表示激活，false 表示停用
	 */
	void OnPassiveActivate(const FGameplayTag& AbilityTag, bool bActivate);

	/**
	 * 如果技能已装备则立即激活特效
	 * 处理 ASC 初始化完成前技能已装备的情况
	 * 在 AbilitiesGivenDelegate 回调中调用
	 * @param AuraASC Aura 专用 ASC 引用
	 */
	void ActivateIfEquipped(UAuraAbilitySystemComponent* AuraASC);
};
