// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "AuraPlayerController.generated.h"

class IHighlightInterface;
class UNiagaraSystem;
class UDamageTextComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UAuraInputConfig;
class UAuraAbilitySystemComponent;
class USplineComponent;
class AMagicCircle;

/**
 * 鼠标目标状态枚举
 * 用于区分当前鼠标光标下的目标类型，影响自动寻路和技能释放行为
 */
enum class ETargetingStatus : uint8
{
	TargetingEnemy,     // 鼠标悬停在敌人上（显示攻击光标，点击触发攻击）
	TargetingNonEnemy,  // 鼠标悬停在非敌人可交互对象上
	NotTargeting        // 鼠标悬停在地面或无效目标上（点击触发自动寻路移动）
};

/**
 * Aura 玩家控制器
 *
 * 职责：
 * - 处理玩家输入（移动、技能释放、Shift 键）
 * - 管理鼠标光标追踪（CursorTrace）和目标高亮
 * - 实现点击移动的自动寻路（基于 Spline 路径）
 * - 管理魔法圆圈的显示/隐藏（用于需要指定位置的技能）
 * - 通过 Client RPC 显示浮动伤害数字
 *
 * 输入流程：
 *   鼠标点击 → AbilityInputTagPressed → ASC.AbilityInputTagPressed
 *   持续按住 → AbilityInputTagHeld → ASC.AbilityInputTagHeld（或触发自动寻路）
 *   释放按键 → AbilityInputTagReleased → ASC.AbilityInputTagReleased
 */
UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AAuraPlayerController();

	/** 每帧更新：执行光标追踪、自动寻路移动、魔法圆圈位置更新 */
	virtual void PlayerTick(float DeltaTime) override;

	/**
	 * 客户端 RPC：在目标角色位置显示浮动伤害数字
	 * 由服务端调用，在本地客户端生成 DamageTextComponent
	 * @param DamageAmount    伤害数值
	 * @param TargetCharacter 受伤角色（用于确定显示位置）
	 * @param bBlockedHit     是否为格挡命中（显示不同颜色/样式）
	 * @param bCriticalHit    是否为暴击（显示不同颜色/样式）
	 */
	UFUNCTION(Client, Reliable)
	void ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit);

	/**
	 * 在地面显示魔法圆圈（蓝图可调用）
	 * 生成 MagicCircle Actor 并跟随鼠标位置移动
	 * @param DecalMaterial 圆圈贴花材质（nullptr 使用默认材质）
	 */
	UFUNCTION(BlueprintCallable)
	void ShowMagicCircle(UMaterialInterface* DecalMaterial = nullptr);

	/** 隐藏并销毁魔法圆圈（蓝图可调用） */
	UFUNCTION(BlueprintCallable)
	void HideMagicCircle();

protected:
	virtual void BeginPlay() override;

	/** 设置输入组件：绑定移动、Shift 键和技能输入 */
	virtual void SetupInputComponent() override;

private:
	/**
	 * 增强输入映射上下文
	 * 包含所有 Aura 游戏的输入映射（移动、技能等）
	 */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputMappingContext> AuraContext;

	/** 移动输入动作（WASD 或鼠标点击移动） */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Shift 键输入动作（按住 Shift 时强制移动而不攻击） */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> ShiftAction;

	/** Shift 键按下回调 */
	void ShiftPressed() { bShiftKeyDown = true; };

	/** Shift 键释放回调 */
	void ShiftReleased() { bShiftKeyDown = false; };

	/** Shift 键当前是否按下（影响点击行为：按住 Shift 时只移动不攻击） */
	bool bShiftKeyDown = false;

	/**
	 * 处理移动输入
	 * @param InputActionValue 包含移动方向的输入值（2D 轴）
	 */
	void Move(const FInputActionValue& InputActionValue);

	/**
	 * 每帧执行鼠标光标追踪
	 * 通过射线检测确定鼠标下方的 Actor，并更新高亮状态和 TargetingStatus
	 */
	void CursorTrace();

	/** 上一帧鼠标悬停的 Actor（用于取消高亮） */
	TObjectPtr<AActor> LastActor;

	/** 当前帧鼠标悬停的 Actor */
	TObjectPtr<AActor> ThisActor;

	/** 鼠标光标射线检测结果（包含命中位置、法线等信息） */
	FHitResult CursorHit;

	/**
	 * 高亮指定 Actor（调用 IHighlightInterface::HighlightActor）
	 * @param InActor 要高亮的 Actor
	 */
	static void HighlightActor(AActor* InActor);

	/**
	 * 取消高亮指定 Actor（调用 IHighlightInterface::UnHighlightActor）
	 * @param InActor 要取消高亮的 Actor
	 */
	static void UnHighlightActor(AActor* InActor);

	/**
	 * 技能输入按下回调（由 AuraInputComponent 绑定）
	 * @param InputTag 按下的输入标签
	 */
	void AbilityInputTagPressed(FGameplayTag InputTag);

	/**
	 * 技能输入释放回调
	 * @param InputTag 释放的输入标签
	 */
	void AbilityInputTagReleased(FGameplayTag InputTag);

	/**
	 * 技能输入持续按住回调
	 * @param InputTag 持续按住的输入标签
	 */
	void AbilityInputTagHeld(FGameplayTag InputTag);

	/**
	 * 技能输入配置数据资产
	 * 定义每个 InputAction 对应的 GameplayTag（如 LMB → InputTag.LMB）
	 */
	UPROPERTY(EditDefaultsOnly, Category="Input")
	TObjectPtr<UAuraInputConfig> InputConfig;

	/** 缓存的 ASC 引用（避免每帧重复查找） */
	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;

	/** 懒加载获取 ASC（首次调用时从 PlayerState 获取并缓存） */
	UAuraAbilitySystemComponent* GetASC();

	/** 自动寻路的目标位置（鼠标点击时记录，自动寻路时作为终点） */
	FVector CachedDestination = FVector::ZeroVector;

	/** 鼠标按住时间（用于区分短按和长按） */
	float FollowTime = 0.f;

	/** 短按阈值（秒）：按住时间小于此值视为短按，触发点击移动 */
	float ShortPressThreshold = 0.5f;

	/** 是否正在自动寻路移动 */
	bool bAutoRunning = false;

	/** 当前鼠标目标状态（影响点击行为） */
	ETargetingStatus TargetingStatus = ETargetingStatus::NotTargeting;

	/**
	 * 自动寻路到达判定半径（单位：cm）
	 * 角色与目标点距离小于此值时停止自动寻路
	 */
	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50.f;

	/**
	 * 自动寻路路径样条线
	 * 使用 Navigation Mesh 生成路径后，将路径点存储在此样条线中
	 * 每帧沿样条线方向移动角色
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> Spline;

	/** 点击地面时在目标位置播放的 Niagara 特效（点击反馈） */
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UNiagaraSystem> ClickNiagaraSystem;

	/**
	 * 执行自动寻路移动
	 * 每帧计算样条线上最近点的切线方向，并向该方向移动角色
	 */
	void AutoRun();

	/** 浮动伤害数字 Widget 组件类（在 Details 面板中指定） */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;

	/** 魔法圆圈 Actor 类（在 Details 面板中指定） */
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AMagicCircle> MagicCircleClass;

	/** 当前场景中的魔法圆圈 Actor 实例（ShowMagicCircle 时生成，HideMagicCircle 时销毁） */
	UPROPERTY()
	TObjectPtr<AMagicCircle> MagicCircle;

	/** 每帧更新魔法圆圈位置（跟随鼠标光标在地面的投影位置） */
	void UpdateMagicCircleLocation();
};
