// Copyright Druid Mechanics


#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Interaction/CombatInterface.h"


/**
 * 构造函数：初始化 Debuff Niagara 组件
 * 
 * 实现流程：
 * 1. 禁用自动激活（等待 Debuff 标签变化时激活）
 * 
 * 使用场景：
 * - 组件创建时自动调用
 */
UDebuffNiagaraComponent::UDebuffNiagaraComponent()
{
	bAutoActivate = false;
}

/**
 * 游戏开始时初始化
 * 
 * 实现流程：
 * 1. 调用父类 BeginPlay
 * 2. 尝试获取 ASC：
 *    - 如果 ASC 已存在，直接注册 Debuff 标签变化回调
 *    - 如果 ASC 未存在但 Owner 实现了 CombatInterface，等待 ASC 注册后再注册回调
 * 
 * 使用场景：
 * - 组件初始化时调用
 * 
 * 注意：
 * - 如果 ASC 还未注册（如角色初始化阶段），会等待 ASC 注册完成
 * - 使用 WeakLambda 避免循环引用
 */
void UDebuffNiagaraComponent::BeginPlay()
{
	Super::BeginPlay();

	ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner());
	
	// 如果 ASC 已存在，直接注册回调
	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
	{
		ASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UDebuffNiagaraComponent::DebuffTagChanged);
	}
	// 如果 ASC 未存在，等待 ASC 注册
	else if (CombatInterface)
	{
		CombatInterface->GetOnASCRegisteredDelegate().AddWeakLambda(this, [this](UAbilitySystemComponent* InASC)
		{
			InASC->RegisterGameplayTagEvent(DebuffTag, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &UDebuffNiagaraComponent::DebuffTagChanged);
		});
	}
}

/**
 * Debuff 标签变化回调
 * 
 * 实现流程：
 * 1. 检查 Owner 是否有效
 * 2. 检查 Owner 是否存活（通过 CombatInterface）
 * 3. 如果标签数量 > 0 且 Owner 有效且存活，激活 Niagara 特效
 * 4. 否则停用 Niagara 特效
 * 
 * @param CallbackTag 触发回调的标签（DebuffTag）
 * @param NewCount 当前标签数量
 * 
 * 使用场景：
 * - Debuff GE 应用/移除时自动调用
 * 
 * 注意：
 * - 只有当角色存活时才会显示 Debuff 特效
 * - 标签数量 > 0 表示 Debuff 已应用
 */
void UDebuffNiagaraComponent::DebuffTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	const bool bOwnerValid = IsValid(GetOwner());
	const bool bOwnerAlive = GetOwner()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(GetOwner());
	
	if (NewCount > 0 && bOwnerValid && bOwnerAlive)
	{
		Activate();
	}
	else
	{
		Deactivate();
	}
}
