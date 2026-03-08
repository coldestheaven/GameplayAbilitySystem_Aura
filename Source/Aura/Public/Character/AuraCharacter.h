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
 *
 * 初始化流程：
 *   服务端：PossessedBy → InitAbilityActorInfo → LoadProgress → LoadWorldState
 *   客户端：OnRep_PlayerState → InitAbilityActorInfo
 *
 * 使用示例：
 *   // 玩家角色在游戏开始时自动调用 PossessedBy
 *   // 服务端会初始化 ASC 并加载存档（如果有）
 *   // 客户端会在 PlayerState 同步后初始化 ASC
 */
UCLASS()
class AURA_API AAuraCharacter : public AAuraCharacterBase, public IPlayerInterface
{
	GENERATED_BODY()
public:
	AAuraCharacter();

	/**
	 * 服务端：角色被控制器接管时调用
	 * 初始化 ASC ActorInfo、加载存档进度、加载世界状态
	 * @param NewController 接管此角色的控制器（玩家控制器）
	 */
	virtual void PossessedBy(AController* NewController) override;

	/**
	 * 客户端：PlayerState 同步完成后调用
	 * 初始化客户端侧的 ASC ActorInfo，确保技能和属性在客户端正常工作
	 * 注意：此函数在客户端 PlayerState 复制完成后自动调用
	 */
	virtual void OnRep_PlayerState() override;

	/** ======================== Player Interface 实现 ======================== */

	/**
	 * 增加经验值
	 * 将 XP 累加到 PlayerState，触发升级检测（如果达到升级阈值）
	 * @param InXP 要增加的 XP 数量（必须 >= 0）
	 */
	virtual void AddToXP_Implementation(int32 InXP) override;

	/**
	 * 触发升级逻辑
	 * 播放升级粒子特效（多播 RPC），属性点和技能点由 PlayerState 自动分配
	 * 注意：此函数由 PlayerState 的 OnRep_Level 回调调用
	 */
	virtual void LevelUp_Implementation() override;

	/**
	 * 返回当前 XP 总量
	 * @return 玩家当前累计的 XP 值（从 PlayerState 获取）
	 */
	virtual int32 GetXP_Implementation() const override;

	/**
	 * 根据 XP 数量反查对应的等级
	 * 通过 LevelUpInfo 数据资产查找达到指定 XP 所需的最低等级
	 * @param InXP 要查询的 XP 值（必须 >= 0）
	 * @return 对应的角色等级（1 级为最低等级）
	 */
	virtual int32 FindLevelForXP_Implementation(int32 InXP) const override;

	/**
	 * 获取指定等级升级时奖励的属性点数量
	 * 从 LevelUpInfo 数据资产中读取该等级的属性点奖励
	 * @param Level 目标等级（必须 >= 1）
	 * @return 该等级奖励的属性点数量（如果等级无效则返回 0）
	 */
	virtual int32 GetAttributePointsReward_Implementation(int32 Level) const override;

	/**
	 * 获取指定等级升级时奖励的技能点数量
	 * 从 LevelUpInfo 数据资产中读取该等级的技能点奖励
	 * @param Level 目标等级（必须 >= 1）
	 * @return 该等级奖励的技能点数量（如果等级无效则返回 0）
	 */
	virtual int32 GetSpellPointsReward_Implementation(int32 Level) const override;

	/**
	 * 增加玩家等级
	 * 累加等级到 PlayerState，并更新所有技能的解锁状态（调用 ASC::UpdateAbilityStatuses）
	 * @param InPlayerLevel 要增加的等级数（通常为 1，表示升一级）
	 */
	virtual void AddToPlayerLevel_Implementation(int32 InPlayerLevel) override;

	/**
	 * 增加属性点
	 * 累加属性点到 PlayerState，供玩家在属性菜单中分配
	 * @param InAttributePoints 要增加的属性点数（必须 >= 0）
	 */
	virtual void AddToAttributePoints_Implementation(int32 InAttributePoints) override;

	/**
	 * 增加技能点
	 * 累加技能点到 PlayerState，供玩家在技能菜单中解锁/升级技能
	 * @param InSpellPoints 要增加的技能点数（必须 >= 0）
	 */
	virtual void AddToSpellPoints_Implementation(int32 InSpellPoints) override;

	/**
	 * 返回当前可用属性点数量
	 * @return 玩家当前可分配的属性点数量（从 PlayerState 获取）
	 */
	virtual int32 GetAttributePoints_Implementation() const override;

	/**
	 * 返回当前可用技能点数量
	 * @return 玩家当前可用的技能点数量（从 PlayerState 获取）
	 */
	virtual int32 GetSpellPoints_Implementation() const override;

