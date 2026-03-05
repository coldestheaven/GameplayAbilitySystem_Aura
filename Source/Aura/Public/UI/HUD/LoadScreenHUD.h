// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LoadScreenHUD.generated.h"

class UMVVM_LoadScreen;
class ULoadScreenWidget;

/**
 * 加载界面 HUD
 *
 * 管理游戏加载/存档选择界面的 HUD
 * 在游戏启动时（主菜单）和关卡切换时显示
 *
 * 职责：
 * - 创建并显示 LoadScreenWidget（加载界面 Widget）
 * - 创建并初始化 LoadScreenViewModel（MVVM_LoadScreen）
 * - 将 ViewModel 与 Widget 关联
 *
 * 初始化流程（BeginPlay）：
 *   1. 创建 LoadScreenViewModel 实例
 *   2. 创建 LoadScreenWidget 实例
 *   3. 将 ViewModel 设置到 Widget
 *   4. 调用 ViewModel.InitializeLoadSlots() 加载存档数据
 *   5. 将 Widget 添加到视口
 */
UCLASS()
class AURA_API ALoadScreenHUD : public AHUD
{
	GENERATED_BODY()
public:

	/**
	 * 加载界面 Widget 类（在 Details 面板中指定）
	 * 必须是 ULoadScreenWidget 的子类
	 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UUserWidget> LoadScreenWidgetClass;

	/**
	 * 加载界面 Widget 实例（蓝图只读）
	 * 在 BeginPlay 中创建，显示存档槽列表和操作按钮
	 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<ULoadScreenWidget> LoadScreenWidget;

	/**
	 * 加载界面 ViewModel 类（在 Details 面板中指定）
	 * 必须是 UMVVM_LoadScreen 的子类
	 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMVVM_LoadScreen> LoadScreenViewModelClass;

	/**
	 * 加载界面 ViewModel 实例（蓝图只读）
	 * 在 BeginPlay 中创建，管理存档槽数据和交互逻辑
	 */
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<UMVVM_LoadScreen> LoadScreenViewModel;

protected:
	/** 初始化：创建 ViewModel 和 Widget，加载存档数据，显示界面 */
	virtual void BeginPlay() override;
};
