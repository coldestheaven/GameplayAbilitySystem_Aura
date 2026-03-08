// Copyright Druid Mechanics


#include "AI/BTService_FindNearestPlayer.h"
#include "AIController.h"
#include "BehaviorTree/BTFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

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
 * @param OwnerComp 行为树组件
 * @param NodeMemory 节点内存
 * @param DeltaSeconds 帧时间间隔
 * 
 * 使用场景：
 * - AI 行为树每帧调用，更新目标信息
 * 
 * 注意：
 * - 此服务会持续更新最近目标，确保 AI 始终追踪最近的敌人/玩家
 * - 如果未找到目标，黑板值会被设置为 nullptr 和最大浮点数
 */
void UBTService_FindNearestPlayer::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	APawn* OwningPawn = AIOwner->GetPawn();

	// 确定目标标签（Player 找 Enemy，Enemy 找 Player）
	const FName TargetTag = OwningPawn->ActorHasTag(FName("Player")) ? FName("Enemy") : FName("Player");

	// 获取所有带有目标标签的 Actor
	TArray<AActor*> ActorsWithTag;
	UGameplayStatics::GetAllActorsWithTag(OwningPawn, TargetTag, ActorsWithTag);

	// 找到距离最近的 Actor
	float ClosestDistance = TNumericLimits<float>::Max();
	AActor* ClosestActor = nullptr;
	for (AActor* Actor : ActorsWithTag)
	{
		if (IsValid(Actor) && IsValid(OwningPawn))
		{
			const float Distance = OwningPawn->GetDistanceTo(Actor);
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestActor = Actor;
			}
		}
	}
	
	// 更新黑板值
	UBTFunctionLibrary::SetBlackboardValueAsObject(this,TargetToFollowSelector, ClosestActor);
	UBTFunctionLibrary::SetBlackboardValueAsFloat(this, DistanceToTargetSelector, ClosestDistance);
}
