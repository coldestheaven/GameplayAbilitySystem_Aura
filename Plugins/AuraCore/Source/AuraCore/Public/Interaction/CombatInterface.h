// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "CombatInterface.generated.h"

class UAbilitySystemComponent;
class UNiagaraSystem;
class UAnimMontage;

/**
 * 角色职业枚举
 * 决定角色使用哪套属性曲线、初始技能和 XP 奖励
 */
UENUM(BlueprintType)
enum class ECharacterClass : uint8
{
	Elementalist,  // 元素法师：高法术伤害，低生命值
	Warrior,       // 战士：高生命值和护甲，近战攻击
	Ranger         // 游侠：远程攻击，中等属性
};

/** ASC 注册完成时广播（携带 ASC 指针，供 UI 系统绑定属性监听） */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnASCRegistered, UAbilitySystemComponent*)

/** 角色死亡时广播（携带死亡的 Actor，供 AI 和 UI 系统响应） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeathSignature, AActor*, DeadActor);

/** 角色受到伤害时广播（携带伤害数值，供伤害数字显示系统使用） */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDamageSignature, float /*DamageAmount*/);

/**
 * 带标签的动画蒙太奇结构体
 * 将动画蒙太奇与 GameplayTag 关联，用于标识攻击类型和战斗插槽
 *
 * 使用示例：
 *   // 在 AuraCharacterBase 的 AttackMontages 数组中配置
 *   FTaggedMontage AttackMontage;
 *   AttackMontage.Montage = AM_Attack_Sword;
 *   AttackMontage.MontageTag = Montage.Attack.Weapon;  // 标识此蒙太奇类型
 *   AttackMontage.SocketTag = CombatSocket.Weapon;     // 标识使用哪个插槽位置
 *   AttackMontage.ImpactSound = S_SwordHit;            // 命中音效
 */
USTRUCT(BlueprintType)
struct FTaggedMontage
{
	GENERATED_BODY()

	/** 动画蒙太奇资产（攻击动画） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* Montage = nullptr;

	/** 蒙太奇类型标签（如 Montage.Attack.Weapon，用于按类型查找蒙太奇） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag MontageTag;

	/** 战斗插槽标签（如 CombatSocket.Weapon，标识伤害判定的位置） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag SocketTag;

	/** 命中音效（攻击命中目标时播放） */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* ImpactSound = nullptr;
};

/** UInterface 声明（不需要修改） */
UINTERFACE(MinimalAPI, BlueprintType)
class UCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 战斗接口
 *
 * 所有参与战斗的角色（玩家和敌人）都实现此接口
 * 提供战斗系统所需的通用功能：
 * - 等级查询（GetPlayerLevel）
 * - 战斗插槽位置（GetCombatSocketLocation）
 * - 受击动画（GetHitReactMontage）
 * - 死亡处理（Die）
 * - 攻击蒙太奇（GetAttackMontages）
 * - 召唤物管理（GetMinionCount/IncrementMinionCount）
 * - 电击状态（IsBeingShocked/SetIsBeingShocked）
 */
class AURACORE_API ICombatInterface
{
	GENERATED_BODY()
public:
	/**
	 * 获取角色等级（蓝图原生事件）
	 * 玩家从 PlayerState 获取，敌人从自身属性获取
	 * @return 当前角色等级
	 */
	UFUNCTION(BlueprintNativeEvent)
	int32 GetPlayerLevel();

