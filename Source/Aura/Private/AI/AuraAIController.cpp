// Copyright Druid Mechanics


#include "AI/AuraAIController.h"

#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

/**
 * 构造函数：初始化 AI 控制器组件
 * 
 * 实现流程：
 * 1. 创建 BlackboardComponent（黑板组件，存储 AI 状态数据）
 * 2. 创建 BehaviorTreeComponent（行为树组件，执行 AI 逻辑）
 * 
 * 使用场景：
 * - AI 控制器在游戏开始时由引擎自动构造
 * 
 * 注意：
 * - Blackboard 用于存储 AI 状态（如目标位置、是否受击等）
 * - BehaviorTree 定义了 AI 的行为逻辑（巡逻、攻击、追击等）
 */
AAuraAIController::AAuraAIController()
{
	Blackboard = CreateDefaultSubobject<UBlackboardComponent>("BlackboardComponent");
	check(Blackboard);
	BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>("BehaviorTreeComponent");
	check(BehaviorTreeComponent);
}
