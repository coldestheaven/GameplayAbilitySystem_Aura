// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/PlayerInterface.h"
#include "AuraCharacter.generated.h"

class UNiagaraComponent;
class UCameraComponent;
class USpringArmComponent;

/**
 * 玩家控制的 Aura 角色类
 *
 * 职责：
 * - 继承 AAuraCharacterBase，额外实现 IPlayerInterface（玩家专属接口）
 * - 管理摄像机（弹簧臂 + 俯视摄像机）
 * - 处理玩家等级提升、XP 获取、属性点/技能点分配
 * - 管理存档进度的加载与保存
 * - 在服务端（PossessedBy）和客户端（OnRep_PlayerState）分别初始化 ASC
 *
 * 网络说明：
 * - ASC 和 AttributeSet 存储在 PlayerState 上（跨关卡持久化）
 * - PossessedBy：服务端初始化 ASC ActorInfo 并赋予技能
 * - OnRep_PlayerState：客户端初始化 ASC ActorInfo
 */
UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase, public IPlayerInterface
{
	GENERATED_BODY()
public:
	AAuraCharacter();

	/** 服务端：角色被控制器接管时调用，初始化 ASC 并赋予初始技能 */
	virtual void PossessedBy(AController* NewController) override;

	/** 客户端：PlayerState 同步完成后调用，初始化客户端侧的 ASC ActorInfo */
	virtual void OnRep_PlayerState() override;

	/** ======================== Player Interface 实现 ======================== */

	/**
	 * 增加经验值
	 * @param InXP 要增加的 XP 数量
	 */
	virtual void AddToXP_Implementation(int32 InXP) override;

	/** 触发升级逻辑（播放升级特效、分配属性点和技能点） */
	virtual void LevelUp_Implementation() override;

	/** 返回当前 XP 总量 */
	virtual int32 GetXP_Implementation() const override;

	/**
	 * 根据 XP 数量反查对应的等级
	 * @param InXP 要查询的 XP 值
	 * @return 对应的角色等级
	 */
	virtual int32 FindLevelForXP_Implementation(int32 InXP) const override;

	/**
	 * 获取指定等级升级时奖励的属性点数量
	 * @param Level 目标等级
	 */
	virtual int32 GetAttributePointsReward_Implementation(int32 Level) const override;

	/**
	 * 获取指定等级升级时奖励的技能点数量
	 * @param Level 目标等级
	 */
	virtual int32 GetSpellPointsReward_Implementation(int32 Level) const override;

	/**
	 * 增加玩家等级
	 * @param InPlayerLevel 要增加的等级数
	 */
	virtual void AddToPlayerLevel_Implementation(int32 InPlayerLevel) override;

	/**
	 * 增加属性点
	 * @param InAttributePoints 要增加的属性点数
	 */
	virtual void AddToAttributePoints_Implementation(int32 InAttributePoints) override;

	/**
	 * 增加技能点
	 * @param InSpellPoints 要增加的技能点数
	 */
	virtual void AddToSpellPoints_Implementation(int32 InSpellPoints) override;

	/** 返回当前可用属性点数量 */
	virtual int32 GetAttributePoints_Implementation() const override;

	/** 返回当前可用技能点数量 */
	virtual int32 GetSpellPoints_Implementation() const override;

	/**
	 * 在地面显示魔法圆圈（用于需要指定目标位置的技能）
	 * @param DecalMaterial 圆圈使用的贴花材质（可为 nullptr 使用默认材质）
	 */
	virtual void ShowMagicCircle_Implementation(UMaterialInterface* DecalMaterial) override;

	/** 隐藏魔法圆圈 */
	virtual void HideMagicCircle_Implementation() override;

	/**
	 * 保存游戏进度到存档
	 * @param CheckpointTag 触发保存的检查点标签，用于记录重生位置
	 */
	virtual void SaveProgress_Implementation(const FName& CheckpointTag) override;

	/** ======================== end Player Interface ======================== */

	/** ======================== Combat Interface 重写 ======================== */

	/** 返回玩家当前等级（从 PlayerState 获取） */
	virtual int32 GetPlayerLevel_Implementation() override;

	/**
	 * 玩家死亡处理（重写基类）
	 * 额外逻辑：启动死亡计时器，倒计时结束后重新加载关卡
	 * @param DeathImpulse 死亡冲量
	 */
	virtual void Die(const FVector& DeathImpulse) override;

	/** ======================== end Combat Interface ======================== */

	/**
	 * 死亡后自动重生/重载的等待时间（秒）
	 * 默认 5 秒，可在蓝图 Details 面板中调整
	 */
	UPROPERTY(EditDefaultsOnly)
	float DeathTime = 5.f;

	/** 死亡计时器句柄，用于在 DeathTime 后触发重生逻辑 */
	FTimerHandle DeathTimer;

	/**
	 * 升级特效 Niagara 组件
	 * 升级时在角色位置播放粒子特效（多播 RPC 触发）
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> LevelUpNiagaraComponent;

	/** 眩晕状态同步回调（重写基类，额外处理玩家输入禁用） */
	virtual void OnRep_Stunned() override;

	/** 燃烧状态同步回调（重写基类） */
	virtual void OnRep_Burned() override;

	/** 从存档加载玩家进度（属性、技能、等级、XP 等） */
	void LoadProgress();

private:
	/**
	 * 俯视摄像机组件
	 * 提供游戏的俯视角视角，挂载在 CameraBoom 末端
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> TopDownCameraComponent;

	/**
	 * 弹簧臂组件
	 * 控制摄像机与角色的距离和角度，支持碰撞缩短
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoom;
	
	/**
	 * 初始化 ASC ActorInfo（重写基类）
	 * 玩家角色的 ASC 在 PlayerState 上，需要从 PlayerState 获取
	 */
	virtual void InitAbilityActorInfo() override;

	/**
	 * 多播 RPC：在所有客户端播放升级粒子特效
	 * 升级时由服务端调用，确保所有玩家都能看到升级特效
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLevelUpParticles() const;
};
