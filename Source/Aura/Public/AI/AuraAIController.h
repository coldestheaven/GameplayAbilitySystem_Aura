// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AuraAIController.generated.h"

class UBlackboardComponent;
class UBehaviorTreeComponent;

/**
 * Aura 敌人 AI 控制器
 *
 * 职责：
 * - 持有并管理行为树组件（BehaviorTreeComponent）
 * - 持有黑板组件（BlackboardComponent，由基类 AAIController 提供）
 * - 在构造函数中创建 BehaviorTreeComponent 并设置为默认行为树组件
 *
 * 使用流程：
 *   1. 敌人被 AI 控制器接管（PossessedBy）
 *   2. 敌人调用 RunBehaviorTree(BehaviorTree) 启动行为树
 *   3. 行为树通过黑板（Blackboard）读写 AI 决策数据
 *      （如 TargetActor、DistanceToTarget 等）
 *
 * 注意：此控制器只在服务端存在，AI 逻辑不在客户端运行
 */
UCLASS()
class AURA_API AAuraAIController : public AAIController
{
	GENERATED_BODY()
public:
	AAuraAIController();

protected:
	/**
	 * 行为树组件
	 * 负责运行和管理敌人的行为树资产（BehaviorTree）
	 * 在构造函数中创建，通过 RunBehaviorTree 启动
	 */
	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;
};
