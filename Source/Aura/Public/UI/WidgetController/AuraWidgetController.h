// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "UObject/NoExportTypes.h"
#include "AuraWidgetController.generated.h"

/** 玩家单一整数属性变化时广播（等级、属性点、技能点等） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChangedSignature, int32, NewValue);

/** 技能信息变化时广播（技能图标、描述等 UI 数据更新） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityInfoSignature, const FAuraAbilityInfo&, Info);

class UAttributeSet;
class UAbilitySystemComponent;
class AAuraPlayerController;
class AAuraPlayerState;
class UAuraAbilitySystemComponent;
class UAuraAttributeSet;
class UAbilityInfo;

/**
 * Widget 控制器初始化参数结构体
 * 封装创建 WidgetController 所需的四个核心对象
 *
 * 使用示例：
 *   // 在 HUD 中创建 WidgetController 时传入
 *   FWidgetControllerParams Params(PlayerController, PlayerState, ASC, AttributeSet);
 *   OverlayWidgetController->SetWidgetControllerParams(Params);
 */
USTRUCT(BlueprintType)
struct FWidgetControllerParams
{
	GENERATED_BODY()

	FWidgetControllerParams() {}

	/**
	 * 便捷构造函数
	 * @param PC  玩家控制器
	 * @param PS  玩家状态
	 * @param ASC 能力系统组件
	 * @param AS  属性集
	 */
	FWidgetControllerParams(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS)
	: PlayerController(PC), PlayerState(PS), AbilitySystemComponent(ASC), AttributeSet(AS) {}

	/** 玩家控制器（用于获取输入和本地玩家信息） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;

	/** 玩家状态（用于获取等级、XP 等进度数据） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerState> PlayerState = nullptr;

	/** 能力系统组件（用于绑定属性变化回调和技能状态监听） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;

	/** 属性集（用于获取具体属性值） */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;
};

/**
 * Aura Widget 控制器基类
 *
 * 职责：
 * - 持有四个核心引用（PC、PS、ASC、AS）
 * - 提供懒加载的 Aura 专用类型转换（GetAuraPC、GetAuraPS 等）
 * - 定义 BroadcastInitialValues 和 BindCallbacksToDependencies 接口
 * - 广播技能信息（BroadcastAbilityInfo）
 *
 * 子类实现模式：
 *   1. 重写 BindCallbacksToDependencies：绑定 ASC/PS 的委托回调
 *   2. 重写 BroadcastInitialValues：广播当前属性值给 Widget 初始化显示
 *
 * 注意：WidgetController 是 UObject，不参与网络同步，只在本地客户端存在
 */
UCLASS()
class AURA_API UAuraWidgetController : public UObject
{
	GENERATED_BODY()
public:
	/**
	 * 设置 Widget 控制器参数（初始化四个核心引用）
	 * 在 HUD 创建 WidgetController 后立即调用
	 * @param WCParams 包含 PC、PS、ASC、AS 的参数结构体
	 */
	UFUNCTION(BlueprintCallable)
	void SetWidgetControllerParams(const FWidgetControllerParams& WCParams);

	/**
	 * 广播初始属性值给 Widget（蓝图可调用）
	 * 在 Widget 初始化时调用，确保 UI 显示正确的初始数据
	 * 子类必须重写此函数以广播各自关心的属性
	 */
	UFUNCTION(BlueprintCallable)
	virtual void BroadcastInitialValues();

	/**
	 * 绑定回调到依赖项（ASC、PS 的委托）
	 * 在 WidgetController 初始化后调用，建立数据变化的监听链
	 * 子类必须重写此函数以绑定各自需要监听的委托
	 */
	virtual void BindCallbacksToDependencies();

	/**
	 * 技能信息广播委托（蓝图可绑定）
	 * 当技能信息更新时广播，Widget 可绑定此委托更新技能图标和描述
	 */
	UPROPERTY(BlueprintAssignable, Category="GAS|Messages")
	FAbilityInfoSignature AbilityInfoDelegate;

	/**
	 * 广播所有技能的信息
	 * 遍历 ASC 中所有已赋予的技能，通过 AbilityInfoDelegate 广播每个技能的 UI 数据
	 * 在 BroadcastInitialValues 中调用，确保技能栏初始显示正确
	 */
	void BroadcastAbilityInfo();

protected:
	/**
	 * 技能信息数据资产
	 * 包含所有技能的图标、名称、描述等 UI 显示数据
	 * 在 Details 面板中指定
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Data")
	TObjectPtr<UAbilityInfo> AbilityInfo;

	/** 玩家控制器（基类类型，通过 GetAuraPC 转换为 Aura 专用类型） */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<APlayerController> PlayerController;

	/** 玩家状态（基类类型） */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<APlayerState> PlayerState;

	/** 能力系统组件（基类类型） */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	/** 属性集（基类类型） */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UAttributeSet> AttributeSet;

	/** Aura 专用玩家控制器（懒加载缓存，通过 GetAuraPC 获取） */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<AAuraPlayerController> AuraPlayerController;

	/** Aura 专用玩家状态（懒加载缓存） */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<AAuraPlayerState> AuraPlayerState;

	/** Aura 专用能力系统组件（懒加载缓存） */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;

	/** Aura 专用属性集（懒加载缓存） */
	UPROPERTY(BlueprintReadOnly, Category="WidgetController")
	TObjectPtr<UAuraAttributeSet> AuraAttributeSet;

	/** 懒加载获取 Aura 玩家控制器（Cast<AAuraPlayerController>(PlayerController)） */
	AAuraPlayerController* GetAuraPC();

	/** 懒加载获取 Aura 玩家状态（Cast<AAuraPlayerState>(PlayerState)） */
	AAuraPlayerState* GetAuraPS();

	/** 懒加载获取 Aura 能力系统组件（Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)） */
	UAuraAbilitySystemComponent* GetAuraASC();

	/** 懒加载获取 Aura 属性集（Cast<UAuraAttributeSet>(AttributeSet)） */
	UAuraAttributeSet* GetAuraAS();
};
