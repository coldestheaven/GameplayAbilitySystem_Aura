// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Aura/Aura.h"
#include "GameFramework/PlayerStart.h"
#include "Interaction/HighlightInterface.h"
#include "Interaction/SaveInterface.h"
#include "Checkpoint.generated.h"

class USphereComponent;

/**
 * 检查点 Actor
 *
 * 继承自 APlayerStart，同时实现了 ISaveInterface（存档接口）和 IHighlightInterface（高亮接口）
 *
 * 功能：
 * - 作为玩家的出生点（PlayerStart），记录玩家到达的位置
 * - 玩家触碰后标记为已到达（bReached=true），并保存游戏进度
 * - 支持高亮描边效果（鼠标悬停时显示）
 * - 到达后触发发光特效（蓝图事件 CheckpointReached）
 * - 提供移动目标位置（SetMoveToLocation），引导玩家移动到检查点前方
 *
 * 存档说明：
 * - bReached 标记了 SaveGame 说明符，会被序列化到存档
 * - 加载存档时，已到达的检查点会恢复发光状态（LoadActor_Implementation）
 *
 * 使用方式：
 *   在关卡中放置此 Actor，配置 PlayerStartTag 作为唯一标识
 *   玩家触碰后自动保存进度，死亡后从此检查点重生
 */
UCLASS()
class AURA_API ACheckpoint : public APlayerStart, public ISaveInterface, public IHighlightInterface
{
	GENERATED_BODY()
public:
	ACheckpoint(const FObjectInitializer& ObjectInitializer);

	/* ======================== Save Interface 实现 ======================== */

	/**
	 * 是否需要加载变换（重写基类）
	 * 检查点不需要恢复变换（位置固定在关卡中），返回 false
	 */
	virtual bool ShouldLoadTransform_Implementation() override { return false; };

	/**
	 * 加载 Actor 状态（重写基类）
	 * 从存档恢复 bReached 状态，如果已到达则触发发光特效
	 */
	virtual void LoadActor_Implementation() override;

	/* ======================== end Save Interface ======================== */

	/**
	 * 是否已到达此检查点
	 * SaveGame 说明符确保此值被序列化到存档
	 * 已到达的检查点在加载时会显示发光状态
	 */
	UPROPERTY(BlueprintReadWrite, SaveGame)
	bool bReached = false;

	/**
	 * 是否绑定重叠回调
	 * true（默认）：在 BeginPlay 中绑定 OnSphereOverlap 回调
	 * false：不绑定（用于地图入口等特殊检查点，由子类自行处理重叠）
	 */
	UPROPERTY(EditAnywhere)
	bool bBindOverlapCallback = true;

protected:
	/**
	 * 球形碰撞体重叠回调
	 * 玩家进入球形范围时调用，触发检查点到达逻辑
	 * @param OverlappedComponent 发生重叠的组件
	 * @param OtherActor          进入范围的 Actor
	 * @param OtherComp           进入范围的组件
	 * @param OtherBodyIndex      碰撞体索引
	 * @param bFromSweep          是否来自 Sweep 检测
	 * @param SweepResult         Sweep 检测结果
	 */
	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 初始化：根据 bBindOverlapCallback 决定是否绑定重叠回调 */
	virtual void BeginPlay() override;

	/* ======================== Highlight Interface 实现 ======================== */

	/**
	 * 设置移动目标位置（重写基类）
	 * 返回 MoveToComponent 的世界位置，引导玩家移动到检查点前方而非正中心
	 * @param OutDestination 输出：推荐的移动目标位置
	 */
	virtual void SetMoveToLocation_Implementation(FVector& OutDestination) override;

	/** 开启高亮描边（鼠标悬停时调用） */
	virtual void HighlightActor_Implementation() override;

	/** 关闭高亮描边 */
	virtual void UnHighlightActor_Implementation() override;

	/* ======================== end Highlight Interface ======================== */

	/**
	 * 移动目标场景组件
	 * 放置在检查点前方，作为玩家点击检查点时的移动目标位置
	 * 避免玩家移动到检查点正中心（可能被遮挡）
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> MoveToComponent;
	
	/**
	 * 自定义深度模板值（用于高亮描边效果）
	 * 默认使用 CUSTOM_DEPTH_TAN（棕褐色描边）
	 * 可在 Details 面板中修改以使用不同颜色的描边
	 */
	UPROPERTY(EditDefaultsOnly)
	int32 CustomDepthStencilOverride = CUSTOM_DEPTH_TAN;

	/**
	 * 蓝图事件：检查点到达时调用
	 * 在蓝图中实现发光特效的启动逻辑（修改材质参数）
	 * @param DynamicMaterialInstance 检查点网格体的动态材质实例
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void CheckpointReached(UMaterialInstanceDynamic* DynamicMaterialInstance);

	/**
	 * 处理发光特效（蓝图可调用）
	 * 创建动态材质实例并调用 CheckpointReached 蓝图事件
	 */
	UFUNCTION(BlueprintCallable)
	void HandleGlowEffects();

	/**
	 * 检查点静态网格体
	 * 显示检查点的视觉外观（如水晶、石碑等）
	 * 支持高亮描边（通过自定义深度）
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UStaticMeshComponent> CheckpointMesh;
	
	/**
	 * 球形碰撞体
	 * 检测玩家是否进入检查点范围，触发到达逻辑
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> Sphere;
};
