// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_LoadScreen.generated.h"

/** 存档槽被选中时广播（通知 UI 更新选中状态） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSlotSelected);

class UMVVM_LoadSlot;

/**
 * 加载界面 ViewModel
 *
 * 管理整个加载界面的状态和交互逻辑，包含三个存档槽的 ViewModel
 *
 * 职责：
 * - 初始化三个存档槽 ViewModel（LoadSlot_0/1/2）
 * - 从磁盘加载存档数据并更新各槽位的显示状态
 * - 处理玩家的存档槽操作（新建、选择、删除、开始游戏）
 * - 通过 MVVM FieldNotify 机制驱动 UI 更新
 *
 * 存档槽操作流程：
 *   新建游戏：NewSlotButtonPressed → 输入名称 → NewGameButtonPressed → 保存并进入游戏
 *   继续游戏：SelectSlotButtonPressed → PlayButtonPressed → 加载存档进入游戏
 *   删除存档：SelectSlotButtonPressed → DeleteButtonPressed → 删除存档文件
 *
 * MVVM 说明：
 * - NumLoadSlots 使用 FieldNotify，UI 可自动响应数量变化
 * - LoadSlots 映射表通过 GetLoadSlotViewModelByIndex 供 Widget 访问
 */
UCLASS()
class AURA_API UMVVM_LoadScreen : public UMVVMViewModelBase
{
	GENERATED_BODY()
public:

	/**
	 * 初始化所有存档槽 ViewModel
	 * 创建三个 UMVVM_LoadSlot 实例，从磁盘加载存档数据并初始化各槽位状态
	 * 在加载界面 Widget 初始化时调用
	 */
	void InitializeLoadSlots();

	/**
	 * 存档槽被选中时广播
	 * Widget 绑定此委托以响应槽位选中事件（如高亮选中槽位、启用操作按钮）
	 */
	UPROPERTY(BlueprintAssignable)
	FSlotSelected SlotSelected;

	/**
	 * 存档槽 ViewModel 类（在 Details 面板中指定）
	 * 用于创建 LoadSlot_0/1/2 实例
	 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMVVM_LoadSlot> LoadSlotViewModelClass;

	/**
	 * 根据索引获取存档槽 ViewModel（蓝图纯函数）
	 * @param Index 存档槽索引（0、1、2）
	 * @return 对应的 UMVVM_LoadSlot 实例
	 */
	UFUNCTION(BlueprintPure)
	UMVVM_LoadSlot* GetLoadSlotViewModelByIndex(int32 Index) const;

	/**
	 * 新建存档槽按钮按下（蓝图可调用）
	 * 将指定槽位状态设为 EnterName，显示名称输入框
	 * @param Slot        存档槽索引
	 * @param EnteredName 玩家输入的角色名称
	 */
	UFUNCTION(BlueprintCallable)
	void NewSlotButtonPressed(int32 Slot, const FString& EnteredName);

	/**
	 * 确认新建游戏按钮按下（蓝图可调用）
	 * 将槽位状态设为 Taken，保存存档，切换到默认地图
	 * @param Slot 存档槽索引
	 */
	UFUNCTION(BlueprintCallable)
	void NewGameButtonPressed(int32 Slot);

	/**
	 * 选择存档槽按钮按下（蓝图可调用）
	 * 记录当前选中的槽位，广播 SlotSelected 委托，启用操作按钮
	 * @param Slot 存档槽索引
	 */
	UFUNCTION(BlueprintCallable)
	void SelectSlotButtonPressed(int32 Slot);

	/**
	 * 删除存档按钮按下（蓝图可调用）
	 * 删除当前选中槽位的存档文件，将槽位状态重置为 Vacant
	 */
	UFUNCTION(BlueprintCallable)
	void DeleteButtonPressed();

	/**
	 * 开始游戏按钮按下（蓝图可调用）
	 * 加载当前选中槽位的存档，切换到对应地图
	 */
	UFUNCTION(BlueprintCallable)
	void PlayButtonPressed();

	/**
	 * 从磁盘加载所有存档槽数据
	 * 遍历三个槽位，读取存档文件并更新对应 ViewModel 的状态
	 */
	void LoadData();

	/**
	 * 设置存档槽数量（触发 FieldNotify 通知 UI）
	 * @param InNumLoadSlots 存档槽数量（固定为 3）
	 */
	void SetNumLoadSlots(int32 InNumLoadSlots);

	/** 获取存档槽数量 */
	int32 GetNumLoadSlots() const { return NumLoadSlots; }
	
private:
	/**
	 * 存档槽 ViewModel 映射表
	 * Key: 槽位索引（0、1、2）
	 * Value: 对应的 UMVVM_LoadSlot 实例
	 */
	UPROPERTY()
	TMap<int32, UMVVM_LoadSlot*> LoadSlots;

	/** 第 0 号存档槽 ViewModel */
	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_0;

	/** 第 1 号存档槽 ViewModel */
	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_1;

	/** 第 2 号存档槽 ViewModel */
	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlot_2;

	/** 当前选中的存档槽 ViewModel（由 SelectSlotButtonPressed 设置） */
	UPROPERTY()
	UMVVM_LoadSlot* SelectedSlot;

	/**
	 * 存档槽数量（支持 MVVM FieldNotify，UI 可自动响应变化）
	 * 固定为 3，但通过 FieldNotify 机制通知 UI 初始化
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"));
	int32 NumLoadSlots;
};
