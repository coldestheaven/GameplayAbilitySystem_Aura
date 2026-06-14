// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "AuraGameModeBase.generated.h"

class ULootTiers;
class ULoadScreenSaveGame;
class USaveGame;
class UMVVM_LoadSlot;
class UAbilityInfo;
class UAttributeInfo;
class UCharacterClassInfo;

/**
 * Aura 游戏模式基类
 *
 * 职责：
 * - 持有全局数据资产（角色职业信息、技能信息、战利品等级）
 * - 管理存档系统（保存/加载/删除存档槽）
 * - 管理关卡状态的序列化（Actor 状态保存/加载）
 * - 处理关卡切换（TravelToMap）
 * - 处理玩家死亡后的重生逻辑
 *
 * 存档系统说明：
 * - 每个存档槽由 SlotName + SlotIndex 唯一标识
 * - 存档数据包含：玩家进度（等级/XP/属性/技能）+ 关卡状态（Actor 变换和序列化数据）
 * - 关卡状态按地图资产名称分组存储，支持多地图存档
 */
UCLASS()
class AURA_API AAuraGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	/**
	 * 角色职业信息数据资产
	 * 定义每种职业的初始属性 GE、初始技能列表和 XP 奖励曲线
	 * 在 Details 面板中指定
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
	TObjectPtr<UCharacterClassInfo> CharacterClassInfo;

	/**
	 * 技能信息数据资产
	 * 定义所有技能的图标、描述、解锁等级等 UI 相关信息
	 * 在 Details 面板中指定
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Ability Info")
	TObjectPtr<UAbilityInfo> AbilityInfo;

	/**
	 * 属性信息数据资产（C1 重构新增 · 2026-06-14）
	 *
	 * 定义所有玩家属性（力量/智力/韧性/活力等）的显示名称、描述文本、UI 图标等
	 * 提供给 UAuraAbilitySystemLibrary::GetAttributeInfo 作为统一访问入口的数据源
	 *
	 * 兼容性说明：
	 * - 此字段为纯新增字段，蓝图 BP_AuraGameMode 现有配置完全不受影响
	 * - 旧的 UAttributeMenuWidgetController::AttributeInfo 字段仍然保留，不强制迁移
	 * - 新增/外部代码建议优先走 Library 入口，便于后续渐进式收拢
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Attribute Info")
	TObjectPtr<UAttributeInfo> AttributeInfo;

	/**
	 * 战利品等级数据资产
	 * 定义不同等级敌人的掉落物品类型和概率
	 * 在 Details 面板中指定
	 */
	UPROPERTY(EditDefaultsOnly, Category = "Loot Tiers")
	TObjectPtr<ULootTiers> LootTiers;

	/**
	 * 保存存档槽数据
	 * 将 LoadSlot ViewModel 中的数据序列化并保存到磁盘
	 * @param LoadSlot   包含存档信息的 ViewModel
	 * @param SlotIndex  存档槽索引（0、1、2）
	 */
	void SaveSlotData(UMVVM_LoadSlot* LoadSlot, int32 SlotIndex);

	/**
	 * 获取存档槽数据
	 * 从磁盘加载指定存档槽的数据，如果不存在则创建新的存档对象
	 * @param SlotName   存档槽名称
	 * @param SlotIndex  存档槽索引
	 * @return 存档数据对象（调用者负责管理生命周期）
	 */
	ULoadScreenSaveGame* GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const;

	/**
	 * 删除指定存档槽
	 * @param SlotName   存档槽名称
	 * @param SlotIndex  存档槽索引
	 */
	static void DeleteSlot(const FString& SlotName, int32 SlotIndex);

	/**
	 * 获取游戏内进度存档数据（当前正在游玩的存档）
	 * 从 GameInstance 中获取当前存档的 SlotName 和 SlotIndex，然后加载存档
	 * @return 当前游戏进度的存档数据对象
	 */
	ULoadScreenSaveGame* RetrieveInGameSaveData();

	/**
	 * 保存游戏内进度数据（检查点触发时调用）
	 * 将玩家当前状态（等级、XP、属性、技能）写入存档并保存到磁盘
	 * @param SaveObject 要保存的存档数据对象
	 */
	void SaveInGameProgressData(ULoadScreenSaveGame* SaveObject);

	/**
	 * 保存当前关卡的世界状态（所有实现 ISaveInterface 的 Actor 的状态）
	 * 遍历关卡中所有 Actor，将标记了 SaveGame 的属性序列化到存档
	 * @param World                  要保存状态的世界
	 * @param DestinationMapAssetName 目标地图资产名称（空字符串表示保存当前地图）
	 */
	void SaveWorldState(UWorld* World, const FString& DestinationMapAssetName = FString("")) const;

	/**
	 * 加载关卡的世界状态（进入关卡时调用）
	 * 从存档中恢复所有 Actor 的状态（变换、序列化属性）
	 * @param World 要恢复状态的世界
	 */
	void LoadWorldState(UWorld* World) const;

	/**
	 * 切换到指定存档槽对应的地图
	 * 保存当前关卡状态后，无缝旅行到目标地图
	 * @param Slot 包含目标地图信息的存档槽 ViewModel
	 */
	void TravelToMap(UMVVM_LoadSlot* Slot);

	/**
	 * 存档数据类（用于 CreateSaveGameObject）
	 * 在 Details 面板中指定，默认为 ULoadScreenSaveGame
	 */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<USaveGame> LoadScreenSaveGameClass;

	/** 默认地图名称（新游戏时使用的地图显示名称） */
	UPROPERTY(EditDefaultsOnly)
	FString DefaultMapName;

	/** 默认地图资产（新游戏时加载的地图） */
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UWorld> DefaultMap;

	/** 默认玩家出生点标签（新游戏时的初始出生位置） */
	UPROPERTY(EditDefaultsOnly)
	FName DefaultPlayerStartTag;

	/**
	 * 地图名称到地图资产的映射表
	 * Key: 地图显示名称（如 "Dungeon"）
	 * Value: 地图资产软引用
	 * 用于 TravelToMap 时根据名称查找地图资产
	 */
	UPROPERTY(EditDefaultsOnly)
	TMap<FString, TSoftObjectPtr<UWorld>> Maps;

	/**
	 * 根据地图资产名称获取地图显示名称
	 * @param MapAssetName 地图资产名称（如 "/Game/Maps/Dungeon"）
	 * @return 地图显示名称（如 "Dungeon"）
	 */
	FString GetMapNameFromMapAssetName(const FString& MapAssetName) const;

	/**
	 * 选择玩家出生点（重写基类）
	 * 根据存档中记录的 PlayerStartTag 查找对应的 PlayerStart Actor
	 * @param Player 请求出生点的控制器
	 * @return 选中的 PlayerStart Actor
	 */
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;

	/**
	 * 处理玩家死亡
	 * 重新加载当前关卡（从最近的检查点重生）
	 * @param DeadCharacter 死亡的玩家角色
	 */
	void PlayerDied(ACharacter* DeadCharacter);

protected:
	virtual void BeginPlay() override;
};
