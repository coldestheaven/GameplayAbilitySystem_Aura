// Copyright Druid Mechanics


#include "UI/HUD/LoadScreenHUD.h"

#include "Blueprint/UserWidget.h"
#include "UI/ViewModel/MVVM_LoadScreen.h"
#include "UI/Widget/LoadScreenWidget.h"

/**
 * 游戏开始时初始化加载屏幕
 * 
 * 实现流程：
 * 1. 调用父类 BeginPlay
 * 2. 创建 LoadScreen ViewModel
 * 3. 初始化存档槽位列表
 * 4. 创建 LoadScreen Widget
 * 5. 将 Widget 添加到视口
 * 6. 调用蓝图初始化（BlueprintInitializeWidget）
 * 7. 加载存档数据（LoadData）
 * 
 * 使用场景：
 * - 加载屏幕关卡开始时自动调用
 * 
 * 注意：
 * - LoadScreen 使用 MVVM 模式，ViewModel 管理数据，Widget 显示 UI
 */
void ALoadScreenHUD::BeginPlay()
{
	Super::BeginPlay();

	// 创建并初始化 ViewModel
	LoadScreenViewModel = NewObject<UMVVM_LoadScreen>(this, LoadScreenViewModelClass);
	LoadScreenViewModel->InitializeLoadSlots();

	// 创建并显示 Widget
	LoadScreenWidget = CreateWidget<ULoadScreenWidget>(GetWorld(), LoadScreenWidgetClass);
	LoadScreenWidget->AddToViewport();
	LoadScreenWidget->BlueprintInitializeWidget();

	// 加载存档数据
	LoadScreenViewModel->LoadData();
}
