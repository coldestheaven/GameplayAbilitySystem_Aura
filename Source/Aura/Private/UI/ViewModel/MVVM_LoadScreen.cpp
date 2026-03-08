// Copyright Druid Mechanics


#include "UI/ViewModel/MVVM_LoadScreen.h"

#include "Game/AuraGameInstance.h"
#include "Game/AuraGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"

/**
 * 初始化存档槽位列表
 * 
 * 实现流程：
 * 1. 创建三个存档槽位 ViewModel（LoadSlot_0, LoadSlot_1, LoadSlot_2）
 * 2. 设置每个槽位的名称和索引
 * 3. 添加到 LoadSlots 映射
 * 4. 广播槽位数量变化
 * 
 * 使用场景：
 * - LoadScreen ViewModel 创建时调用
 */
void UMVVM_LoadScreen::InitializeLoadSlots()
{
	LoadSlot_0 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlot_0->SetLoadSlotName(FString("LoadSlot_0"));
	LoadSlot_0->SlotIndex = 0;
	LoadSlots.Add(0, LoadSlot_0);
	LoadSlot_1 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlots.Add(1, LoadSlot_1);
	LoadSlot_1->SlotIndex = 1;
	LoadSlot_1->SetLoadSlotName(FString("LoadSlot_1"));
	LoadSlot_2 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlots.Add(2, LoadSlot_2);
	LoadSlot_2->SetLoadSlotName(FString("LoadSlot_2"));
	LoadSlot_2->SlotIndex = 2;

	SetNumLoadSlots(LoadSlots.Num());
}

/**
 * 根据索引获取存档槽位 ViewModel
 * 
 * @param Index 槽位索引（0, 1, 2）
 * @return 存档槽位 ViewModel 指针
 * 
 * 使用场景：
 * - 在 UI 中访问特定槽位数据时调用
 */
UMVVM_LoadSlot* UMVVM_LoadScreen::GetLoadSlotViewModelByIndex(int32 Index) const
{
	return LoadSlots.FindChecked(Index);
}

/**
 * 新存档按钮按下（创建新存档）
 * 
 * 实现流程：
 * 1. 获取 GameMode（必须是 AuraGameModeBase）
 * 2. 如果 GameMode 无效，显示错误消息
 * 3. 设置槽位数据：
 *    - 地图名称（使用默认地图）
 *    - 玩家名称（用户输入）
 *    - 玩家等级（初始为 1）
 *    - 槽位状态（Taken）
 *    - 玩家起始标签和地图资源名称
 * 4. 保存槽位数据到 GameMode
 * 
 * @param Slot 槽位索引
 * @param EnteredName 玩家输入的名称
 * 
 * 使用场景：
 * - 玩家点击"新建存档"按钮时调用
 * 
 * 注意：
 * - 新存档使用默认地图和初始等级
 */
void UMVVM_LoadScreen::NewSlotButtonPressed(int32 Slot, const FString& EnteredName)
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!IsValid(AuraGameMode))
	{
		GEngine->AddOnScreenDebugMessage(1, 15.f, FColor::Magenta, FString("Please switch to Single Player"));
		return;
	}

	// 设置新存档数据
	LoadSlots[Slot]->SetMapName(AuraGameMode->DefaultMapName);
	LoadSlots[Slot]->SetPlayerName(EnteredName);
	LoadSlots[Slot]->SetPlayerLevel(1);
	LoadSlots[Slot]->SlotStatus = Taken;
	LoadSlots[Slot]->PlayerStartTag = AuraGameMode->DefaultPlayerStartTag;
	LoadSlots[Slot]->MapAssetName = AuraGameMode->DefaultMap.ToSoftObjectPath().GetAssetName();

	// 保存到 GameMode
	AuraGameMode->SaveSlotData(LoadSlots[Slot], Slot);
	LoadSlots[Slot]->InitializeSlot();

	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(AuraGameMode->GetGameInstance());
	AuraGameInstance->LoadSlotName = LoadSlots[Slot]->GetLoadSlotName();
	AuraGameInstance->LoadSlotIndex = LoadSlots[Slot]->SlotIndex;
	AuraGameInstance->PlayerStartTag = AuraGameMode->DefaultPlayerStartTag;
}

/**
 * 新游戏按钮按下（显示名称输入界面）
 * 
 * 实现流程：
 * 1. 广播 WidgetSwitcher 索引为 1（切换到名称输入界面）
 * 
 * @param Slot 槽位索引
 * 
 * 使用场景：
 * - 玩家点击"新游戏"按钮时调用
 */
void UMVVM_LoadScreen::NewGameButtonPressed(int32 Slot)
{
	LoadSlots[Slot]->SetWidgetSwitcherIndex.Broadcast(1);
}

/**
 * 选择槽位按钮按下
 * 
 * 实现流程：
 * 1. 广播槽位选中委托
 * 2. 遍历所有槽位：
 *    - 选中的槽位：禁用选择按钮
 *    - 其他槽位：启用选择按钮
 * 3. 设置 SelectedSlot 为选中的槽位
 * 
 * @param Slot 槽位索引
 * 
 * 使用场景：
 * - 玩家点击槽位选择按钮时调用
 */
