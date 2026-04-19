// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AuraHUD.generated.h"

class UAttributeMenuWidgetController;
class UAttributeSet;
class UAbilitySystemComponent;
class UOverlayWidgetController;
class UAuraUserWidget;
struct FWidgetControllerParams;
class USpellMenuWidgetController;

/**
 * Aura 游戏 HUD 类
 *
 * 职责：
 * - 管理主 Overlay Widget（游戏内 HUD，显示生命值、法力值、技能栏等）
 * - 管理三个 WidgetController（Overlay、AttributeMenu、SpellMenu）
 * - 提供懒加载的 WidgetController 获取接口（首次调用时创建并初始化）
 * - 在玩家初始化完成后（InitOverlay）创建并显示主 HUD Widget
 *
 * 使用流程：
 *   1. 玩家角色初始化 ASC 后调用 InitOverlay
 *   2. InitOverlay 创建 OverlayWidget 并设置其 WidgetController
 *   3. 各 WidgetController 通过 GetXxxWidgetController 懒加载获取
 *
 * 注意：此类只在本地玩家的客户端上存在（HUD 不参与网络同步）
 */
UCLASS()
class AURA_API AAuraHUD : public AHUD
{
	GENERATED_BODY()
public:

	/**
	 * 获取 OverlayWidgetController（懒加载）
	 * 首次调用时创建实例并绑定回调，后续调用直接返回缓存实例
	 * @param WCParams 包含 PC、PS、ASC、AS 的参数结构体
	 * @return OverlayWidgetController 实例
	 */
	UOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WCParams);

	/**
	 * 获取 AttributeMenuWidgetController（懒加载）
	 * @param WCParams 包含 PC、PS、ASC、AS 的参数结构体
	 * @return AttributeMenuWidgetController 实例
	 */
	UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const FWidgetControllerParams& WCParams);

	/**
	 * 获取 SpellMenuWidgetController（懒加载）
	 * @param WCParams 包含 PC、PS、ASC、AS 的参数结构体
	 * @return SpellMenuWidgetController 实例
	 */
	USpellMenuWidgetController* GetSpellMenuWidgetController(const FWidgetControllerParams& WCParams);

	/**
	 * 初始化主 Overlay HUD
	 * 创建 OverlayWidget，设置其 WidgetController，并添加到视口
	 * 在玩家角色的 InitAbilityActorInfo 完成后调用
	 * @param PC  玩家控制器
	 * @param PS  玩家状态
	 * @param ASC 能力系统组件
	 * @param AS  属性集
	 */
	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

protected:

private:
	/**
	 * 懒加载 WidgetController 的通用模板辅助函数
	 * 若 Controller 为空则创建、初始化并绑定回调，否则直接返回缓存实例
	 * @param Controller      缓存的控制器引用（TObjectPtr，首次调用后被填充）
	 * @param ControllerClass 要创建的控制器类
	 * @param WCParams        WidgetController 初始化参数
	 * @return 已初始化的 WidgetController 实例
	 */
	template<typename T>
	T* GetOrCreateWidgetController(TObjectPtr<T>& Controller, TSubclassOf<T> ControllerClass, const FWidgetControllerParams& WCParams)
	{
		if (Controller == nullptr)
		{
			Controller = NewObject<T>(this, ControllerClass);
			Controller->SetWidgetControllerParams(WCParams);
			Controller->BindCallbacksToDependencies();
		}
		return Controller;
	}

	/** 主 Overlay Widget 实例（游戏内 HUD，显示生命值、法力值等） */
	UPROPERTY()
	TObjectPtr<UAuraUserWidget> OverlayWidget;

	/**
	 * 主 Overlay Widget 类（在 Details 面板中指定）
	 * 必须是 UAuraUserWidget 的子类
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAuraUserWidget> OverlayWidgetClass;

	/** OverlayWidgetController 实例（懒加载缓存） */
	UPROPERTY()
	TObjectPtr<UOverlayWidgetController> OverlayWidgetController;

	/**
	 * OverlayWidgetController 类（在 Details 面板中指定）
	 * 必须是 UOverlayWidgetController 的子类
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;

	/** AttributeMenuWidgetController 实例（懒加载缓存） */
	UPROPERTY()
	TObjectPtr<UAttributeMenuWidgetController> AttributeMenuWidgetController;

	/**
	 * AttributeMenuWidgetController 类（在 Details 面板中指定）
	 * 必须是 UAttributeMenuWidgetController 的子类
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAttributeMenuWidgetController> AttributeMenuWidgetControllerClass;

	/** SpellMenuWidgetController 实例（懒加载缓存） */
	UPROPERTY()
	TObjectPtr<USpellMenuWidgetController> SpellMenuWidgetController;

	/**
	 * SpellMenuWidgetController 类（在 Details 面板中指定）
	 * 必须是 USpellMenuWidgetController 的子类
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<USpellMenuWidgetController> SpellMenuWidgetControllerClass;
};
