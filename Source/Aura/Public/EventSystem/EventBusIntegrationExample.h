// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EventSystem/AuraEventBus.h"
#include "EventSystem/AuraEventTypes.h"
#include "EventBusIntegrationExample.generated.h"

/**
 * 事件总线集成示例 Actor
 *
 * 演示如何在游戏系统中使用 AuraEventBus 进行解耦通信
 * 此类仅作为示例和参考，不在实际游戏逻辑中使用
 *
 * 事件总线模式说明：
 * - 发布者（Publisher）：通过 EventBus->Publish(Event) 发布事件
 * - 订阅者（Subscriber）：通过 EventBus->Subscribe<EventType>(Handler) 订阅事件
 * - 解耦优势：发布者和订阅者不需要直接引用对方
 *
 * 支持的事件类型（来自 AuraEventTypes.h）：
 * - FAttributeChangedEvent：属性值变化事件（生命值、法力值等）
 * - FAbilityActivatedEvent：技能激活事件
 * - FAuraDamageEvent：伤害事件
 * - FXPGainedEvent：获得经验值事件
 * - FLevelUpEvent：升级事件
 * - FCharacterDeathEvent：角色死亡事件
 *
 * 使用示例：
 *   // 订阅伤害事件
 *   int32 Handle = EventBus->Subscribe<FAuraDamageEvent>(
 *       [this](const FAuraDamageEvent& Event) { OnDamageReceived(Event); }
 *   );
 *   // 发布伤害事件
 *   FAuraDamageEvent DamageEvent;
 *   DamageEvent.DamageAmount = 100.f;
 *   EventBus->Publish(DamageEvent);
 *   // 取消订阅
 *   EventBus->Unsubscribe(Handle);
 */
UCLASS()
class AURA_API AEventBusIntegrationExample : public AActor
{
	GENERATED_BODY()
	
public:	
	AEventBusIntegrationExample();

protected:
	virtual void BeginPlay() override;

	/** 结束时取消所有订阅，防止内存泄漏 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	virtual void Tick(float DeltaTime) override;

	/**
	 * 订阅所有事件类型（蓝图可调用）
	 * 演示如何一次性订阅多种事件
	 * 订阅句柄存储在 SubscriptionHandles 中，用于后续取消订阅
	 */
	UFUNCTION(BlueprintCallable, Category = "EventBus|Example")
	void SubscribeToAllEvents();

	/**
	 * 发布测试事件（蓝图可调用）
	 * 演示如何发布各种类型的事件
	 * 可在编辑器中调用此函数测试事件系统
	 */
	UFUNCTION(BlueprintCallable, Category = "EventBus|Example")
	void PublishTestEvents();

private:
	/** 属性变化事件处理器（演示如何响应属性变化） */
	void OnAttributeChanged(const FAttributeChangedEvent& Event);

	/** 技能激活事件处理器（演示如何响应技能激活） */
	void OnAbilityActivated(const FAbilityActivatedEvent& Event);

	/** 伤害事件处理器（演示如何响应伤害） */
	void OnDamageReceived(const FAuraDamageEvent& Event);

	/** 经验值获得事件处理器（演示如何响应 XP 变化） */
	void OnXPGained(const FXPGainedEvent& Event);

	/** 升级事件处理器（演示如何响应升级） */
	void OnLevelUp(const FLevelUpEvent& Event);

	/** 角色死亡事件处理器（演示如何响应死亡） */
	void OnCharacterDeath(const FCharacterDeathEvent& Event);

	/**
	 * 订阅句柄数组
	 * 存储所有订阅的句柄，在 EndPlay 时遍历取消订阅
	 * 防止 Actor 销毁后事件回调仍然触发导致崩溃
	 */
	TArray<int32> SubscriptionHandles;

	/**
	 * 事件总线引用
	 * 在 BeginPlay 中从 GameInstance 或全局单例获取
	 */
	UPROPERTY()
	UAuraEventBus* EventBus;
};
