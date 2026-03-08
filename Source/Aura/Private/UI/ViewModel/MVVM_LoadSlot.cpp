// Copyright Druid Mechanics


#include "UI/ViewModel/MVVM_LoadSlot.h"

/**
 * 初始化存档槽位
 * 
 * 实现流程：
 * 1. 获取槽位状态值（Taken/Empty）
 * 2. 广播 WidgetSwitcher 索引（用于切换 UI 显示）
 * 
 * 使用场景：
 * - 存档槽位 ViewModel 创建时调用
 * - 用于初始化 UI 显示状态（显示"已占用"或"空"）
 */
void UMVVM_LoadSlot::InitializeSlot()
{
	const int32 WidgetSwitcherIndex = SlotStatus.GetValue();
	SetWidgetSwitcherIndex.Broadcast(WidgetSwitcherIndex);
}

/**
 * 设置玩家名称（MVVM 属性设置）
 * 
 * @param InPlayerName 玩家名称
 * 
 * 使用场景：
 * - 加载存档数据时设置
 * - 创建新存档时设置
 */
void UMVVM_LoadSlot::SetPlayerName(FString InPlayerName)
{
	UE_MVVM_SET_PROPERTY_VALUE(PlayerName, InPlayerName);
}

/**
 * 设置地图名称（MVVM 属性设置）
 * 
 * @param InMapName 地图名称
 */
void UMVVM_LoadSlot::SetMapName(FString InMapName)
{
	UE_MVVM_SET_PROPERTY_VALUE(MapName, InMapName);
}

/**
 * 设置玩家等级（MVVM 属性设置）
 * 
 * @param InLevel 玩家等级
 */
void UMVVM_LoadSlot::SetPlayerLevel(int32 InLevel)
{
	UE_MVVM_SET_PROPERTY_VALUE(PlayerLevel, InLevel);
}

/**
 * 设置存档槽位名称（MVVM 属性设置）
 * 
 * @param InLoadSlotName 存档槽位名称（如 "LoadSlot_0"）
 */
void UMVVM_LoadSlot::SetLoadSlotName(FString InLoadSlotName)
{
	UE_MVVM_SET_PROPERTY_VALUE(LoadSlotName, InLoadSlotName);
}
