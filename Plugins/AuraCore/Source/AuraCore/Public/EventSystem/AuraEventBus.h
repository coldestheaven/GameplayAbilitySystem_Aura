// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "EventSystem/AuraEventTypes.h"
#include "AuraEventBus.generated.h"

/**
 * Aura 事件总线（GameInstance 子系统）
 *
 * 基于发布-订阅模式的中央事件分发系统
 * 作为 GameInstance 子系统，在整个游戏生命周期内存在
 *
 * 核心功能：
 * - 解耦组件间通信（发布者和订阅者不需要直接引用对方）
 * - 支持 C++ 模板方法（类型安全）和蓝图方法
 * - 自动生命周期管理（GameInstance 销毁时自动清理）
 *
 * 支持的事件类型（来自 AuraEventTypes.h）：
 * - FAttributeChangedEvent：属性值变化
 * - FAbilityActivatedEvent：技能激活
 * - FAuraDamageEvent：伤害
 * - FXPGainedEvent：获得经验值
 * - FLevelUpEvent：升级
 * - FCharacterDeathEvent：角色死亡
 * - FAbilityEquippedEvent：技能装备
 *
 * 使用示例（C++）：
 *   // 获取事件总线
 *   UAuraEventBus* Bus = UAuraEventBus::GetEventBus(this);
 *   // 订阅伤害事件
 *   int32 Handle = Bus->Subscribe<FAuraDamageEvent>(
 *       [this](const FAuraDamageEvent& Event) { OnDamageReceived(Event); }
 *   );
 *   // 发布伤害事件
 *   FAuraDamageEvent DamageEvent;
 *   DamageEvent.Damage = 100.f;
 *   Bus->Publish(DamageEvent);
 *   // 取消订阅
 *   Bus->Unsubscribe(FAuraDamageEvent::StaticStruct()->GetFName(), Handle);
 */
UCLASS()
class AURACORE_API UAuraEventBus : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	/** 初始化子系统（重置统计计数器） */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** 反初始化子系统（清理所有事件处理器） */
	virtual void Deinitialize() override;

	/**
	 * 订阅事件（C++ 模板方法，类型安全）
	 * @tparam TEvent 事件类型（必须继承自 FAuraEvent）
	 * @param Handler 事件处理函数（Lambda 或函数指针）
	 * @return 订阅句柄（用于后续取消订阅）
	 */
	template<typename TEvent>
	int32 Subscribe(TFunction<void(const TEvent&)> Handler)
	{
		static_assert(TIsDerivedFrom<TEvent, FAuraEvent>::Value, "TEvent must derive from FAuraEvent");
		
		const FName EventName = TEvent::StaticStruct()->GetFName();
		
		if (!EventHandlers.Contains(EventName))
		{
			EventHandlers.Add(EventName, TArray<TFunction<void(const void*)>>());
		}
		
		// 创建类型擦除包装器（将类型安全的 Handler 包装为 void* 版本）
		TFunction<void(const void*)> Wrapper = [Handler](const void* Data) {
			Handler(*static_cast<const TEvent*>(Data));
		};
		
		EventHandlers[EventName].Add(Wrapper);
		
		UE_LOG(LogTemp, Log, TEXT("[EventBus] Subscribed to event: %s"), *EventName.ToString());
		
		// 返回数组索引作为句柄
		return EventHandlers[EventName].Num() - 1;
	}
	
	/**
	 * 发布事件（C++ 模板方法，类型安全）
	 * 遍历所有订阅此事件类型的处理器并依次调用
	 * @tparam TEvent 事件类型（必须继承自 FAuraEvent）
	 * @param Event 要发布的事件数据
	 */
	template<typename TEvent>
	void Publish(const TEvent& Event)
	{
		static_assert(TIsDerivedFrom<TEvent, FAuraEvent>::Value, "TEvent must derive from FAuraEvent");
		
		const FName EventName = TEvent::StaticStruct()->GetFName();
		
		if (EventHandlers.Contains(EventName))
		{
			UE_LOG(LogTemp, Verbose, TEXT("[EventBus] Publishing event: %s to %d handlers"), 
				*EventName.ToString(), EventHandlers[EventName].Num());
			
			for (auto& Handler : EventHandlers[EventName])
			{
				Handler(&Event);
			}
		}
		else
		{
			UE_LOG(LogTemp, Verbose, TEXT("[EventBus] No handlers for event: %s"), *EventName.ToString());
		}
	}

	/**
	 * 取消订阅（蓝图可调用）
	 * @param EventName 事件名称（使用 StaticStruct()->GetFName() 获取）
	 * @param Handle    Subscribe 返回的订阅句柄
	 */
	UFUNCTION(BlueprintCallable, Category = "EventBus")
	void Unsubscribe(FName EventName, int32 Handle);

	/**
	 * 清除所有订阅（蓝图可调用）
	 * 移除所有事件类型的所有处理器
	 * 通常在关卡切换时调用，防止旧关卡的回调在新关卡中触发
	 */
	UFUNCTION(BlueprintCallable, Category = "EventBus")
	void ClearAllSubscriptions();

	/**
	 * 获取指定事件的订阅数量（蓝图可调用，用于调试）
	 * @param EventName 事件名称
	 * @return 当前订阅此事件的处理器数量
	 */
	UFUNCTION(BlueprintCallable, Category = "EventBus")
	int32 GetSubscriptionCount(FName EventName) const;

	/** 蓝图：发布属性变化事件 */
	UFUNCTION(BlueprintCallable, Category = "EventBus|Events")
	void PublishAttributeChanged(const FAttributeChangedEvent& Event);

	/** 蓝图：发布技能激活事件 */
	UFUNCTION(BlueprintCallable, Category = "EventBus|Events")
	void PublishAbilityActivated(const FAbilityActivatedEvent& Event);

	/** 蓝图：发布伤害事件 */
	UFUNCTION(BlueprintCallable, Category = "EventBus|Events")
	void PublishDamage(const FAuraDamageEvent& Event);

	/** 蓝图：发布经验值获得事件 */
	UFUNCTION(BlueprintCallable, Category = "EventBus|Events")
	void PublishXPGained(const FXPGainedEvent& Event);

	/** 蓝图：发布等级提升事件 */
	UFUNCTION(BlueprintCallable, Category = "EventBus|Events")
	void PublishLevelUp(const FLevelUpEvent& Event);

	/** 蓝图：发布角色死亡事件 */
	UFUNCTION(BlueprintCallable, Category = "EventBus|Events")
	void PublishCharacterDeath(const FCharacterDeathEvent& Event);

	/**
	 * 获取事件总线实例（静态便捷方法，蓝图纯函数）
	 * @param WorldContextObject 世界上下文对象（传 self）
	 * @return UAuraEventBus 实例，如果 GameInstance 不存在则返回 nullptr
	 */
	UFUNCTION(BlueprintPure, Category = "EventBus", meta = (WorldContext = "WorldContextObject"))
	static UAuraEventBus* GetEventBus(const UObject* WorldContextObject);

private:
	/**
	 * 事件处理器映射表
	 * Key: 事件结构体名称（FName）
	 * Value: 该事件类型的所有处理器数组（类型擦除为 void* 版本）
	 */
	TMap<FName, TArray<TFunction<void(const void*)>>> EventHandlers;

	/** 已发布的事件总数（用于调试统计） */
	UPROPERTY()
	int32 TotalEventsPublished;

	/** 当前活跃的订阅总数（用于调试统计） */
	UPROPERTY()
	int32 TotalSubscriptions;
};
