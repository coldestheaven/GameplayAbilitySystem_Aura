// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/SaveGame.h"
#include "LoadScreenSaveGame.generated.h"

class UGameplayAbility;

/**
 * 存档槽状态枚举
 * 用于标识存档槽的当前状态，控制加载界面的 UI 显示
 */
UENUM(BlueprintType)
enum ESaveSlotStatus
{
	Vacant,     // 空槽：未使用，显示"新建游戏"按钮
	EnterName,  // 输入名称中：玩家正在输入角色名，显示名称输入框
	Taken       // 已占用：有存档数据，显示存档信息和"继续游戏"按钮
};

/**
 * 单个 Actor 的存档数据结构体
 * 存储 Actor 的名称、变换和序列化的属性数据
 *
 * 使用说明：
 * - 只有标记了 SaveGame 说明符的 UPROPERTY 才会被序列化到 Bytes 中
 * - 通过 UGameplayStatics::SaveGameToSlot 时，引擎会自动处理序列化
 */
USTRUCT()
struct FSavedActor
{
	GENERATED_BODY()

	/** Actor 的名称（用于在加载时查找对应的 Actor） */
	UPROPERTY()
	FName ActorName = FName();

	/** Actor 的世界变换（位置、旋转、缩放） */
	UPROPERTY()
	FTransform Transform = FTransform();

	/**
	 * Actor 序列化数据（二进制格式）
	 * 包含所有标记了 SaveGame 说明符的属性值
	 * 加载时通过 FMemoryReader + FObjectAndNameAsStringProxyArchive 反序列化
	 */
	UPROPERTY()
	TArray<uint8> Bytes;
};

/** FSavedActor 相等比较运算符（按 ActorName 比较，用于 TArray::Find 等操作） */
inline bool operator==(const FSavedActor& Left, const FSavedActor& Right)
{
	return Left.ActorName == Right.ActorName;
}

/**
 * 单个地图的存档数据结构体
 * 存储一个地图中所有需要保存状态的 Actor 数据
 */
USTRUCT()
struct FSavedMap
{
	GENERATED_BODY()

	/** 地图资产名称（如 "/Game/Maps/Dungeon"，用于标识是哪个地图的存档） */
	UPROPERTY()
	FString MapAssetName = FString();

	/** 该地图中所有已保存 Actor 的数据列表 */
	UPROPERTY()
	TArray<FSavedActor> SavedActors;
};

/**
 * 单个技能的存档数据结构体
 * 存储技能的类型、状态、槽位和等级信息
 */
USTRUCT(BlueprintType)
struct FSavedAbility
{
	GENERATED_BODY()

	/** 技能类（用于在加载时重新赋予技能） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ClassDefaults")
	TSubclassOf<UGameplayAbility> GameplayAbility;

	/** 技能标签（唯一标识技能，用于查找和比较） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityTag = FGameplayTag();

	/**
	 * 技能状态标签（Locked/Eligible/Unlocked/Equipped）
	 * 决定技能在菜单中的显示状态
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityStatus = FGameplayTag();

	/**
	 * 技能槽位标签（绑定到哪个输入槽）
	 * 空 Tag 表示未装备到任何槽位
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilitySlot = FGameplayTag();

	/**
	 * 技能类型标签（主动/被动）
	 * 用于区分技能类型，影响装备逻辑
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FGameplayTag AbilityType = FGameplayTag();

	/** 技能当前等级（初始为 1，消耗技能点可升级） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	int32 AbilityLevel = 1;
};

/** FSavedAbility 相等比较运算符（按 AbilityTag 精确匹配，用于查找已存档的技能） */
inline bool operator==(const FSavedAbility& Left, const FSavedAbility& Right)
{
	return Left.AbilityTag.MatchesTagExact(Right.AbilityTag);
}

/**
 * 加载界面存档数据类
 *
 * 存储完整的游戏进度，包括：
 * - 存档槽元数据（名称、索引、状态）
 * - 玩家进度（等级、XP、属性点、技能点、主属性值）
 * - 技能数据（所有已解锁/装备的技能状态）
 * - 关卡状态（所有地图中已保存 Actor 的状态）
 *
 * 存档文件命名规则：SlotName + "_" + SlotIndex
 */
UCLASS()
class AURA_API ULoadScreenSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	
	/** 存档槽名称（如 "SaveSlot_0"，与 SlotIndex 共同唯一标识存档文件） */
	UPROPERTY()
	FString SlotName = FString();

	/** 存档槽索引（0、1、2，对应三个存档槽位） */
	UPROPERTY()
	int32 SlotIndex = 0;

	/** 玩家角色名称（在加载界面显示，由玩家输入） */
	UPROPERTY()
	FString PlayerName = FString("Default Name");

	/** 当前所在地图的显示名称（在加载界面显示） */
	UPROPERTY()
	FString MapName = FString("Default Map Name");

	/** 当前所在地图的资产名称（用于加载时定位地图文件） */
	UPROPERTY()
	FString MapAssetName = FString("Default Map Asset Name");

	/** 玩家出生点标签（记录最近到达的检查点，用于重生位置） */
	UPROPERTY()
	FName PlayerStartTag;

	/** 存档槽状态（Vacant/EnterName/Taken，控制加载界面 UI） */
	UPROPERTY()
	TEnumAsByte<ESaveSlotStatus> SaveSlotStatus = Vacant;

	/**
	 * 是否为首次加载此存档
	 * true：首次进入游戏，需要应用默认属性和赋予初始技能
	 * false：从存档恢复，从存档数据中读取属性和技能
	 */
	UPROPERTY()
	bool bFirstTimeLoadIn = true;

	/* ======================== 玩家进度数据 ======================== */

	/** 玩家等级（初始为 1） */
	UPROPERTY()
	int32 PlayerLevel = 1;

	/** 玩家当前 XP 总量 */
	UPROPERTY()
	int32 XP = 0;

	/** 可用技能点数量 */
	UPROPERTY()
	int32 SpellPoints = 0;

	/** 可用属性点数量 */
	UPROPERTY()
	int32 AttributePoints = 0;

	/** 力量属性值（主属性，影响物理伤害和护甲） */
	UPROPERTY()
	float Strength = 0;

	/** 智力属性值（主属性，影响法术伤害和法力值） */
	UPROPERTY()
	float Intelligence = 0;

	/** 韧性属性值（主属性，影响护甲和格挡率） */
	UPROPERTY()
	float Resilience = 0;

	/** 活力属性值（主属性，影响生命值和生命恢复） */
	UPROPERTY()
	float Vigor = 0;
	
	/* ======================== 技能数据 ======================== */

	/** 所有已解锁/装备的技能存档数据列表 */
	UPROPERTY()
	TArray<FSavedAbility> SavedAbilities;

	/* ======================== 关卡状态数据 ======================== */

	/** 所有已访问地图的状态数据列表（每个地图一个 FSavedMap） */
	UPROPERTY()
	TArray<FSavedMap> SavedMaps;

	/**
	 * 根据地图资产名称获取对应的地图存档数据
	 * @param InMapName 地图资产名称
	 * @return 对应的 FSavedMap 结构体（如果不存在则返回空结构体）
	 */
	FSavedMap GetSavedMapWithMapName(const FString& InMapName);

	/**
	 * 检查是否存在指定地图的存档数据
	 * @param InMapName 地图资产名称
	 * @return true 表示该地图有存档数据
	 */
	bool HasMap(const FString& InMapName);
};
