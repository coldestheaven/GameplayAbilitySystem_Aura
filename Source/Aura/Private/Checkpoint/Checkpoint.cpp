// Copyright Druid Mechanics


#include "Checkpoint/Checkpoint.h"

#include "Components/SphereComponent.h"
#include "Game/AuraGameModeBase.h"
#include "Interaction/PlayerInterface.h"
#include "Kismet/GameplayStatics.h"

/**
 * 构造函数：初始化检查点组件
 * 
 * 实现流程：
 * 1. 禁用 Tick
 * 2. 创建检查点网格体组件，设置为阻挡碰撞
 * 3. 设置自定义深度模板值（用于高亮显示）
 * 4. 创建球体碰撞组件（用于检测玩家重叠）：
 *    - 挂载到网格体
 *    - 仅查询模式
 *    - 默认忽略所有通道
 *    - 与 Pawn 重叠
 * 5. 创建移动目标组件（玩家点击检查点时的移动目标位置）
 * 
 * 使用场景：
 * - 检查点 Actor 在关卡中放置时构造
 * 
 * 注意：
 * - MoveToComponent 的位置是玩家点击检查点时的移动目标
 */
ACheckpoint::ACheckpoint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
	
	// 创建检查点网格体（阻挡碰撞）
	CheckpointMesh = CreateDefaultSubobject<UStaticMeshComponent>("CheckpointMesh");
	CheckpointMesh->SetupAttachment(GetRootComponent());
	CheckpointMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CheckpointMesh->SetCollisionResponseToAllChannels(ECR_Block);

	// 设置自定义深度模板值（用于高亮显示）
	CheckpointMesh->SetCustomDepthStencilValue(CustomDepthStencilOverride);
	CheckpointMesh->MarkRenderStateDirty();

	// 创建重叠检测球体（检测玩家进入）
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	Sphere->SetupAttachment(CheckpointMesh);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// 创建移动目标组件（玩家点击时的移动目标位置）
	MoveToComponent = CreateDefaultSubobject<USceneComponent>("MoveToComponent");
	MoveToComponent->SetupAttachment(GetRootComponent());
}

/**
 * 从存档加载时调用（ISaveInterface 实现）
 * 
 * 实现流程：
 * 1. 如果检查点已被到达（bReached），处理发光效果
 * 
 * 使用场景：
 * - 关卡加载时恢复检查点状态
 * - 由 GameMode::LoadWorldState 调用
 */
void ACheckpoint::LoadActor_Implementation()
{
	if (bReached)
	{
		HandleGlowEffects();
	}
}

/**
 * 球体重叠事件处理（玩家进入检查点）
 * 
 * 实现流程：
 * 1. 检查目标是否实现了 PlayerInterface
 * 2. 设置 bReached = true（标记检查点已到达）
 * 3. 保存关卡世界状态（所有 Actor 状态）
 * 4. 保存玩家进度（等级、XP、属性、技能等）
 * 5. 处理发光效果（激活检查点）
 * 
 * @param OverlappedComponent 重叠的组件（Sphere）
 * @param OtherActor 重叠的 Actor（玩家）
 * @param OtherComp 其他组件的碰撞组件
 * @param OtherBodyIndex 其他组件的 Body 索引
 * @param bFromSweep 是否来自扫描
 * @param SweepResult 扫描结果
 * 
 * 使用场景：
 * - 玩家进入检查点范围时自动调用
 * 
 * 注意：
 * - 保存操作在服务端执行
 * - PlayerStartTag 用于玩家重生时确定出生点
 */
void ACheckpoint::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor->Implements<UPlayerInterface>())
	{
		bReached = true;

		// 保存关卡世界状态（所有 Actor 状态）
		if (AAuraGameModeBase* AuraGM = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this)))
		{
			const UWorld* World = GetWorld();
			FString MapName = World->GetMapName();
			MapName.RemoveFromStart(World->StreamingLevelsPrefix);
			
			AuraGM->SaveWorldState(GetWorld(), MapName);
		}
		
		// 保存玩家进度（等级、XP、属性、技能等）
		IPlayerInterface::Execute_SaveProgress(OtherActor, PlayerStartTag);
		HandleGlowEffects();
	}
}

/**
 * 游戏开始时初始化
 * 
 * 实现流程：
 * 1. 调用父类 BeginPlay
 * 2. 如果 bBindOverlapCallback 为 true，绑定重叠事件
 * 
 * 使用场景：
 * - 检查点生成后自动调用
 */
void ACheckpoint::BeginPlay()
{
	Super::BeginPlay();

	if (bBindOverlapCallback)
	{
		Sphere->OnComponentBeginOverlap.AddDynamic(this, &ACheckpoint::OnSphereOverlap);
	}
}

/**
 * 设置移动目标位置（IHighlightInterface 实现）
 * 
 * 实现流程：
 * 1. 将 OutDestination 设置为 MoveToComponent 的位置
 * 
 * @param OutDestination 输出的目标位置（会被修改）
 * 
 * 使用场景：
 * - 玩家点击检查点时，角色会移动到 MoveToComponent 的位置
 */
void ACheckpoint::SetMoveToLocation_Implementation(FVector& OutDestination)
{
	OutDestination = MoveToComponent->GetComponentLocation();
}

/**
 * 高亮显示检查点（IHighlightInterface 实现）
 * 
 * 实现流程：
 * 1. 如果检查点未被到达，启用自定义深度渲染（显示高亮轮廓）
 * 
 * 使用场景：
 * - 鼠标悬停在未到达的检查点上时调用
 */
void ACheckpoint::HighlightActor_Implementation()
{
	if (!bReached)
	{
		CheckpointMesh->SetRenderCustomDepth(true);
	}
}

/**
 * 取消高亮显示检查点（IHighlightInterface 实现）
 * 
 * 实现流程：
 * 1. 禁用自定义深度渲染
 */
void ACheckpoint::UnHighlightActor_Implementation()
{
	CheckpointMesh->SetRenderCustomDepth(false);
}

/**
 * 处理检查点发光效果（到达后激活）
 * 
 * 实现流程：
 * 1. 禁用球体碰撞（避免重复触发）
 * 2. 创建动态材质实例
 * 3. 替换网格体材质
 * 4. 调用蓝图事件 CheckpointReached（驱动材质参数变化）
 * 
 * 使用场景：
 * - 检查点被到达时调用
 * - 在 OnSphereOverlap 和 LoadActor 中调用
 * 
 * 注意：
 * - CheckpointReached 是蓝图实现事件，在蓝图中驱动材质参数动画
 */
void ACheckpoint::HandleGlowEffects()
{
	Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	UMaterialInstanceDynamic* DynamicMaterialInstace = UMaterialInstanceDynamic::Create(CheckpointMesh->GetMaterial(0), this);
	CheckpointMesh->SetMaterial(0, DynamicMaterialInstace);
	CheckpointReached(DynamicMaterialInstace);
}
