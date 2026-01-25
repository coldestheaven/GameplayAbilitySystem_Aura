// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "EventSystem/AuraEventTypes.h"
#include "AuraEventBus.generated.h"

/**
 * 事件总线 - 中央事件分发系统
 * 
 * 功能：
 * - 解耦组件间通信
 * - 集中管理事件订阅和发布
 * - 支持C++和蓝图
 * - 自动生命周期管理
 */
UCLASS()
class AURACORE_API UAuraEventBus : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	// Subsystem 生命周期
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * 订阅事件（C++模板方法）
	 * @param Handler 事件处理函数
	 * @return 订阅句柄，用于取消订阅
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
		
		// 创建包装器
		TFunction<void(const void*)> Wrapper = [Handler](const void* Data) {
			Handler(*static_cast<const TEvent*>(Data));
		};
		
		EventHandlers[EventName].Add(Wrapper);
		
		UE_LOG(LogTemp, Log, TEXT("[EventBus] Subscribed to event: %s"), *EventName.ToString());
		
		// 返回索引作为句柄
		return EventHandlers[EventName].Num() - 1;
	}
	
	/**
	 * 发布事件（C++模板方法）
	 * @param Event 事件数据
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
	 * 取消订阅（C++）
	 * @param EventName 事件名称
	 * @param Handle 订阅句柄
	 */
	UFUNCTION(BlueprintCallable, Category = "EventBus")
	void Unsubscribe(FName EventName, int32 Handle);

	/**
	 * 清除所有订阅
	 */
	UFUNCTION(BlueprintCallable, Category = "EventBus")
	void ClearAllSubscriptions();

	/**
	 * 获取事件订阅数量
	 */
	UFUNCTION(BlueprintCallable, Category = "EventBus")
	int32 GetSubscriptionCount(FName EventName) const;

	/**
	 * 蓝图：发布属性变化事件
	 */
	UFUNCTION(BlueprintCallable, Category = "EventBus|Events")
	void PublishAttributeChanged(const FAttributeChangedEvent& Event);

	/**
	 * 蓝图：发布技能激活事件
	 */
	UFUNCTION(BlueprintCallable, Category = "EventBus|Events")
	void PublishAbilityActivated(const FAbilityActivatedEvent& Event);

	/**
	 * 蓝图：发布伤害事件
	 */
	UFUNCTION(BlueprintCallable, Category = "EventBus|Events")
	void PublishDamage(const FAuraDamageEvent& Event);

	/**
	 * 蓝图：发布经验值获得事件
	 */
	UFUNCTION(BlueprintCallable, Category = "EventBus|Events")
	void PublishXPGained(const FXPGainedEvent& Event);

	/**
	 * 蓝图：发布等级提升事件
	 */
	UFUNCTION(BlueprintCallable, Category = "EventBus|Events")
	void PublishLevelUp(const FLevelUpEvent& Event);

	/**
	 * 蓝图：发布角色死亡事件
	 */
	UFUNCTION(BlueprintCallable, Category = "EventBus|Events")
	void PublishCharacterDeath(const FCharacterDeathEvent& Event);

	/**
	 * 获取事件总线实例（便捷方法）
	 */
	UFUNCTION(BlueprintPure, Category = "EventBus", meta = (WorldContext = "WorldContextObject"))
	static UAuraEventBus* GetEventBus(const UObject* WorldContextObject);

private:
	// 事件处理器映射表
	TMap<FName, TArray<TFunction<void(const void*)>>> EventHandlers;

	// 统计信息
	UPROPERTY()
	int32 TotalEventsPublished;

	UPROPERTY()
	int32 TotalSubscriptions;
};
