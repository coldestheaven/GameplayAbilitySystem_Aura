// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlueprintBase.h"
#include "BTService_FindNearestPlayer.generated.h"

/**
 * 行为树服务：查找最近的目标
 *
 * 功能：
 * - 每帧（或按指定间隔）根据 AI 自身标签确定目标标签：
 *   - 如果 AI 是 Player，目标标签为 "Enemy"
 *   - 否则目标标签为 "Player"
 * - 找到距离 AI 最近的目标 Actor，将其写入黑板的 TargetToFollowSelector 键
 * - 同时将距离写入黑板的 DistanceToTargetSelector 键
 *
 * 使用方式：
 *   在行为树编辑器中，将此服务挂载到需要追踪目标的行为树节点上
 *   配置 TargetToFollowSelector 和 DistanceToTargetSelector 黑板键
 *
 * 黑板键说明：
 *   - TargetToFollowSelector：Object 类型，存储最近目标的 Actor 引用
 *   - DistanceToTargetSelector：Float 类型，存储到最近目标的距离（单位：cm）
 *
 * 注意：
 * - 此服务会持续更新最近目标，确保 AI 始终追踪最近的敌人/玩家
 * - 如果未找到目标，黑板值会被设置为 nullptr 和最大浮点数
 */
UCLASS()
class AURA_API UBTService_FindNearestPlayer : public UBTService_BlueprintBase
{
	GENERATED_BODY()
protected:
	/**
	 * 每帧更新节点（BTService 的核心函数）
	 *
	 * 实现流程：
	 * 1. 调用父类 TickNode
	 * 2. 获取 AI 控制的 Pawn
	 * 3. 确定目标标签：
	 *    - 如果 Pawn 是 Player，目标标签为 "Enemy"
	 *    - 否则目标标签为 "Player"
	 * 4. 获取所有带有目标标签的 Actor
	 * 5. 遍历所有 Actor，找到距离最近的：
	 *    - 计算距离（GetDistanceTo）
	 *    - 更新最近距离和最近 Actor
	 * 6. 将最近 Actor 设置到黑板（TargetToFollowSelector）
	 * 7. 将最近距离设置到黑板（DistanceToTargetSelector）
	 *
	 * @param OwnerComp    行为树组件
	 * @param NodeMemory   节点内存
	 * @param DeltaSeconds 帧时间间隔
	 *
	 * 使用场景：
	 * - AI 行为树每帧调用，更新目标信息
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
