// Copyright Druid Mechanics


#include "Actor/AuraEnemySpawnVolume.h"

#include "Actor/AuraEnemySpawnPoint.h"
#include "Aura/Aura.h"
#include "Components/BoxComponent.h"
#include "Interaction/PlayerInterface.h"

/**
 * 构造函数：初始化敌人生成区域
 * 
 * 实现流程：
 * 1. 禁用 Tick
 * 2. 创建盒子碰撞组件，设置为根组件
 * 3. 配置碰撞：
 *    - 碰撞模式：QueryOnly（仅查询）
 *    - 碰撞对象类型：ECC_WorldStatic
 *    - 默认忽略所有通道
 *    - 与 Pawn 重叠
 * 
 * 使用场景：
 * - 敌人生成区域 Actor 在关卡中放置时构造
 */
AAuraEnemySpawnVolume::AAuraEnemySpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	Box = CreateDefaultSubobject<UBoxComponent>("Box");
	SetRootComponent(Box);
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Box->SetCollisionObjectType(ECC_WorldStatic);
	Box->SetCollisionResponseToAllChannels(ECR_Ignore);
	Box->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

/**
 * 从存档加载时调用（ISaveInterface 实现）
 * 
 * 实现流程：
 * 1. 如果区域已被触发（bReached），销毁自身
 * 
 * 使用场景：
 * - 关卡加载时调用
 * 
 * 注意：
 * - 已触发的生成区域会被销毁（避免重复生成）
 */
void AAuraEnemySpawnVolume::LoadActor_Implementation()
{
	if (bReached)
	{
		Destroy();
	}
}

/**
 * 游戏开始时初始化
 * 
 * 实现流程：
 * 1. 调用父类 BeginPlay
 * 2. 绑定盒子重叠事件（OnBoxOverlap）
 */
void AAuraEnemySpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	Box->OnComponentBeginOverlap.AddDynamic(this, &AAuraEnemySpawnVolume::OnBoxOverlap);
}

/**
 * 盒子重叠事件处理（玩家进入生成区域）
 * 
 * 实现流程：
 * 1. 检查目标是否实现了 PlayerInterface
 * 2. 设置 bReached = true（标记区域已触发）
 * 3. 遍历所有生成点（SpawnPoints）：
 *    - 如果点有效，调用 SpawnEnemy 生成敌人
 * 4. 禁用盒子碰撞（避免重复触发）
 * 
 * @param OverlappedComponent 重叠的组件（Box）
 * @param OtherActor 重叠的 Actor（玩家）
 * @param OtherComp 其他组件的碰撞组件
 * @param OtherBodyIndex 其他组件的 Body 索引
 * @param bFromSweep 是否来自扫描
 * @param SweepResult 扫描结果
 * 
 * 使用场景：
 * - 玩家进入敌人生成区域时自动调用
 * 
 * 注意：
 * - 生成区域只能触发一次（触发后禁用碰撞）
 * - 所有生成点会同时生成敌人
 */
void AAuraEnemySpawnVolume::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor->Implements<UPlayerInterface>())
	{
		return;
	}
	
	bReached = true;
	
	// 遍历所有生成点，生成敌人
	for (AAuraEnemySpawnPoint* Point : SpawnPoints)
	{
		if (IsValid(Point))
		{
			Point->SpawnEnemy();
		}
	}
	
	// 禁用碰撞（避免重复触发）
	Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


