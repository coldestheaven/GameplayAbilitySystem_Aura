// Copyright Druid Mechanics


#include "AI/BTTask_Attack.h"

/**
 * 执行任务（重写基类）
 * 
 * 实现流程：
 * 1. 调用父类 ExecuteTask
 * 
 * @param OwnerComp 行为树组件
 * @param NodeMemory 节点内存
 * @return 任务执行结果
 * 
 * 使用场景：
 * - AI 行为树执行攻击任务时调用
 * 
 * 注意：
 * - 这是基类实现，实际攻击逻辑在蓝图中实现
 */
EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return Super::ExecuteTask(OwnerComp, NodeMemory);
}
