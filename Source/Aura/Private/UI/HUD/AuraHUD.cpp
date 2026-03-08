// Copyright Druid Mechanics


#include "UI/HUD/AuraHUD.h"

#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/AttributeMenuWidgetController.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "UI/WidgetController/SpellMenuWidgetController.h"

/**
 * 获取或创建 Overlay WidgetController（覆盖层 UI 控制器）
 * 
 * 实现流程：
 * 1. 如果控制器未创建，创建新的 OverlayWidgetController 对象
 * 2. 设置 WidgetController 参数（PC、PS、ASC、AS）
 * 3. 绑定回调到依赖项（属性变化、技能变化等）
 * 4. 返回控制器（单例模式，确保只有一个实例）
 * 
 * @param WCParams WidgetController 参数结构体
 * @return OverlayWidgetController 指针
 * 
 * 使用场景：
 * - 初始化覆盖层 UI 时调用
 * - 由 AuraAbilitySystemLibrary::GetOverlayWidgetController 调用
 * 
 * 注意：
 * - 使用单例模式，确保整个游戏只有一个 OverlayWidgetController
 * - BindCallbacksToDependencies 会绑定所有属性变化回调
 */
UOverlayWidgetController* AAuraHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	if (OverlayWidgetController == nullptr)
	{
		OverlayWidgetController = NewObject<UOverlayWidgetController>(this, OverlayWidgetControllerClass);
		OverlayWidgetController->SetWidgetControllerParams(WCParams);
		OverlayWidgetController->BindCallbacksToDependencies();
	}
	return OverlayWidgetController;
}

/**
 * 获取或创建 AttributeMenu WidgetController（属性菜单控制器）
 * 
 * 实现流程：
 * 1. 如果控制器未创建，创建新的 AttributeMenuWidgetController 对象
 * 2. 设置 WidgetController 参数
 * 3. 绑定回调到依赖项
 * 4. 返回控制器（单例模式）
 * 
 * @param WCParams WidgetController 参数结构体
 * @return AttributeMenuWidgetController 指针
 * 
 * 使用场景：
 * - 打开属性菜单时调用
 */
UAttributeMenuWidgetController* AAuraHUD::GetAttributeMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	if (AttributeMenuWidgetController == nullptr)
	{
		AttributeMenuWidgetController = NewObject<UAttributeMenuWidgetController>(this, AttributeMenuWidgetControllerClass);
		AttributeMenuWidgetController->SetWidgetControllerParams(WCParams);
		AttributeMenuWidgetController->BindCallbacksToDependencies();
	}
	return AttributeMenuWidgetController;
}

/**
 * 获取或创建 SpellMenu WidgetController（技能菜单控制器）
 * 
 * 实现流程：
 * 1. 如果控制器未创建，创建新的 SpellMenuWidgetController 对象
 * 2. 设置 WidgetController 参数
 * 3. 绑定回调到依赖项
 * 4. 返回控制器（单例模式）
 * 
 * @param WCParams WidgetController 参数结构体
 * @return SpellMenuWidgetController 指针
 * 
 * 使用场景：
 * - 打开技能菜单时调用
 */
USpellMenuWidgetController* AAuraHUD::GetSpellMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	if (SpellMenuWidgetController == nullptr)
	{
		SpellMenuWidgetController = NewObject<USpellMenuWidgetController>(this, SpellMenuWidgetControllerClass);
		SpellMenuWidgetController->SetWidgetControllerParams(WCParams);
		SpellMenuWidgetController->BindCallbacksToDependencies();
	}
	return SpellMenuWidgetController;
}

/**
 * 初始化覆盖层 UI（血条、法力条、经验条等）
 * 
 * 实现流程：
 * 1. 校验 OverlayWidgetClass 和 OverlayWidgetControllerClass 已配置
 * 2. 创建 Overlay Widget 对象
 * 3. 创建 WidgetControllerParams
 * 4. 获取或创建 OverlayWidgetController
 * 5. 将 WidgetController 设置到 Widget
 * 6. 广播初始值（确保 UI 显示正确的初始状态）
 * 7. 将 Widget 添加到视口
 * 
 * @param PC 玩家控制器
 * @param PS 玩家状态
 * @param ASC AbilitySystemComponent
 * @param AS AttributeSet
 * 
 * 使用场景：
 * - 玩家角色初始化完成后调用
 * - 在 AuraCharacter::InitAbilityActorInfo 中调用
 * 
 * 注意：
 * - BroadcastInitialValues 确保 UI 在绑定回调前显示正确的初始值
 * - Widget 会一直显示在视口中（覆盖层 UI）
 */
void AAuraHUD::InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
{
	checkf(OverlayWidgetClass, TEXT("Overlay Widget Class uninitialized, please fill out BP_AuraHUD"));
	checkf(OverlayWidgetControllerClass, TEXT("Overlay Widget Controller Class uninitialized, please fill out BP_AuraHUD"));
	
	UUserWidget* Widget = CreateWidget<UUserWidget>(GetWorld(), OverlayWidgetClass);
	OverlayWidget = Cast<UAuraUserWidget>(Widget);
	
	const FWidgetControllerParams WidgetControllerParams(PC, PS, ASC, AS);
	UOverlayWidgetController* WidgetController = GetOverlayWidgetController(WidgetControllerParams);

	OverlayWidget->SetWidgetController(WidgetController);
	WidgetController->BroadcastInitialValues();
	Widget->AddToViewport();
}

