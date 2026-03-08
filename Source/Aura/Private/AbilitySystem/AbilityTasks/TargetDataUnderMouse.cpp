// Copyright Druid Mechanics


#include "AbilitySystem/AbilityTasks/TargetDataUnderMouse.h"
#include "AbilitySystemComponent.h"
#include "Aura/Aura.h"

/**
 * 创建目标数据任务（静态工厂函数）
 * 
 * @param OwningAbility 拥有此任务的技能
 * @return 创建的 TargetDataUnderMouse 任务对象
 * 
 * 使用场景：
 * - 技能需要获取鼠标光标下的目标时调用
 */
UTargetDataUnderMouse* UTargetDataUnderMouse::CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility)
{
	UTargetDataUnderMouse* MyObj = NewAbilityTask<UTargetDataUnderMouse>(OwningAbility);
	return MyObj;
}

/**
 * 激活任务
 * 
 * 实现流程：
 * 1. 检查是否为本地控制
 * 2. 如果是本地控制：
 *    - 直接发送鼠标光标数据（SendMouseCursorData）
 * 3. 如果不是本地控制（服务器/其他客户端）：
 *    - 绑定目标数据复制回调
 *    - 尝试调用已设置的回调（如果数据已到达）
 *    - 如果数据未到达，设置等待远程玩家数据状态
 * 
 * 使用场景：
 * - 技能激活时自动调用
 * 
 * 网络同步说明：
 * - 本地控制：立即获取鼠标数据并发送到服务器
 * - 非本地控制：等待服务器复制目标数据
 */
void UTargetDataUnderMouse::Activate()
{
	const bool bIsLocallyControlled = Ability->GetCurrentActorInfo()->IsLocallyControlled();
	if (bIsLocallyControlled)
	{
		SendMouseCursorData();
	}
	else
	{
		const FGameplayAbilitySpecHandle SpecHandle = GetAbilitySpecHandle();
		const FPredictionKey ActivationPredictionKey = GetActivationPredictionKey();
		AbilitySystemComponent.Get()->AbilityTargetDataSetDelegate(SpecHandle, ActivationPredictionKey).AddUObject(this, &UTargetDataUnderMouse::OnTargetDataReplicatedCallback);
		const bool bCalledDelegate = AbilitySystemComponent.Get()->CallReplicatedTargetDataDelegatesIfSet(SpecHandle, ActivationPredictionKey);
		if (!bCalledDelegate)
		{
			SetWaitingOnRemotePlayerData();
		}
	}
}

/**
 * 发送鼠标光标数据（仅本地控制调用）
 * 
 * 实现流程：
 * 1. 创建预测窗口（用于客户端预测）
 * 2. 获取玩家控制器
 * 3. 使用 GetHitResultUnderCursor 获取光标下的命中结果（ECC_Target 通道）
 * 4. 创建目标数据（SingleTargetHit）并设置命中结果
 * 5. 发送到服务器（ServerSetReplicatedTargetData）
 * 6. 如果应该广播委托，立即广播（客户端预测）
 * 
 * 使用场景：
 * - 本地控制的角色激活技能时调用
 * 
 * 网络同步说明：
 * - 使用客户端预测，立即广播委托（提供即时反馈）
 * - 同时发送到服务器进行权威验证
 * 
 * 注意：
 * - ECC_Target 通道用于光标追踪（在 Aura.h 中定义）
 */
void UTargetDataUnderMouse::SendMouseCursorData()
{
	FScopedPredictionWindow ScopedPrediction(AbilitySystemComponent.Get());
	
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult CursorHit;
	PC->GetHitResultUnderCursor(ECC_Target, false, CursorHit);

	FGameplayAbilityTargetDataHandle DataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = CursorHit;
	DataHandle.Add(Data);
	
	// 发送到服务器
	AbilitySystemComponent->ServerSetReplicatedTargetData(
		GetAbilitySpecHandle(),
		GetActivationPredictionKey(),
		DataHandle,
		FGameplayTag(),
		AbilitySystemComponent->ScopedPredictionKey);

	// 客户端预测：立即广播委托
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}

/**
 * 目标数据复制回调（非本地控制时调用）
 * 
 * 实现流程：
 * 1. 消费客户端复制的目标数据（ConsumeClientReplicatedTargetData）
 * 2. 如果应该广播委托，广播目标数据
 * 
 * @param DataHandle 复制的目标数据句柄
 * @param ActivationTag 激活标签
 * 
 * 使用场景：
 * - 服务器复制目标数据到客户端时调用
 * 
 * 注意：
 * - ConsumeClientReplicatedTargetData 确保数据只被使用一次
 */
void UTargetDataUnderMouse::OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag)
{
	AbilitySystemComponent->ConsumeClientReplicatedTargetData(GetAbilitySpecHandle(), GetActivationPredictionKey());
	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ValidData.Broadcast(DataHandle);
	}
}
