// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Interaction/HUDOverlayInitializer.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "AuraHUD.generated.h"

class UAttributeMenuWidgetController;
class UAttributeSet;
class UAbilitySystemComponent;
class UOverlayWidgetController;
class UAuraUserWidget;
class USpellMenuWidgetController;

/**
 * Aura 游戏 HUD 类
 *
 * 职责：
 * - 管理主 Overlay Widget（游戏内 HUD，显示生命值、法力值、技能栏等）
 * - 通过统一注册表管理所有 WidgetController（Overlay、AttributeMenu、SpellMenu 等）
 * - 提供懒加载的 WidgetController 获取接口（首次调用时创建并初始化）
 * - 在玩家初始化完成后（InitOverlay）创建并显示主 HUD Widget
 *
 * 使用流程：
 *   1. 玩家角色初始化 ASC 后调用 InitOverlay
 *   2. InitOverlay 创建 OverlayWidget 并设置其 WidgetController
 *   3. 各 WidgetController 通过 GetXxxWidgetController 懒加载获取
 *
 * 架构（C1 重构 · 2026-06-14）：
 * - WidgetController 实例统一存放在 ControllerInstances 注册表（TMap<UClass*, UAuraWidgetController*>）
 * - 公开的 GetXxxWidgetController 接口保留签名兼容，内部转发到通用模板 GetWidgetController<T>
 * - 添加新菜单类型：只需新增 ControllerClass 字段 + 一个 Get 接口（不再需要并列实例字段）
 *
 * 注意：此类只在本地玩家的客户端上存在（HUD 不参与网络同步）
 */
UCLASS()
class AURA_API AAuraHUD : public AHUD, public IHUDOverlayInitializer
{
	GENERATED_BODY()
public:

	//~ Begin IHUDOverlayInitializer
	/**
	 * 接口实现：转发到已有的 InitOverlay
	 * 使 Character 能通过接口调用，避免依赖 AAuraHUD 具体类型
	 */
	virtual void InitOverlayHUD_Implementation(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS) override;
	//~ End IHUDOverlayInitializer

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
	 * 通用 WidgetController 懒加载模板（C1 重构 · 2026-06-14）
	 *
	 * 实现：以「控制器类」为 Key 在注册表中查找；找不到则创建、初始化、绑定回调，并写入注册表
	 *
	 * 替代原 GetOrCreateWidgetController(TObjectPtr<T>&, TSubclassOf<T>, ...) 模板：
	 * - 旧版要求每个控制器类型都有一个独立的 TObjectPtr 字段，新增菜单 = 新增字段 + 新增 Get 接口
	 * - 新版统一存放在 TMap，新增菜单 = 新增 ControllerClass 字段 + 新增 Get 接口（不再增字段）
	 *
	 * @tparam T              要返回的具体 WidgetController 类型（必须继承 UAuraWidgetController）
	 * @param  ControllerClass 控制器类引用（蓝图 Detail 面板中配置的 TSubclassOf<T>）
	 * @param  WCParams        WidgetController 初始化参数
	 * @return 已初始化的 WidgetController 实例（创建失败返回 nullptr）
	 *
	 * 注意：
	 * - 调用前必须确保 ControllerClass 在蓝图中已配置
	 * - 注册表 Key 用「控制器类对象」而非具体派生类型，避免 TMap<TSubclassOf<T>, ...> 的类型擦除问题
	 */
	template<typename T>
	T* GetWidgetController(TSubclassOf<T> ControllerClass, const FWidgetControllerParams& WCParams)
	{
		if (ControllerClass == nullptr) return nullptr;

		if (TObjectPtr<UAuraWidgetController>* Found = ControllerInstances.Find(ControllerClass))
		{
			return Cast<T>(Found->Get());
		}

		T* NewController = NewObject<T>(this, ControllerClass);
		NewController->SetWidgetControllerParams(WCParams);
		NewController->BindCallbacksToDependencies();
		ControllerInstances.Add(ControllerClass, NewController);
		return NewController;
	}

	/**
	 * WidgetController 实例注册表（C1 重构 · 2026-06-14）
	 *
	 * Key  : 具体控制器类对象（与 ControllerClass 字段一致）
	 * Value: 该类的唯一实例（懒加载创建后缓存）
	 *
	 * 使用 UClass* 作为 Key 而非 TSubclassOf<T>：避免模板类型擦除带来的 TMap 复杂度
	 * UPROPERTY 让所有 Value 受 GC 管理，无需手动释放
	 */
	UPROPERTY()
	TMap<TObjectPtr<UClass>, TObjectPtr<UAuraWidgetController>> ControllerInstances;

	/** 主 Overlay Widget 实例（游戏内 HUD，显示生命值、法力值等） */
	UPROPERTY()
	TObjectPtr<UAuraUserWidget> OverlayWidget;

	/**
	 * 主 Overlay Widget 类（在 Details 面板中指定）
	 * 必须是 UAuraUserWidget 的子类
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAuraUserWidget> OverlayWidgetClass;

	/**
	 * OverlayWidgetController 类（在 Details 面板中指定）
	 * 必须是 UOverlayWidgetController 的子类
	 * （实例统一存放在 ControllerInstances，蓝图配置零影响）
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;

	/**
	 * AttributeMenuWidgetController 类（在 Details 面板中指定）
	 * 必须是 UAttributeMenuWidgetController 的子类
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAttributeMenuWidgetController> AttributeMenuWidgetControllerClass;

	/**
	 * SpellMenuWidgetController 类（在 Details 面板中指定）
	 * 必须是 USpellMenuWidgetController 的子类
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<USpellMenuWidgetController> SpellMenuWidgetControllerClass;
};
