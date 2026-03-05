// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "GameplayTagContainer.h"
#include "ActiveGameplayEffectHandle.h"
#include "WaitCooldownChange.generated.h"

class UAbilitySystemComponent;
struct FGameplayEffectSpec;

/** 冷却时间变化时广播（携带剩余冷却时间，秒） */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCooldownChangeSignature, float, TimeRemaining);

/**
 * 异步任务：等待冷却时间变化
 *
 * 用于 UI 系统监听技能冷却状态（冷却开始/结束）
 * 继承自 UBlueprintAsyncActionBase，可在蓝图中作为异步节点使用
 *
 * 工作原理：
 * - 监听 ASC 上指定 CooldownTag 的 GameplayTag 数量变化
 * - 冷却开始时（Tag 数量从 0 变为 1）：广播 CooldownStart，携带剩余时间
 * - 冷却结束时（Tag 数量从 1 变为 0）：广播 CooldownEnd，携带 0
 * - 同时监听新 GE 被应用事件，用于获取冷却时间的精确剩余时间
 *
 * 使用示例（蓝图中）：
 *   // 创建异步任务并绑定委托
 *   UWaitCooldownChange* Task = UWaitCooldownChange::WaitForCooldownChange(ASC, CooldownTag);
 *   Task->CooldownStart.AddDynamic(this, &UMyWidget::OnCooldownStart);
 *   Task->CooldownEnd.AddDynamic(this, &UMyWidget::OnCooldownEnd);
 *   // 不再需要时调用 EndTask 清理
 *   Task->EndTask();
 */
UCLASS(BlueprintType, meta = (ExposedAsyncProxy = "AsyncTask"))
class AURA_API UWaitCooldownChange : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	/**
	 * 冷却开始时广播
	 * 携带冷却的剩余时间（秒），UI 可用此值驱动冷却进度条动画
	 */
	UPROPERTY(BlueprintAssignable)
	FCooldownChangeSignature CooldownStart;

	/**
	 * 冷却结束时广播
	 * 携带 0（表示冷却已结束），UI 可用此事件重置冷却显示
	 */
	UPROPERTY(BlueprintAssignable)
	FCooldownChangeSignature CooldownEnd;

	/**
	 * 创建并启动冷却等待任务（蓝图可调用，内部使用）
	 * @param AbilitySystemComponent 要监听的 ASC
	 * @param InCooldownTag          要监听的冷却 GameplayTag
	 * @return 创建的异步任务实例
	 */
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	static UWaitCooldownChange* WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTag& InCooldownTag);

	/**
	 * 结束任务并清理（蓝图可调用）
	 * 解绑所有委托，防止内存泄漏
	 * 在 Widget 销毁时调用
	 */
	UFUNCTION(BlueprintCallable)
	void EndTask();

protected:
	/** 监听的 ASC 引用 */
	UPROPERTY()
	TObjectPtr<UAbilitySystemComponent> ASC;

	/** 监听的冷却 GameplayTag */
	FGameplayTag CooldownTag;

	/**
	 * 冷却 Tag 数量变化回调
	 * 绑定到 ASC 的 RegisterGameplayTagEvent
	 * @param InCooldownTag 变化的 Tag
	 * @param NewCount      新的 Tag 数量（0 表示冷却结束，>0 表示冷却中）
	 */
	void CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount);

	/**
	 * 新 GE 被应用时的回调
	 * 用于获取冷却 GE 的精确剩余时间（通过 GE Spec 的 Duration）
	 * @param TargetASC         目标 ASC
	 * @param SpecApplied       被应用的 GE 规格
	 * @param ActiveEffectHandle 激活的 GE 句柄
	 */
	void OnActiveEffectAdded(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveEffectHandle);
};