	/**
	 * 获取战斗插槽的世界位置（蓝图原生事件，蓝图可调用）
	 * 根据 MontageTag 返回对应插槽的世界坐标（武器尖端、左手、右手等）
	 * 用于投射物生成位置和近战伤害判定位置
	 * @param MontageTag 标识插槽类型的 GameplayTag
	 * @return 插槽的世界坐标
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FVector GetCombatSocketLocation(const FGameplayTag& MontageTag);

	/**
	 * 更新角色面朝目标方向（蓝图实现事件，蓝图可调用）
	 * 在攻击时调用，使角色面向攻击目标
	 * @param Target 目标世界坐标
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateFacingTarget(const FVector& Target);

	/**
	 * 获取受击动画蒙太奇（蓝图原生事件，蓝图可调用）
	 * 被攻击时播放的受击反应动画
	 * @return 受击动画蒙太奇
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetHitReactMontage();

	/**
	 * 角色死亡处理（纯虚函数，子类必须实现）
	 * 处理死亡逻辑：布娃娃、溶解特效、死亡音效等
	 * @param DeathImpulse 死亡冲量（用于布娃娃物理击飞效果）
	 */
	virtual void Die(const FVector& DeathImpulse) = 0;

	/** 获取死亡委托引用（纯虚函数，子类必须实现） */
	virtual FOnDeathSignature& GetOnDeathDelegate() = 0;

	/** 获取伤害委托引用（纯虚函数，子类必须实现） */
	virtual FOnDamageSignature& GetOnDamageSignature() = 0; 

	/**
	 * 判断角色是否已死亡（蓝图原生事件，蓝图可调用）
	 * @return true 表示角色已死亡
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsDead() const;

	/**
	 * 获取 Avatar Actor（蓝图原生事件，蓝图可调用）
	 * 返回实际的角色 Pawn（通常为 this）
	 * @return Avatar Actor 指针
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	AActor* GetAvatar();

	/**
	 * 获取所有带标签的攻击蒙太奇（蓝图原生事件，蓝图可调用）
	 * 返回角色配置的所有攻击动画及其对应的标签
	 * @return 带标签蒙太奇数组
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	TArray<FTaggedMontage> GetAttackMontages();

	/**
	 * 获取死亡血液特效（蓝图原生事件，蓝图可调用）
	 * 角色死亡时播放的 Niagara 粒子特效
	 * @return 血液 Niagara 特效资产
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UNiagaraSystem* GetBloodEffect();

	/**
	 * 根据标签获取对应的带标签蒙太奇（蓝图原生事件，蓝图可调用）
	 * @param MontageTag 要查找的蒙太奇标签
	 * @return 对应的 FTaggedMontage 结构体
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FTaggedMontage GetTaggedMontageByTag(const FGameplayTag& MontageTag);

	/**
	 * 获取当前召唤物数量（蓝图原生事件，蓝图可调用）
	 * @return 当前存活的召唤物数量
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	int32 GetMinionCount();

	/**
	 * 增加/减少召唤物计数（蓝图原生事件，蓝图可调用）
	 * @param Amount 正数增加，负数减少
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void IncrementMinionCount(int32 Amount);

	/**
	 * 获取角色职业类型（蓝图原生事件，蓝图可调用）
	 * @return 角色职业枚举值
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	ECharacterClass GetCharacterClass();

	/** 获取 ASC 注册完成委托引用（纯虚函数，子类必须实现） */
	virtual FOnASCRegistered& GetOnASCRegisteredDelegate() = 0;

	/**
	 * 设置电击循环状态（蓝图实现事件，蓝图可调用）
	 * 控制电击特效的循环播放（在蓝图中实现）
	 * @param bInLoop true 表示开始循环，false 表示停止循环
	 */
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void SetInShockLoop(bool bInLoop);

	/**
	 * 获取武器骨骼网格体（蓝图原生事件，蓝图可调用）
	 * @return 武器的骨骼网格体组件
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	USkeletalMeshComponent* GetWeapon();

	/**
	 * 判断角色是否正在被电击（蓝图原生事件，蓝图可调用）
	 * @return true 表示正在被电击
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool IsBeingShocked() const;

	/**
	 * 设置角色的电击状态（蓝图原生事件，蓝图可调用）
	 * @param bInShock true 表示进入电击状态，false 表示退出
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void SetIsBeingShocked(bool bInShock);
};
