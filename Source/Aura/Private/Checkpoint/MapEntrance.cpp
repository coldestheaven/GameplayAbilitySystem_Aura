// Copyright Druid Mechanics


#include "Checkpoint/MapEntrance.h"

#include "Components/SphereComponent.h"
#include "Game/AuraGameModeBase.h"
#include "Interaction/PlayerInterface.h"
#include "Kismet/GameplayStatics.h"

/**
 * 构造函数：初始化地图入口
 * 
 * 实现流程：
 * 1. 将球体碰撞组件挂载到 MoveToComponent
 * 
 * 使用场景：
 * - 地图入口 Actor 在关卡中放置时构造
 * 
 * 注意：
 * - MapEntrance 继承自 Checkpoint，用于地图传送
 */
AMapEntrance::AMapEntrance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Sphere->SetupAttachment(MoveToComponent);
}

/**
 * 高亮显示地图入口（IHighlightInterface 实现）
 * 
 * 实现流程：
 * 1. 启用网格体的自定义深度渲染（显示高亮轮廓）
 */
void AMapEntrance::HighlightActor_Implementation()
{
	CheckpointMesh->SetRenderCustomDepth(true);
}

/**
 * 从存档加载时调用（ISaveInterface 实现）
 * 
 * 实现流程：
 * 1. 不执行任何操作（地图入口不需要加载状态）
 * 
 * 使用场景：
 * - 关卡加载时调用（但此函数为空实现）
 */
void AMapEntrance::LoadActor_Implementation()
{
	// 地图入口不需要加载状态
}

/**
 * 球体重叠事件处理（玩家进入地图入口）
 * 
 * 实现流程：
 * 1. 检查目标是否实现了 PlayerInterface
 * 2. 设置 bReached = true
 * 3. 保存目标地图的世界状态
 * 4. 保存玩家进度（使用目标地图的 PlayerStartTag）
 * 5. 打开目标地图（OpenLevelBySoftObjectPtr）
 * 
 * @param OverlappedComponent 重叠的组件（Sphere）
 * @param OtherActor 重叠的 Actor（玩家）
 * @param OtherComp 其他组件的碰撞组件
 * @param OtherBodyIndex 其他组件的 Body 索引
 * @param bFromSweep 是否来自扫描
 * @param SweepResult 扫描结果
 * 
 * 使用场景：
 * - 玩家进入地图入口范围时自动调用
 * 
 * 注意：
 * - 传送前会保存当前地图状态和目标地图状态
 * - DestinationPlayerStartTag 用于玩家在目标地图的出生点
 */
void AMapEntrance::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                   UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->Implements<UPlayerInterface>())
	{
		bReached = true;

		// 保存目标地图的世界状态
		if (AAuraGameModeBase* AuraGM = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this)))
		{
			AuraGM->SaveWorldState(GetWorld(), DestinationMap.ToSoftObjectPath().GetAssetName());
		}
		
		// 保存玩家进度（使用目标地图的 PlayerStartTag）
		IPlayerInterface::Execute_SaveProgress(OtherActor, DestinationPlayerStartTag);

		// 打开目标地图
		UGameplayStatics::OpenLevelBySoftObjectPtr(this, DestinationMap);
	}
}
