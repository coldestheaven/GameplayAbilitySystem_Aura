// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MagicCircleController.generated.h"

class UMaterialInterface;

UINTERFACE(MinimalAPI, BlueprintType)
class UMagicCircleController : public UInterface
{
	GENERATED_BODY()
};

/**
 * 魔法圆圈控制接口
 *
 * 用途：解耦 Character 与具体 PlayerController 类型（AAuraPlayerController）
 *      让 Character 通过接口调用魔法圆圈显示/隐藏，不依赖具体 Controller 类型
 *
 * 实现者：通常由 PlayerController 实现（魔法圆圈是玩家专属 UI）
 *
 * 设计动机：
 *   - 原 Character::ShowMagicCircle_Implementation 直接 Cast<AAuraPlayerController>(GetController())
 *   - 通过接口可消除该向上 Cast，且支持未来扩展（如 AI 控制器调试用魔法圆圈）
 */
class AURACORE_API IMagicCircleController
{
	GENERATED_BODY()

public:
	/**
	 * 显示魔法圆圈（用于需要指定目标位置的技能）
	 * @param DecalMaterial 圆圈贴花材质（nullptr 使用默认材质）
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MagicCircle")
	void ShowMagicCircleUI(UMaterialInterface* DecalMaterial);

	/**
	 * 隐藏魔法圆圈
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MagicCircle")
	void HideMagicCircleUI();
};
