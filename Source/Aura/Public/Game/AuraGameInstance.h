// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "AuraGameInstance.generated.h"

/**
 * Aura 游戏实例
 *
 * 在整个游戏生命周期内持久存在（跨关卡），用于传递关卡切换时的临时数据
 *
 * 存储的数据：
 * - PlayerStartTag：玩家在目标关卡的出生点标签
 * - LoadSlotName：当前使用的存档槽名称
 * - LoadSlotIndex：当前使用的存档槽索引
 *
 * 使用场景：
 * - 关卡切换时（TravelToMap），GameMode 将目标出生点信息写入 GameInstance
 * - 新关卡加载后，GameMode 的 ChoosePlayerStart_Implementation 从 GameInstance 读取出生点标签
 * - 存档系统通过 LoadSlotName + LoadSlotIndex 定位存档文件
 *
 * 注意：GameInstance 中的数据不参与网络同步，只在本地客户端有效
 */
UCLASS()
class AURA_API UAuraGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:

	/**
	 * 玩家出生点标签
	 * 关卡切换时记录目标出生点的 PlayerStartTag
	 * GameMode 的 ChoosePlayerStart_Implementation 使用此标签查找对应的 PlayerStart Actor
	 */
	UPROPERTY()
	FName PlayerStartTag = FName();

	/**
	 * 当前存档槽名称
	 * 与 LoadSlotIndex 共同唯一标识存档文件
	 * 格式通常为 "SaveSlot_0"、"SaveSlot_1"、"SaveSlot_2"
	 */
	UPROPERTY()
	FString LoadSlotName = FString();

	/**
	 * 当前存档槽索引（0、1、2）
	 * 与 LoadSlotName 共同唯一标识存档文件
	 * 用于 UGameplayStatics::SaveGameToSlot 和 LoadGameFromSlot
	 */
	UPROPERTY()
	int32 LoadSlotIndex = 0;
};
