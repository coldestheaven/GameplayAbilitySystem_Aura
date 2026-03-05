// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "Game/LoadScreenSaveGame.h"
#include "MVVM_LoadSlot.generated.h"

/** 委托：用于设置 WidgetSwitcher 的显示索引，切换存档槽的 UI 状态 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSetWidgetSwitcherIndex, int32, WidgetSwitcherIndex);

/** 委托：用于控制"选择存档槽"按钮的启用/禁用状态 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnableSelectSlotButton, bool, bEnable);

/**
 * 存档槽 ViewModel
 * 负责管理加载界面中单个存档槽的数据绑定与状态通知
 */
UCLASS()
class AURA_API UMVVM_LoadSlot : public UMVVMViewModelBase
{
	GENERATED_BODY()
public:

	/** 广播事件：切换 WidgetSwitcher 显示的子页面索引 */
	UPROPERTY(BlueprintAssignable)
	FSetWidgetSwitcherIndex SetWidgetSwitcherIndex;

	/** 广播事件：启用或禁用"选择存档槽"按钮 */
	UPROPERTY(BlueprintAssignable)
	FEnableSelectSlotButton EnableSelectSlotButton;

	/** 初始化存档槽，根据当前 SlotStatus 广播对应的 UI 状态 */
	void InitializeSlot();

	/** 存档槽索引（对应存档文件的编号） */
	UPROPERTY()
	int32 SlotIndex;

	/** 存档槽当前状态（空槽、已占用等） */
	UPROPERTY()
	TEnumAsByte<ESaveSlotStatus> SlotStatus;

	/** 玩家出生点标签，用于确定进入地图时的起始位置 */
	UPROPERTY()
	FName PlayerStartTag;

	/** 地图资产名称，用于标识存档对应的地图 */
	UPROPERTY()
	FString MapAssetName;
	
	/** Field Notifies */

	/** 设置玩家名称，并触发 FieldNotify 通知 UI 更新 */
	void SetPlayerName(FString InPlayerName);

	/** 设置地图名称，并触发 FieldNotify 通知 UI 更新 */
	void SetMapName(FString InMapName);

	/** 设置玩家等级，并触发 FieldNotify 通知 UI 更新 */
	void SetPlayerLevel(int32 InLevel);

	/** 设置存档槽名称，并触发 FieldNotify 通知 UI 更新 */
	void SetLoadSlotName(FString InLoadSlotName);

	/** 获取玩家名称 */
	FString GetPlayerName() const { return PlayerName; }

	/** 获取地图名称 */
	FString GetMapName() const { return MapName; }

	/** 获取玩家等级 */
	int32 GetPlayerLevel() const { return PlayerLevel; }

	/** 获取存档槽名称 */
	FString GetLoadSlotName() const { return LoadSlotName; }

private:

	/** 玩家名称（支持 MVVM FieldNotify，UI 可自动响应变化） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"));
	FString PlayerName;

	/** 地图名称（支持 MVVM FieldNotify，UI 可自动响应变化） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"));
	FString MapName;

	/** 玩家等级（支持 MVVM FieldNotify，UI 可自动响应变化） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"));
	int32 PlayerLevel;

	/** 存档槽名称（支持 MVVM FieldNotify，UI 可自动响应变化） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"));
	FString LoadSlotName;
};
