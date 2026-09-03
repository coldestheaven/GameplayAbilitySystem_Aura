// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EventSystem/AuraEventBus.h"
#include "EventSystem/AuraEventTypes.h"
#include "EventBusIntegrationExample.generated.h"

/**
 * 事件总线集成示例 Actor（AuraCore 插件）
 *
 * 演示如何在游戏系统中使用 AuraEventBus 进行解耦通信。
 * 此类仅作为示例和参考，不在实际游戏逻辑中使用。
 *
 * 原位于游戏模块（/Script/Aura），已随事件总线整体归入 AuraCore 插件
 * （/Script/AuraCore）——示例应与其演示的功能同处一个模块。
 *
 * 事件总线模式说明：
 * - 发布者（Publisher）：通过 EventBus->Publish(Event) 发布事件
 * - 订阅者（Subscriber）：通过 EventBus->Subscribe<EventType>(Handler) 订阅事件
 * - 解耦优势：发布者和订阅者不需要直接引用对方
 *
 * 使用示例：
 *   int32 Handle = EventBus->Subscribe<FAuraDamageEvent>(
 *       [this](const FAuraDamageEvent& Event) { OnDamageReceived(Event); }
 *   );
 *   EventBus->Publish(DamageEvent);   // 蓝图亦可用万能通配符节点 PublishEvent
 *   EventBus->Unsubscribe(FAuraDamageEvent::StaticStruct()->GetFName(), Handle);
 */
UCLASS()
class AURACORE_API AEventBusIntegrationExample : public AActor
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

	/** 订阅所有事件类型（蓝图可调用，演示批量订阅） */
	UFUNCTION(BlueprintCallable, Category = "EventBus|Example")
	void SubscribeToAllEvents();

	/** 发布测试事件（蓝图可调用，演示事件发布） */
	UFUNCTION(BlueprintCallable, Category = "EventBus|Example")
	void PublishTestEvents();

private:
	void OnAttributeChanged(const FAttributeChangedEvent& Event);
	void OnAbilityActivated(const FAbilityActivatedEvent& Event);
	void OnDamageReceived(const FAuraDamageEvent& Event);
	void OnXPGained(const FXPGainedEvent& Event);
	void OnLevelUp(const FLevelUpEvent& Event);
	void OnCharacterDeath(const FCharacterDeathEvent& Event);

	/** 订阅句柄数组（EndPlay 时清理，防止 Actor 销毁后回调触发崩溃） */
	TArray<int32> SubscriptionHandles;

	/** 事件总线引用（BeginPlay 中从 GameInstance 获取） */
	UPROPERTY()
	TObjectPtr<UAuraEventBus> EventBus;
};
