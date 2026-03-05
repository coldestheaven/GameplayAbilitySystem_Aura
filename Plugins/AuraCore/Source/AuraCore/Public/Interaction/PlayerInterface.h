// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PlayerInterface.generated.h"

/** UInterface 声明（不需要修改） */
UINTERFACE(MinimalAPI)
class UPlayerInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 玩家接口
 *
 * 玩家角色（AAuraCharacter）实现此接口，提供玩家专属的功能
 * 与 ICombatInterface 的区别：此接口只有玩家角色实现，敌人不实现
 *
 * 功能分类：
 * - XP 系统：AddToXP、GetXP、FindLevelForXP
 * - 等级系统：AddToPlayerLevel、GetAttributePointsReward、GetSpellPointsReward
 * - 属性点/技能点：AddToAttributePoints、GetAttributePoints、AddToSpellPoints、GetSpellPoints
 * - 升级事件：LevelUp（触发升级特效和音效）
 * - UI 辅助：ShowMagicCircle、HideMagicCircle
 * - 存档：SaveProgress
 */
class AURACORE_API IPlayerInterface
{
	GENERATED_BODY()
public:
	/**
	 * 根据 XP 总量查找对应等级（蓝图原生事件）
	 * @param InXP 要查询的 XP 值
	 * @return 对应的角色等级
	 */
	UFUNCTION(BlueprintNativeEvent)
	int32 FindLevelForXP(int32 InXP) const;

	/**
	 * 获取当前 XP 总量（蓝图原生事件）
	 * @return 当前 XP 值
	 */
	UFUNCTION(BlueprintNativeEvent)
	int32 GetXP() const;

	/**
	 * 获取指定等级升级时奖励的属性点数量（蓝图原生事件）
	 * @param Level 目标等级
	 * @return 属性点奖励数量
	 */
	UFUNCTION(BlueprintNativeEvent)
	int32 GetAttributePointsReward(int32 Level) const;

	/**
	 * 获取指定等级升级时奖励的技能点数量（蓝图原生事件）
	 * @param Level 目标等级
	 * @return 技能点奖励数量
	 */
	UFUNCTION(BlueprintNativeEvent)
	int32 GetSpellPointsReward(int32 Level) const;
	
	/**
	 * 增加 XP（蓝图原生事件）
	 * @param InXP 要增加的 XP 数量
	 */
	UFUNCTION(BlueprintNativeEvent)
	void AddToXP(int32 InXP);

	/**
	 * 增加玩家等级（蓝图原生事件）
	 * @param InPlayerLevel 要增加的等级数
	 */
	UFUNCTION(BlueprintNativeEvent)
	void AddToPlayerLevel(int32 InPlayerLevel);

	/**
	 * 增加属性点（蓝图原生事件）
	 * @param InAttributePoints 要增加的属性点数
	 */
	UFUNCTION(BlueprintNativeEvent)
	void AddToAttributePoints(int32 InAttributePoints);

	/**
	 * 获取当前可用属性点数量（蓝图原生事件）
	 * @return 可用属性点数量
	 */
	UFUNCTION(BlueprintNativeEvent)
	int32 GetAttributePoints() const;

	/**
	 * 增加技能点（蓝图原生事件）
	 * @param InSpellPoints 要增加的技能点数
	 */
	UFUNCTION(BlueprintNativeEvent)
	void AddToSpellPoints(int32 InSpellPoints);

	/**
	 * 获取当前可用技能点数量（蓝图原生事件）
	 * @return 可用技能点数量
	 */
	UFUNCTION(BlueprintNativeEvent)
	int32 GetSpellPoints() const;
	
	/**
	 * 触发升级逻辑（蓝图原生事件）
	 * 播放升级粒子特效（MulticastLevelUpParticles）
	 * 通常在 AttributeSet 检测到 XP 达到升级阈值后调用
	 */
	UFUNCTION(BlueprintNativeEvent)
	void LevelUp();

	/**
	 * 在地面显示魔法圆圈（蓝图原生事件，蓝图可调用）
	 * 生成 MagicCircle Actor 并跟随鼠标位置
	 * @param DecalMaterial 圆圈贴花材质（nullptr 使用默认材质）
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void ShowMagicCircle(UMaterialInterface* DecalMaterial = nullptr);

	/**
	 * 隐藏魔法圆圈（蓝图原生事件，蓝图可调用）
	 * 销毁当前的 MagicCircle Actor
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void HideMagicCircle();

	/**
	 * 保存游戏进度（蓝图原生事件，蓝图可调用）
	 * 将玩家当前状态写入存档并保存到磁盘
	 * @param CheckpointTag 触发保存的检查点标签（记录重生位置）
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SaveProgress(const FName& CheckpointTag);
};