void UMVVM_LoadScreen::SelectSlotButtonPressed(int32 Slot)
{
	SlotSelected.Broadcast();
	for (const TTuple<int32, UMVVM_LoadSlot*> LoadSlot : LoadSlots)
	{
		if (LoadSlot.Key == Slot)
		{
			LoadSlot.Value->EnableSelectSlotButton.Broadcast(false);
		}
		else
		{
			LoadSlot.Value->EnableSelectSlotButton.Broadcast(true);
		}
	}
	SelectedSlot = LoadSlots[Slot];
}

/**
 * 删除按钮按下（删除选中存档）
 * 
 * 实现流程：
 * 1. 检查 SelectedSlot 是否有效
 * 2. 调用 GameMode::DeleteSlot 删除存档文件
 * 3. 设置槽位状态为 Vacant（空）
 * 4. 重新初始化槽位（更新 UI 显示）
 * 5. 启用选择按钮
 * 
 * 使用场景：
 * - 玩家点击"删除"按钮时调用
 */
void UMVVM_LoadScreen::DeleteButtonPressed()
{
	if (IsValid(SelectedSlot))
	{
		AAuraGameModeBase::DeleteSlot(SelectedSlot->GetLoadSlotName(), SelectedSlot->SlotIndex);
		SelectedSlot->SlotStatus = Vacant;
		SelectedSlot->InitializeSlot();
		SelectedSlot->EnableSelectSlotButton.Broadcast(true);
	}
}

/**
 * 开始游戏按钮按下（加载存档并传送）
 * 
 * 实现流程：
 * 1. 获取 GameMode 和 GameInstance
 * 2. 设置 GameInstance 的存档信息：
 *    - PlayerStartTag（玩家起始标签）
 *    - LoadSlotName（存档槽位名称）
 *    - LoadSlotIndex（存档槽位索引）
 * 3. 调用 GameMode::TravelToMap 传送到存档地图
 * 
 * 使用场景：
 * - 玩家点击"开始游戏"按钮时调用
 * 
 * 注意：
 * - 传送前会设置 GameInstance 的存档信息，用于加载时恢复
 */
void UMVVM_LoadScreen::PlayButtonPressed()
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
	UAuraGameInstance* AuraGameInstance = Cast<UAuraGameInstance>(AuraGameMode->GetGameInstance());
	
	// 设置 GameInstance 的存档信息
	AuraGameInstance->PlayerStartTag = SelectedSlot->PlayerStartTag;
	AuraGameInstance->LoadSlotName = SelectedSlot->GetLoadSlotName();
	AuraGameInstance->LoadSlotIndex = SelectedSlot->SlotIndex;
	
	if (IsValid(SelectedSlot))
	{
		// 传送到存档地图
		AuraGameMode->TravelToMap(SelectedSlot);
	}
}

/**
 * 加载存档数据
 * 
 * 实现流程：
 * 1. 获取 GameMode
 * 2. 遍历所有槽位：
 *    - 从 GameMode 获取存档数据（GetSaveSlotData）
 *    - 设置槽位状态（Taken/Vacant）
 *    - 设置玩家名称、地图名称、等级、起始标签
 *    - 初始化槽位（更新 UI 显示）
 * 
 * 使用场景：
 * - LoadScreen ViewModel 创建时调用
 * - 用于显示所有存档槽位的状态
 */
void UMVVM_LoadScreen::LoadData()
{
	AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!IsValid(AuraGameMode)) return;
	
	// 遍历所有槽位，加载存档数据
	for (const TTuple<int32, UMVVM_LoadSlot*> LoadSlot : LoadSlots)
	{
		ULoadScreenSaveGame* SaveObject = AuraGameMode->GetSaveSlotData(LoadSlot.Value->GetLoadSlotName(), LoadSlot.Key);

		const FString PlayerName = SaveObject->PlayerName;
		TEnumAsByte<ESaveSlotStatus> SaveSlotStatus = SaveObject->SaveSlotStatus;

		// 设置槽位数据
		LoadSlot.Value->SlotStatus = SaveSlotStatus;
		LoadSlot.Value->SetPlayerName(PlayerName);
		LoadSlot.Value->InitializeSlot();
		
		LoadSlot.Value->SetMapName(SaveObject->MapName);
		LoadSlot.Value->PlayerStartTag = SaveObject->PlayerStartTag;
		LoadSlot.Value->SetPlayerLevel(SaveObject->PlayerLevel);
	}
}

/**
 * 设置存档槽位数量（MVVM 属性设置）
 * 
 * @param InNumLoadSlots 存档槽位数量
 */
void UMVVM_LoadScreen::SetNumLoadSlots(int32 InNumLoadSlots)
{
	UE_MVVM_SET_PROPERTY_VALUE(NumLoadSlots, InNumLoadSlots);
}
