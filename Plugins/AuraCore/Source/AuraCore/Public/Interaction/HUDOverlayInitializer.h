// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HUDOverlayInitializer.generated.h"

class APlayerController;
class APlayerState;
class UAbilitySystemComponent;
class UAttributeSet;

UINTERFACE(MinimalAPI, BlueprintType)
class UHUDOverlayInitializer : public UInterface
{
	GENERATED_BODY()
};

/**
 * HUD Overlay 初始化接口
 *
 * 用途：解耦 Character / PlayerState 与具体 HUD 类型（AAuraHUD）
 *      使 Character 不再 #include "UI/HUD/AuraHUD.h"
 *      不再 Cast<AAuraHUD>(PC->GetHUD()) 然后调用 InitOverlay(...)
 *
 * 实现者：
 *   - 通常由项目中的 HUD 类（AAuraHUD）实现
 *   - 替换 HUD 实现时，调用方零修改
 *
 * 调用约定：
 *   - 通常在客户端 ASC 初始化完成后由 Character 调用
 *   - 实现者负责创建并显示主 Overlay Widget、绑定 WidgetController
 */
class AURACORE_API IHUDOverlayInitializer
{
	GENERATED_BODY()

public:
	/**
	 * 初始化主 Overlay HUD
	 *
	 * @param PC  玩家控制器
	 * @param PS  玩家状态（提供 ASC、AttributeSet 持久化）
	 * @param ASC 能力系统组件
	 * @param AS  属性集
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "HUD")
	void InitOverlayHUD(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);
};
