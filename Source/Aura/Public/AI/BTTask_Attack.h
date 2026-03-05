// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "BTTask_Attack.generated.h"

/**
 * 行为树任务：执行攻击
 *
 * 敌人 AI 行为树中的攻击任务节点
 * 通过 ICombatInterface 触发角色的攻击技能
 *
 * 工作原理：
 * - 从行为树组件获取 AI 控制器和被控制的 Pawn
 * - 通过 ICombatInterface 调用 Pawn 的攻击逻辑
 * - 攻击完成后返回 EBTNodeResult::Succeeded
 *
 * 使用方式：
 *   在行为树编辑器中，将此任务节点添加到攻击序列中
 *   通常在 BTDecorator_IsInAttackRange 检测通过后执行
 *
 * 注意：此任务是同步的，攻击动画播放完毕后立即返回成功
 * 如果需要等待动画完成，应使用 AbilityTask 或 LatentTask
 */
UCLASS()
class AURA_API UBTTask_Attack : public UBTTask_BlueprintBase
{
	GENERATED_BODY()

	/**
	 * 执行攻击任务（重写基类）
	 * 获取 AI 控制的 Pawn，通过 ICombatInterface 触发攻击
	 * @param OwnerComp  拥有此任务的行为树组件
	 * @param NodeMemory 节点内存（此任务不使用）
	 * @return EBTNodeResult::Succeeded（攻击触发成功）
	 *         EBTNodeResult::Failed（无法获取 Pawn 或 Pawn 不实现 ICombatInterface）
	 */
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
