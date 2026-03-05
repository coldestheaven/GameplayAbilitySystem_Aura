// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "AuraPlayerState.generated.h"

class UAbilitySystemComponent;
class UAttributeSet;
class ULevelUpInfo;

/** 玩家单一数值属性变化时广播（XP、属性点、技能点） */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChanged, int32 /*StatValue*/)

/** 玩家等级变化时广播（携带新等级和是否为升级事件） */
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnLevelChanged, int32 /*StatValue*/, bool /*bLevelUp*/)

/**
 * Aura 玩家状态类
 *
 * 职责：
 * - 持有 AbilitySystemComponent 和 AttributeSet（跨关卡持久化）
 * - 存储玩家进度数据：等级、XP、属性点、技能点
 * - 提供数据变化委托，供 UI 系统监听
 *
 * 网络说明：
 * - PlayerState 在服务端和所有客户端上都存在
 * - Level、XP、AttributePoints、SpellPoints 均通过 ReplicatedUsing 同步
 * - ASC 存储在 PlayerState 上，确保跨关卡（无缝旅行）时数据不丢失
 *
 * 使用示例：
 *   // 监听 XP 变化（在 WidgetController 中）
 *   PlayerState->OnXPChangedDelegate.AddUObject(this, &UMyController::OnXPChanged);
 *   // 增加 XP
 *   PlayerState->AddToXP(100);
 */
UCLASS()
class AURA_API AAuraPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
public:
	AAuraPlayerState();

	/** 注册需要网络同步的属性（Level、XP、AttributePoints、SpellPoints） */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 实现 IAbilitySystemInterface，返回玩家的 ASC */
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** 获取属性集 */
	UAttributeSet* GetAttributeSet() const { return AttributeSet; }

	/**
	 * 升级信息数据资产
	 * 定义每个等级所需的 XP 阈值、属性点奖励和技能点奖励
	 * 在 Details 面板中指定
	 */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<ULevelUpInfo> LevelUpInfo;

	/** XP 变化时广播（UI 经验条监听此委托） */
	FOnPlayerStatChanged OnXPChangedDelegate;

	/** 等级变化时广播（UI 等级显示监听此委托，bLevelUp=true 时触发升级特效） */
	FOnLevelChanged OnLevelChangedDelegate;

	/** 属性点变化时广播（属性菜单 UI 监听此委托） */
	FOnPlayerStatChanged OnAttributePointsChangedDelegate;

	/** 技能点变化时广播（技能菜单 UI 监听此委托） */
	FOnPlayerStatChanged OnSpellPointsChangedDelegate;

	/** 获取当前等级 */
	FORCEINLINE int32 GetPlayerLevel() const { return Level; }

	/** 获取当前 XP 总量 */
	FORCEINLINE int32 GetXP() const { return XP; }

	/** 获取当前可用属性点数量 */
	FORCEINLINE int32 GetAttributePoints() const { return AttributePoints; }

	/** 获取当前可用技能点数量 */
	FORCEINLINE int32 GetSpellPoints() const { return SpellPoints; }

	/**
	 * 增加 XP（累加到当前值）
	 * @param InXP 要增加的 XP 数量
	 */
	void AddToXP(int32 InXP);

	/**
	 * 增加等级（累加到当前值）
	 * @param InLevel 要增加的等级数
	 */
	void AddToLevel(int32 InLevel);

	/**
	 * 增加属性点（累加到当前值）
	 * @param InPoints 要增加的属性点数
	 */
	void AddToAttributePoints(int32 InPoints);

	/**
	 * 增加技能点（累加到当前值）
	 * @param InPoints 要增加的技能点数
	 */
	void AddToSpellPoints(int32 InPoints);
	
	/**
	 * 直接设置 XP（用于从存档加载）
	 * @param InXP 要设置的 XP 值
	 */
	void SetXP(int32 InXP);

	/**
	 * 直接设置等级（用于从存档加载）
	 * @param InLevel 要设置的等级
	 */
	void SetLevel(int32 InLevel);

	/**
	 * 直接设置属性点（用于从存档加载）
	 * @param InPoints 要设置的属性点数
	 */
	void SetAttributePoints(int32 InPoints);

	/**
	 * 直接设置技能点（用于从存档加载）
	 * @param InPoints 要设置的技能点数
	 */
	void SetSpellPoints(int32 InPoints);
	
protected:
	/**
	 * 能力系统组件（存储在 PlayerState 上，跨关卡持久化）
	 * 玩家角色的 ASC 通过 GetAbilitySystemComponent() 返回此组件
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** 属性集（存储在 PlayerState 上，跨关卡持久化） */
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

private:
	/**
	 * 玩家当前等级（网络同步，变化时触发 OnRep_Level）
	 * 初始值为 1
	 */
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_Level)
	int32 Level = 1;

	/**
	 * 玩家当前 XP 总量（网络同步，变化时触发 OnRep_XP）
	 * 初始值为 0
	 */
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_XP)
	int32 XP = 0;

	/**
	 * 可用属性点数量（网络同步，变化时触发 OnRep_AttributePoints）
	 * 升级时获得，用于提升主属性
	 */
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_AttributePoints)
	int32 AttributePoints = 0;

	/**
	 * 可用技能点数量（网络同步，变化时触发 OnRep_SpellPoints）
	 * 升级时获得，用于解锁/升级技能
	 */
	UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_SpellPoints)
	int32 SpellPoints = 0;
	
	/**
	 * 等级同步回调：广播 OnLevelChangedDelegate
	 * @param OldLevel 同步前的旧等级
	 */
	UFUNCTION()
	void OnRep_Level(int32 OldLevel);

	/**
	 * XP 同步回调：广播 OnXPChangedDelegate
	 * @param OldXP 同步前的旧 XP 值
	 */
	UFUNCTION()
	void OnRep_XP(int32 OldXP);

	/**
	 * 属性点同步回调：广播 OnAttributePointsChangedDelegate
	 * @param OldAttributePoints 同步前的旧属性点数
	 */
	UFUNCTION()
	void OnRep_AttributePoints(int32 OldAttributePoints);

	/**
	 * 技能点同步回调：广播 OnSpellPointsChangedDelegate
	 * @param OldSpellPoints 同步前的旧技能点数
	 */
	UFUNCTION()
	void OnRep_SpellPoints(int32 OldSpellPoints);
};
