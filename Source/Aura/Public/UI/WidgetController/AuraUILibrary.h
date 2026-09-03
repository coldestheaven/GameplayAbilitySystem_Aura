// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "AuraUILibrary.generated.h"

class AAuraHUD;
class UOverlayWidgetController;
class UAttributeMenuWidgetController;
class USpellMenuWidgetController;

/**
 * Aura UI 工具库（蓝图函数库）
 *
 * 负责 WidgetController 的获取（UI 层关注点）：
 * - MakeWidgetControllerParams：从 WorldContext 收集 PC/PS/ASC/AS + HUD
 * - Get*WidgetController：获取三种菜单控制器
 *
 * 架构说明：这组函数原位于 UAuraAbilitySystemLibrary（GAS 库），
 * 现拆分至 UI 层专用库——GAS 库只留数据/机制职责，UI 访问归 UI 层。
 * 既有蓝图引用经 DefaultEngine.ini 的 FunctionRedirects 自动重定向。
 *
 * 使用示例（蓝图中）：
 *   UOverlayWidgetController* OWC = UAuraUILibrary::GetOverlayWidgetController(this);
 */
UCLASS()
class AURA_API UAuraUILibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/**
	 * 构建 WidgetControllerParams 并获取 AuraHUD
	 * @param WorldContextObject 世界上下文对象（通常传 self）
	 * @param OutWCParams        输出：填充好的 WidgetControllerParams
	 * @param OutAuraHUD         输出：AuraHUD 引用
	 * @return true 表示成功获取所有必要对象
	 */
	UFUNCTION(BlueprintPure, Category = "AuraUILibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static bool MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, AAuraHUD*& OutAuraHUD);

	/** 获取 OverlayWidgetController（主 HUD 控制器） */
	UFUNCTION(BlueprintPure, Category = "AuraUILibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	/** 获取 AttributeMenuWidgetController（属性菜单控制器） */
	UFUNCTION(BlueprintPure, Category = "AuraUILibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);

	/** 获取 SpellMenuWidgetController（技能菜单控制器） */
	UFUNCTION(BlueprintPure, Category = "AuraUILibrary|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static USpellMenuWidgetController* GetSpellMenuWidgetController(const UObject* WorldContextObject);
};
