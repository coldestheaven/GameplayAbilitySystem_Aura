// Copyright Druid Mechanics


#include "AbilitySystem/AsyncTasks/WaitCooldownChange.h"
#include "AbilitySystemComponent.h"

/**
 * 创建等待冷却时间变化任务（静态工厂函数）
 * 
 * 实现流程：
 * 1. 创建 WaitCooldownChange 对象
 * 2. 设置 ASC 和冷却标签
 * 3. 校验参数有效性，如果无效则结束任务
 * 4. 注册冷却标签变化回调（CooldownTagChanged）
 * 5. 注册 GE 添加回调（OnActiveEffectAdded，用于检测冷却开始）
 * 6. 返回任务对象
 * 
 * @param AbilitySystemComponent 要监听冷却的 ASC
 * @param InCooldownTag 冷却标签
 * @return 创建的 WaitCooldownChange 任务对象
 * 
 * 使用场景：
 * - 技能需要监听冷却时间变化时调用（如 UI 显示冷却进度）
 */
UWaitCooldownChange* UWaitCooldownChange::WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTag& InCooldownTag)
{
	UWaitCooldownChange* WaitCooldownChange = NewObject<UWaitCooldownChange>();
	WaitCooldownChange->ASC = AbilitySystemComponent;
	WaitCooldownChange->CooldownTag = InCooldownTag;
	
	if (!IsValid(AbilitySystemComponent) || !InCooldownTag.IsValid())
	{
		WaitCooldownChange->EndTask();
		return nullptr;
	}

	// 注册冷却标签变化回调（用于检测冷却结束）
	AbilitySystemComponent->RegisterGameplayTagEvent(
		InCooldownTag,
		EGameplayTagEventType::NewOrRemoved).AddUObject(
			WaitCooldownChange,
			&UWaitCooldownChange::CooldownTagChanged);

	// 注册 GE 添加回调（用于检测冷却开始）
	AbilitySystemComponent->OnActiveGameplayEffectAddedDelegateToSelf.AddUObject(WaitCooldownChange, &UWaitCooldownChange::OnActiveEffectAdded);

	return WaitCooldownChange;
}

/**
 * 结束任务
 * 
 * 实现流程：
 * 1. 移除冷却标签变化回调
 * 2. 设置任务为准备销毁
 * 3. 标记为垃圾回收
 * 
 * 使用场景：
 * - 任务完成或取消时调用
 */
void UWaitCooldownChange::EndTask()
{
	if (!IsValid(ASC)) return;
	ASC->RegisterGameplayTagEvent(CooldownTag, EGameplayTagEventType::NewOrRemoved).RemoveAll(this);

	SetReadyToDestroy();
	MarkAsGarbage();
}

/**
 * 冷却标签变化回调
 * 
 * 实现流程：
 * 1. 如果标签数量为 0（冷却结束），广播 CooldownEnd 委托
 * 
 * @param InCooldownTag 冷却标签
 * @param NewCount 当前标签数量
 * 
 * 使用场景：
 * - 冷却 GE 移除时自动调用
 */
void UWaitCooldownChange::CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount)
{
	if (NewCount == 0)
	{
		CooldownEnd.Broadcast(0.f);
	}
}

/**
 * 活跃 GE 添加回调（检测冷却开始）
 * 
 * 实现流程：
 * 1. 获取 GE 的 AssetTags 和 GrantedTags
 * 2. 检查是否包含冷却标签
 * 3. 如果包含，查询所有匹配的冷却 GE 的剩余时间
 * 4. 找到最长的剩余时间（如果有多个冷却 GE）
 * 5. 广播 CooldownStart 委托（传递剩余时间）
 * 
 * @param TargetASC 目标 ASC
 * @param SpecApplied 应用的 GE Spec
 * @param ActiveEffectHandle 活跃 GE 句柄
 * 
 * 使用场景：
 * - 冷却 GE 应用时自动调用
 * 
 * 注意：
 * - 如果有多个冷却 GE，使用最长的剩余时间
 */
void UWaitCooldownChange::OnActiveEffectAdded(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveEffectHandle)
{
	FGameplayTagContainer AssetTags;
	SpecApplied.GetAllAssetTags(AssetTags);

	FGameplayTagContainer GrantedTags;
	SpecApplied.GetAllGrantedTags(GrantedTags);

	// 检查是否包含冷却标签
	if (AssetTags.HasTagExact(CooldownTag) || GrantedTags.HasTagExact(CooldownTag))
	{
		// 查询所有匹配的冷却 GE 的剩余时间
		FGameplayEffectQuery GameplayEffectQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownTag.GetSingleTagContainer());
		TArray<float> TimesRemaining = ASC->GetActiveEffectsTimeRemaining(GameplayEffectQuery);
		if (TimesRemaining.Num() > 0)
		{
			// 找到最长的剩余时间
			float TimeRemaining = TimesRemaining[0];
			for (int32 i = 0; i < TimesRemaining.Num(); i++)
			{
				if (TimesRemaining[i] > TimeRemaining)
				{
					TimeRemaining = TimesRemaining[i];
				}
			}
			
			CooldownStart.Broadcast(TimeRemaining);
		}
	}
}
