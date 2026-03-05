// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlueprintBase.h"
#include "BTService_FindNearestPlayer.generated.h"

/**
 * 行为树服务：查找最近的玩家
 *
 * 功能：
 * - 每帧（或按指定间隔）扫描场景中所有实现了 IPlayerInterface 的 Actor
 * - 找到距离 AI 最近的玩家，将其写入黑板的 TargetToFollowSelector 键
 * - 同时将距离写入黑板的 DistanceToTargetSelector 键
 *
 * 使用方式：
 *   在行为树编辑器中，将此服务挂载到需要追踪玩家的行为树节点上
 *   配置 TargetToFollowSelector 和 DistanceToTargetSelector 黑板键
 *
 * 黑板键说明：
 *   - TargetToFollowSelector：Object 类型，存储最近玩家的 Actor 引用
 *   - DistanceToTargetSelector：Float 类型，存储到最近玩家的距离（单位：cm）
 */
UCLASS()
class AURA_API UBTService_FindNearestPlayer : public UBTService_BlueprintBase
{
	GENERATED_BODY()
protected:
	/**
	 * 行为树服务 Tick 回调（每次服务更新时调用）
	 * 遍历所有玩家，找到最近的一个，更新黑板数据
	 * @param OwnerComp   拥有此服务的行为树组件
	 * @param NodeMemory  节点内存（此服务不使用）
	 * @param DeltaSeconds 上次更新到本次更新的时间间隔
	 */
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	/**
	 * 追踪目标黑板键选择器
	 * 在行为树编辑器中配置，指向存储目标 Actor 的黑板键（Object 类型）
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FBlackboardKeySelector TargetToFollowSelector;

	/**
	 * 到目标距离的黑板键选择器
	 * 在行为树编辑器中配置，指向存储距离值的黑板键（Float 类型）
	 * 行为树的 Decorator 可以使用此值判断是否在攻击范围内
	 */
	UPROPERTY(BlueprintReadOnly, EditAnywhere)
	FBlackboardKeySelector DistanceToTargetSelector;
};