	/**
	 * 在地面显示魔法圆圈（用于需要指定目标位置的技能）
	 * 通过 PlayerController 显示魔法圆圈，并隐藏鼠标光标（便于精确选择位置）
	 * @param DecalMaterial 圆圈使用的贴花材质（nullptr 表示使用 PlayerController 的默认材质）
	 */
	virtual void ShowMagicCircle_Implementation(UMaterialInterface* DecalMaterial) override;

	/**
	 * 隐藏魔法圆圈
	 * 通过 PlayerController 隐藏魔法圆圈，并恢复鼠标光标显示
	 */
	virtual void HideMagicCircle_Implementation() override;

	/**
	 * 保存游戏进度到存档
	 * 保存玩家等级、XP、属性点、技能点、主属性值和技能状态到 SaveGame
	 * 注意：只有服务端会实际写入存档文件，客户端调用无效
	 * @param CheckpointTag 触发保存的检查点标签，用于记录玩家重生位置
	 */
	virtual void SaveProgress_Implementation(const FName& CheckpointTag) override;

	/** ======================== end Player Interface ======================== */

	/** ======================== Combat Interface 重写 ======================== */

	/**
	 * 返回玩家当前等级（从 PlayerState 获取）
	 * @return 玩家当前等级（1 级为最低等级）
	 */
	virtual int32 GetPlayerLevel_Implementation() override;

	/**
	 * 玩家死亡处理（重写基类）
	 * 额外逻辑：启动死亡计时器（DeathTime 秒），倒计时结束后调用 GameMode::PlayerDied 重新加载关卡
	 * 同时将摄像机从角色分离，保持世界位置（便于观察死亡动画）
	 * @param DeathImpulse 死亡冲量（用于布娃娃物理击飞效果）
	 */
	virtual void Die(const FVector& DeathImpulse) override;

	/** ======================== end Combat Interface ======================== */

	/**
	 * 死亡后自动重生/重载的等待时间（秒）
	 * 玩家死亡后，等待此时间后自动重新加载关卡
	 * 默认 5 秒，可在蓝图 Details 面板中调整
	 */
	UPROPERTY(EditDefaultsOnly)
	float DeathTime = 5.f;

	/**
	 * 死亡计时器句柄
	 * 用于在 DeathTime 后触发重生逻辑（调用 GameMode::PlayerDied）
	 * 在 Die 函数中设置，计时器到期后自动触发
	 */
	FTimerHandle DeathTimer;

	/**
	 * 升级特效 Niagara 组件
	 * 升级时在角色位置播放粒子特效（通过 MulticastLevelUpParticles 多播 RPC 触发）
	 * 特效会面向摄像机方向播放，确保视觉效果最佳
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> LevelUpNiagaraComponent;

	/**
	 * 眩晕状态同步回调（重写基类）
	 * 额外处理：当进入眩晕状态时，添加输入阻止标签（阻止光标追踪、输入响应等）
	 * 当退出眩晕状态时，移除输入阻止标签
	 */
	virtual void OnRep_Stunned() override;

	/**
	 * 燃烧状态同步回调（重写基类）
	 * 根据 bIsBurned 状态激活或停用燃烧 Debuff 特效组件
	 */
	virtual void OnRep_Burned() override;

	/**
	 * 从存档加载玩家进度（属性、技能、等级、XP 等）
	 * 在 PossessedBy 中调用，根据存档数据恢复玩家状态
	 * 如果是首次加载（bFirstTimeLoadIn=true），则初始化默认属性和技能
	 * 否则从存档数据恢复属性值、技能状态和进度数据
	 */
	void LoadProgress();

private:
	/**
	 * 俯视摄像机组件
	 * 提供游戏的俯视角视角，挂载在 CameraBoom 末端
	 * 使用绝对旋转（bUsePawnControlRotation=false），不随角色旋转
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> TopDownCameraComponent;

	/**
	 * 弹簧臂组件
	 * 控制摄像机与角色的距离和角度，支持碰撞缩短
	 * 使用绝对旋转（SetUsingAbsoluteRotation=true），禁用碰撞测试（bDoCollisionTest=false）
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoom;
	
	/**
	 * 初始化 ASC ActorInfo（重写基类）
	 * 玩家角色的 ASC 存储在 PlayerState 上（跨关卡持久化），需要从 PlayerState 获取
	 * 流程：
	 *   1. 从 PlayerState 获取 ASC 和 AttributeSet
	 *   2. 初始化 ASC 的 ActorInfo（Avatar=this, Owner=PlayerState）
	 *   3. 绑定眩晕标签变化回调
	 *   4. 初始化 HUD Overlay（绑定 WidgetController）
	 *   5. 广播 ASC 注册完成委托
	 */
	virtual void InitAbilityActorInfo() override;

	/**
	 * 多播 RPC：在所有客户端播放升级粒子特效
	 * 升级时由服务端调用，确保所有玩家都能看到升级特效
	 * 特效会面向摄像机方向播放，确保视觉效果最佳
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLevelUpParticles() const;
};
