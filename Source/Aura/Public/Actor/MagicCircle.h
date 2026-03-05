// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MagicCircle.generated.h"

/**
 * 魔法圆圈 Actor
 *
 * 在地面显示的技能目标指示器，用于需要指定目标位置的技能
 * 跟随鼠标光标在地面的投影位置移动
 *
 * 功能：
 * - 使用 DecalComponent 在地面投影圆圈贴花
 * - 支持自定义贴花材质（不同技能可以有不同的圆圈外观）
 * - 由 AAuraPlayerController 管理生命周期（ShowMagicCircle/HideMagicCircle）
 * - 每帧跟随鼠标光标位置更新（在 PlayerController 的 UpdateMagicCircleLocation 中）
 *
 * 使用方式：
 *   // 在 PlayerController 中
 *   MagicCircle = GetWorld()->SpawnActor<AMagicCircle>(MagicCircleClass);
 *   MagicCircle->MagicCircleDecal->SetMaterial(0, DecalMaterial);
 *   // 每帧更新位置
 *   MagicCircle->SetActorLocation(CursorHitLocation);
 */
UCLASS()
class AURA_API AMagicCircle : public AActor
{
	GENERATED_BODY()
	
public:	
	AMagicCircle();
	virtual void Tick(float DeltaTime) override;

	/**
	 * 魔法圆圈贴花组件
	 * 在地面投影圆圈图案，支持自定义材质
	 * 蓝图可读写，允许在运行时修改贴花材质
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UDecalComponent> MagicCircleDecal;
	
protected:
	virtual void BeginPlay() override;
};
