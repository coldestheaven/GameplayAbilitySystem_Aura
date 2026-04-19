// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "TargetDataUnderMouse.generated.h"

/** 鼠标目标数据就绪时广播（携带目标数据句柄） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMouseTargetDataSignature, const FGameplayAbilityTargetDataHandle&, DataHandle);

/**
 * 技能任务：获取鼠标光标下的目标数据
 *
 * 用于需要指定目标位置的技能（如火焰箭、奥术碎片等）
 * 在服务端和客户端之间同步鼠标光标的命中位置
 *
 * 工作原理：
 * - 本地客户端：通过 ECC_Target 碰撞通道射线检测获取鼠标光标命中位置，
 *              打包成 FGameplayAbilityTargetData_SingleTargetHit 发送给服务端，
 *              同时立即广播 ValidData 委托（客户端预测，提供即时反馈）
 * - 服务端：绑定目标数据复制回调，等待客户端发送的 TargetData；
 *          收到后消费数据并广播 ValidData 委托
 * - 若数据在绑定前已到达，则立即调用回调；否则进入等待远程玩家数据状态
 *
 * 使用示例（蓝图中）：
 *   // 在技能激活时创建此任务
 *   UTargetDataUnderMouse* Task = UTargetDataUnderMouse::CreateTargetDataUnderMouse(this);
 *   Task->ValidData.AddDynamic(this, &UMyAbility::OnTargetDataReady);
 *   Task->ReadyForActivation();
 */
UCLASS()
class AURA_API UTargetDataUnderMouse : public UAbilityTask
{
	GENERATED_BODY()
public:

	/**
	 * 创建鼠标目标数据任务（蓝图可调用，内部使用）
	 * 工厂函数，创建并返回任务实例
	 * @param OwningAbility 拥有此任务的技能实例
	 * @return 创建的任务实例
	 */
	UFUNCTION(BlueprintCallable, Category="Ability|Tasks", meta = (DisplayName = "TargetDataUnderMouse", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true"))
	static UTargetDataUnderMouse* CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility);

	/**
	 * 目标数据就绪委托（蓝图可绑定）
	 * 当鼠标目标数据可用时广播（本地预测立即触发，服务端等待客户端数据）
	 * 技能绑定此委托以获取目标位置并执行技能逻辑
	 */
	UPROPERTY(BlueprintAssignable)
	FMouseTargetDataSignature ValidData;

private:
	/**
	 * 任务激活（重写基类）
	 * - 本地控制器：调用 SendMouseCursorData 发送目标数据
	 * - 非本地控制器（服务端）：绑定目标数据复制回调；
	 *   若数据已到达则立即处理，否则调用 SetWaitingOnRemotePlayerData 进入等待状态
	 */
	virtual void Activate() override;

	/**
	 * 发送鼠标光标目标数据（仅本地控制器调用）
	 * 在预测窗口（FScopedPredictionWindow）内，通过 ECC_Target 碰撞通道射线检测
	 * 获取鼠标光标命中位置，打包成 FGameplayAbilityTargetData_SingleTargetHit，
	 * 通过 ASC 的 ServerSetReplicatedTargetData 发送给服务端，
	 * 并立即广播 ValidData 委托实现客户端预测
	 */
	void SendMouseCursorData();

	/**
	 * 目标数据复制回调（服务端调用）
	 * 服务端收到客户端复制的目标数据后触发，
	 * 消费该数据（ConsumeClientReplicatedTargetData 确保只使用一次），
	 * 并广播 ValidData 委托以触发技能的目标处理逻辑
	 * @param DataHandle    收到的目标数据句柄
	 * @param ActivationTag 激活标签（用于标识此次数据传输）
	 */
	void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);
};
