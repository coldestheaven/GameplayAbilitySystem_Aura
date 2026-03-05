// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraCharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include "Interaction/HighlightInterface.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "AuraEnemy.generated.h"

class UWidgetComponent;
class UBehaviorTree;
class AAuraAIController;
class UEnemyHealthBarWidgetController;

/**
 * 敌人角色类
 *
 * 职责：
 * - 继承 AAuraCharacterBase，额外实现 IEnemyInterface（敌人专属接口）和 IHighlightInterface（高亮接口）
 * - 管理 AI 行为树（BehaviorTree）和 AI 控制器（AuraAIController）
 * - 管理头顶血条 Widget（HealthBar + HealthBarWidgetController）
 * - 处理高亮描边效果（鼠标悬停时显示）
 * - 死亡后触发战利品掉落（蓝图事件 SpawnLoot）
 *
 * 网络说明：
 * - ASC 和 AttributeSet 直接存储在敌人自身上（不跨关卡持久化）
 * - 在 BeginPlay 中初始化 ASC ActorInfo（服务端）
 */
UCLASS()
class AURA_API AAuraEnemy : public AAuraCharacterBase, public IEnemyInterface, public IHighlightInterface
{
	GENERATED_BODY()
public:
	AAuraEnemy();

	/** 服务端：被 AI 控制器接管时，启动行为树 */
	virtual void PossessedBy(AController* NewController) override;

	/** ======================== Highlight Interface 实现 ======================== */

	/** 开启角色高亮描边（鼠标悬停时调用，通过自定义深度实现轮廓效果） */
	virtual void HighlightActor_Implementation() override;

	/** 关闭角色高亮描边 */
	virtual void UnHighlightActor_Implementation() override;

	/**
	 * 设置移动目标位置（点击敌人时，角色移动到敌人附近而非敌人正中心）
	 * @param OutDestination 输出参数，返回推荐的移动目标位置
	 */
	virtual void SetMoveToLocation_Implementation(FVector& OutDestination) override;

	/** ======================== end Highlight Interface ======================== */

	/** ======================== Combat Interface 实现 ======================== */

	/** 返回敌人当前等级 */
	virtual int32 GetPlayerLevel_Implementation() override;

	/**
	 * 敌人死亡处理（重写基类）
	 * 额外逻辑：设置 LifeSpan 定时销毁 Actor，触发战利品掉落
	 * @param DeathImpulse 死亡冲量
	 */
	virtual void Die(const FVector& DeathImpulse) override;

	/**
	 * 设置当前战斗目标（AI 追击的目标 Actor）
	 * @param InCombatTarget 目标 Actor（通常为玩家）
	 */
	virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;

	/** 返回当前战斗目标 Actor */
	virtual AActor* GetCombatTarget_Implementation() const override;

	/** ======================== end Combat Interface ======================== */

	/**
	 * 当前战斗目标（AI 正在追击/攻击的 Actor）
	 * BlueprintReadWrite 允许行为树蓝图节点读写此值
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	TObjectPtr<AActor> CombatTarget;

	/** 生命值变化时广播（供血条 Widget 绑定） */
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnHealthChanged;

	/** 最大生命值变化时广播（供血条 Widget 绑定） */
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnMaxHealthChanged;
	
	/**
	 * 受击标签变化回调
	 * 当 ASC 上的受击 GameplayTag 数量变化时调用，控制受击动画的播放/停止
	 * @param CallbackTag 触发回调的标签
	 * @param NewCount 当前标签数量（>0 表示正在受击）
	 */
	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	/** 是否正在播放受击动画（由 HitReactTagChanged 维护） */
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bHitReacting = false;

	/**
	 * 死亡后 Actor 的存活时间（秒）
	 * 死亡后等待此时间再销毁 Actor，给溶解动画留出播放时间
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float LifeSpan = 5.f;

	/**
	 * 设置敌人等级（由 AuraEnemySpawnPoint 在生成时调用）
	 * @param InLevel 要设置的等级
	 */
	void SetLevel(int32 InLevel) { Level = InLevel; }

protected:
	virtual void BeginPlay() override;

	/** 初始化 ASC ActorInfo（敌人的 ASC 在自身上，直接初始化） */
	virtual void InitAbilityActorInfo() override;

	/** 根据职业类型和等级应用默认属性 GE（从 CharacterClassInfo 数据资产读取） */
	virtual void InitializeDefaultAttributes() const override;

	/**
	 * 眩晕标签变化回调（重写基类）
	 * 额外逻辑：控制 AI 行为树的暂停/恢复
	 * @param CallbackTag 触发回调的标签
	 * @param NewCount 当前标签数量
	 */
	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount) override;

	/**
	 * 敌人等级
	 * 影响属性数值（通过 ScalableFloat 曲线）和 XP 奖励
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	int32 Level = 1;

	/**
	 * 头顶血条 Widget 组件
	 * 显示敌人当前生命值百分比，玩家靠近时可见
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> HealthBar;

	/** 血条 Widget 的控制器（负责绑定 ASC 属性变化并广播给 Widget） */
	UPROPERTY()
	TObjectPtr<UEnemyHealthBarWidgetController> HealthBarWidgetController;

	/** 血条 Widget 控制器的类（在蓝图 Details 面板中指定） */
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UEnemyHealthBarWidgetController> HealthBarWidgetControllerClass;

	/** 初始化头顶血条 Widget（创建控制器、绑定委托、广播初始值） */
	void InitializeHealthBarWidget();

	/** 敌人名称（显示在血条上方，在 Details 面板中配置） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Info")
	FText EnemyName;

	/**
	 * AI 行为树资产
	 * 定义敌人的 AI 决策逻辑（巡逻、追击、攻击等）
	 */
	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	/** AI 控制器引用（在 PossessedBy 中赋值，用于控制行为树） */
	UPROPERTY()
	TObjectPtr<AAuraAIController> AuraAIController;

	/**
	 * 蓝图事件：生成战利品
	 * 敌人死亡时调用，在蓝图中实现具体的掉落逻辑（随机物品、金币等）
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void SpawnLoot();
};
