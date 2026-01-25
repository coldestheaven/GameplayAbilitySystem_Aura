// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EventSystem/AuraEventBus.h"
#include "EventSystem/AuraEventTypes.h"
#include "EventBusIntegrationExample.generated.h"

/**
 * 事件总线集成示例
 * 
 * 展示如何在现有系统中使用事件总线：
 * 1. 订阅事件
 * 2. 发布事件
 * 3. 取消订阅
 */
UCLASS()
class AURA_API AEventBusIntegrationExample : public AActor
{
	GENERATED_BODY()
	
public:	
	AEventBusIntegrationExample();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	virtual void Tick(float DeltaTime) override;

	// 示例：订阅所有事件
	UFUNCTION(BlueprintCallable, Category = "EventBus|Example")
	void SubscribeToAllEvents();

	// 示例：发布测试事件
	UFUNCTION(BlueprintCallable, Category = "EventBus|Example")
	void PublishTestEvents();

private:
	// 事件处理器
	void OnAttributeChanged(const FAttributeChangedEvent& Event);
	void OnAbilityActivated(const FAbilityActivatedEvent& Event);
	void OnDamageReceived(const FAuraDamageEvent& Event);
	void OnXPGained(const FXPGainedEvent& Event);
	void OnLevelUp(const FLevelUpEvent& Event);
	void OnCharacterDeath(const FCharacterDeathEvent& Event);

	// 订阅句柄
	TArray<int32> SubscriptionHandles;

	// 事件总线引用
	UPROPERTY()
	UAuraEventBus* EventBus;
};
