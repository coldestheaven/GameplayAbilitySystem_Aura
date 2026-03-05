// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "NiagaraComponent.h"
#include "GameplayTagContainer.h"
#include "DebuffNiagaraComponent.generated.h"

/**
 * Debuff 特效 Niagara 组件
 *
 * 挂载在角色上，负责在角色处于特定 Debuff 状态时自动激活/停用对应的粒子特效
 *
 * 支持的 Debuff 类型（通过 DebuffTag 配置）：
 * - 燃烧（Debuff.Burn）：火焰粒子特效
 * - 眩晕（Debuff.Stun）：眩晕粒子特效
 *
 * 工作原理：
 * - BeginPlay 时从 Avatar Actor 获取 ASC
 * - 注册 DebuffTag 的 GameplayTag 变化回调
 * - Tag 数量 > 0 时激活粒子特效（Activate）
 * - Tag 数量 = 0 时停用粒子特效（Deactivate）
 *
 * 网络说明：
 * - 通过 ReplicatedUsing 属性（bIsBurned/bIsStunned）同步到客户端
 * - 客户端的 OnRep 回调会触发此组件的激活/停用
 */
UCLASS()
class AURA_API UDebuffNiagaraComponent : public UNiagaraComponent
{
	GENERATED_BODY()
public:
	UDebuffNiagaraComponent();

	/**
	 * 此组件对应的 Debuff GameplayTag
	 * 在 Details 面板中配置（如 Debuff.Burn 或 Debuff.Stun）
	 * 当 ASC 上此 Tag 的数量变化时，自动激活/停用粒子特效
	 */
	UPROPERTY(VisibleAnywhere)
	FGameplayTag DebuffTag;

protected:
	/** 初始化：从 Avatar Actor 获取 ASC 并注册 Tag 变化回调 */
	virtual void BeginPlay() override;

	/**
	 * Debuff Tag 数量变化回调
	 * 绑定到 ASC 的 RegisterGameplayTagEvent
	 * @param CallbackTag 变化的 Tag（应与 DebuffTag 匹配）
	 * @param NewCount    新的 Tag 数量（>0 激活特效，=0 停用特效）
	 */
	void DebuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
};
