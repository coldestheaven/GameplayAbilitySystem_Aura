// Copyright Druid Mechanics


#include "UI/HUD/AuraHUD.h"

#include "UI/Widget/AuraUserWidget.h"
#include "UI/WidgetController/AttributeMenuWidgetController.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "UI/WidgetController/SpellMenuWidgetController.h"

UOverlayWidgetController* AAuraHUD::GetOverlayWidgetController(const FWidgetControllerParams& WCParams)
{
	return GetOrCreateWidgetController(OverlayWidgetController, OverlayWidgetControllerClass, WCParams);
}

UAttributeMenuWidgetController* AAuraHUD::GetAttributeMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	return GetOrCreateWidgetController(AttributeMenuWidgetController, AttributeMenuWidgetControllerClass, WCParams);
}

USpellMenuWidgetController* AAuraHUD::GetSpellMenuWidgetController(const FWidgetControllerParams& WCParams)
{
	return GetOrCreateWidgetController(SpellMenuWidgetController, SpellMenuWidgetControllerClass, WCParams);
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

