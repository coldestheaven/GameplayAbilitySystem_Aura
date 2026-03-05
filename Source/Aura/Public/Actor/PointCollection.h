// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PointCollection.generated.h"

/**
 * 点集合 Actor
 *
 * 提供一组预定义的场景组件点位（Pt_0 ~ Pt_10），用于技能的多点生成
 * 主要用于奥术碎片技能（UArcaneShards）计算均匀分布的生成位置
 *
 * 功能：
 * - 持有 11 个场景组件点位（Pt_0 ~ Pt_10）
 * - GetGroundPoints 函数将这些点位投影到地面，返回实际的生成位置
 * - 支持 YawOverride 参数旋转整个点集合
 *
 * 使用方式（蓝图中）：
 *   // 在技能激活时
 *   APointCollection* PC = GetWorld()->SpawnActor<APointCollection>(PointCollectionClass);
 *   PC->SetActorLocation(TargetLocation);
 *   TArray<USceneComponent*> Points = PC->GetGroundPoints(TargetLocation, NumShards);
 *   for (USceneComponent* Point : Points)
 *   {
 *       // 在 Point->GetComponentLocation() 处生成奥术尖刺
 *   }
 */
UCLASS()
class AURA_API APointCollection : public AActor
{
	GENERATED_BODY()
	
public:	
	APointCollection();

	/**
	 * 获取投影到地面的点位数组（蓝图纯函数）
	 * 将 ImmutablePts 中的前 NumPoints 个点位通过向下射线检测投影到地面
	 * @param GroundLocation 参考地面位置（用于确定射线起点高度）
	 * @param NumPoints      需要的点位数量（不超过 ImmutablePts 的大小）
	 * @param YawOverride    旋转偏移角度（度），用于旋转整个点集合
	 * @return 投影到地面后的场景组件数组
	 */
	UFUNCTION(BlueprintPure)
	TArray<USceneComponent*> GetGroundPoints(const FVector& GroundLocation, int32 NumPoints, float YawOverride = 0.f);
	
protected:
	virtual void BeginPlay() override;

	/**
	 * 所有点位的不可变数组（按顺序存储 Pt_0 ~ Pt_10）
	 * 在构造函数中初始化，GetGroundPoints 从此数组中取前 NumPoints 个
	 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<USceneComponent*> ImmutablePts;

	/** 第 0 个点位（中心点） */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_0;

	/** 第 1 个点位 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_1;

	/** 第 2 个点位 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_2;

	/** 第 3 个点位 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_3;

	/** 第 4 个点位 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_4;

	/** 第 5 个点位 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_5;

	/** 第 6 个点位 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_6;

	/** 第 7 个点位 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_7;

	/** 第 8 个点位 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_8;

	/** 第 9 个点位 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_9;

	/** 第 10 个点位 */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TObjectPtr<USceneComponent> Pt_10;
};
