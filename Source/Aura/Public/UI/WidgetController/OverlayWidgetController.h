// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "OverlayWidgetController.generated.h"

struct FAuraAbilityInfo;

/**
 * UI 消息 Widget 数据表行结构体
 * 用于 DataTable，通过 GameplayTag 查找对应的消息 Widget 配置
 * 当玩家拾取物品或触发特定事件时，显示对应的消息 Widget
 *
 * 使用示例（DataTable 配置）：
 *   行名：Abilities.Fire.FireBolt
 *   MessageTag: Abilities.Fire.FireBolt
 *   Message: "获得了火焰箭技能！"
 *   MessageWidget: WBP_AbilityPickupMessage
 *   Image: T_FireBolt_Icon
 */
USTRUCT(BlueprintType)
struct FUIWidgetRow : public FTableRowBase
{
	GENERATED_BODY()

	/** 触发此消息的 GameplayTag（GE 的 AssetTag，用于在 DataTable 中查找对应行） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag MessageTag = FGameplayTag();

	/** 消息文本内容（显示在消息 Widget 上） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Message = FText();

	/** 消息 Widget 类（用于创建并显示消息 UI） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<class UAuraUserWidget> MessageWidget;

	/** 消息图标（显示在消息 Widget 上，如技能图标或物品图标） */
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTexture2D* Image = nullptr;
};

class UAuraUserWidget;
class UAbilityInfo;
class UAuraAbilitySystemComponent;

/** 浮点属性变化时广播（生命值、法力值等） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAttributeChangedSignature, float, NewValue);

/** 玩家等级变化时广播（携带新等级和是否为升级事件） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLevelChangedSignature, int32, NewLevel, bool, bLevelUp);

/** 消息 Widget 行数据广播（拾取物品或触发事件时显示消息） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMessageWidgetRowSignature, FUIWidgetRow, Row);

/**
 * 主 Overlay HUD 的 Widget 控制器
 *
 * 职责：
 * - 监听 ASC 属性变化（生命值、法力值）并广播给 Widget
 * - 监听 PlayerState 等级/XP 变化并广播给 Widget
 * - 监听 GE 应用事件，通过 DataTable 查找并广播消息 Widget 数据
 * - 监听技能装备事件，更新技能栏显示
 *
 * 广播的委托：
 * - OnHealthChanged / OnMaxHealthChanged：生命值变化（血条更新）
 * - OnManaChanged / OnMaxManaChanged：法力值变化（法力条更新）
 * - MessageWidgetRowDelegate：消息提示（拾取物品等）
 * - OnXPPercentChangedDelegate：XP 百分比变化（经验条更新）
 * - OnPlayerLevelChangedDelegate：等级变化（等级显示更新）
 */
UCLASS(BlueprintType, Blueprintable)
class AURA_API UOverlayWidgetController : public UAuraWidgetController
{
	GENERATED_BODY()
public:
	/**
	 * 广播初始属性值（重写基类）
	 * 广播当前生命值、最大生命值、法力值、最大法力值给 Widget 初始化显示
	 */
	virtual void BroadcastInitialValues() override;

	/**
	 * 绑定回调到依赖项（重写基类）
	 * 绑定 ASC 属性变化、GE 应用、技能装备等事件的回调
	 */
	virtual void BindCallbacksToDependencies() override;

	/** 当前生命值变化时广播（血条 Widget 绑定此委托） */
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnHealthChanged;

	/** 最大生命值变化时广播（血条 Widget 绑定此委托以更新上限） */
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnMaxHealthChanged;

	/** 当前法力值变化时广播（法力条 Widget 绑定此委托） */
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnManaChanged;

	/** 最大法力值变化时广播（法力条 Widget 绑定此委托以更新上限） */
	UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
	FOnAttributeChangedSignature OnMaxManaChanged;

	/**
	 * 消息 Widget 行数据广播委托
	 * 当 GE 被应用且其 AssetTag 在 MessageWidgetDataTable 中有对应行时广播
	 * Widget 可绑定此委托来显示拾取提示、技能获得提示等
	 */
	UPROPERTY(BlueprintAssignable, Category="GAS|Messages")
	FMessageWidgetRowSignature MessageWidgetRowDelegate;

	/**
	 * XP 百分比变化时广播（经验条 Widget 绑定此委托）
	 * 值为 0.0~1.0，表示当前等级的 XP 进度百分比
	 */
	UPROPERTY(BlueprintAssignable, Category="GAS|XP")
	FOnAttributeChangedSignature OnXPPercentChangedDelegate;

	/**
	 * 玩家等级变化时广播（等级显示 Widget 绑定此委托）
	 * bLevelUp=true 时触发升级特效和音效
	 */
	UPROPERTY(BlueprintAssignable, Category="GAS|Level")
	FOnLevelChangedSignature OnPlayerLevelChangedDelegate;

protected:
	/**
	 * 消息 Widget 数据表
	 * 每行对应一个 GameplayTag，定义触发该 Tag 时显示的消息内容和 Widget 类型
	 * 在 Details 面板中指定
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Data")
	TObjectPtr<UDataTable> MessageWidgetDataTable;

	/**
	 * 根据 GameplayTag 从 DataTable 中查找对应的行数据
	 * @tparam T     DataTable 行结构体类型
	 * @param DataTable 要查找的数据表
	 * @param Tag       要查找的 GameplayTag（使用 Tag 名称作为行名）
	 * @return 对应行的指针，未找到则返回 nullptr
	 */
	template<typename T>
	T* GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag);

	/**
	 * XP 变化回调（绑定到 PlayerState 的 OnXPChangedDelegate）
	 * 计算当前等级的 XP 百分比并广播 OnXPPercentChangedDelegate
	 * @param NewXP 新的 XP 总量
	 */
	void OnXPChanged(int32 NewXP);

	/**
	 * 技能装备回调（绑定到 ASC 的 AbilityEquipped 委托）
	 * 当技能被装备到槽位时，广播技能信息更新技能栏 Widget
	 * @param AbilityTag   已装备的技能标签
	 * @param Status       技能当前状态
	 * @param Slot         新槽位
	 * @param PreviousSlot 原槽位
	 */
	void OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& Status, const FGameplayTag& Slot, const FGameplayTag& PreviousSlot) const;
};

/**
 * 模板函数实现：根据 GameplayTag 从 DataTable 查找行
 * 使用 Tag 的名称（FName）作为 DataTable 的行名进行查找
 */
template <typename T>
T* UOverlayWidgetController::GetDataTableRowByTag(UDataTable* DataTable, const FGameplayTag& Tag)
{
	return DataTable->FindRow<T>(Tag.GetTagName(), TEXT(""));
}
