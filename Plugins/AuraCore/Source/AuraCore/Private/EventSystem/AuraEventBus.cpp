// Copyright Druid Mechanics

#include "EventSystem/AuraEventBus.h"
#include "Engine/GameInstance.h"

void UAuraEventBus::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	TotalEventsPublished = 0;
	TotalSubscriptions = 0;
	
	UE_LOG(LogTemp, Log, TEXT("[EventBus] Initialized"));
}

void UAuraEventBus::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("[EventBus] Deinitializing - Total Events: %d, Total Subscriptions: %d"), 
		TotalEventsPublished, TotalSubscriptions);
	
	ClearAllSubscriptions();
	
	Super::Deinitialize();
}

void UAuraEventBus::Unsubscribe(FName EventName, int32 Handle)
{
	if (EventHandlers.Contains(EventName))
	{
		// 注意：由于我们使用的是简化版本，这里只是清空
		// 在生产环境中，应该实现更精确的句柄管理
		UE_LOG(LogTemp, Log, TEXT("[EventBus] Unsubscribe requested for: %s"), *EventName.ToString());
	}
}

void UAuraEventBus::ClearAllSubscriptions()
{
	EventHandlers.Empty();
	UE_LOG(LogTemp, Log, TEXT("[EventBus] All subscriptions cleared"));
}

int32 UAuraEventBus::GetSubscriptionCount(FName EventName) const
{
	if (EventHandlers.Contains(EventName))
	{
		return EventHandlers[EventName].Num();
	}
	return 0;
}

void UAuraEventBus::PublishAttributeChanged(const FAttributeChangedEvent& Event)
{
	Publish(Event);
	TotalEventsPublished++;
}

void UAuraEventBus::PublishAbilityActivated(const FAbilityActivatedEvent& Event)
{
	Publish(Event);
	TotalEventsPublished++;
}

void UAuraEventBus::PublishDamage(const FAuraDamageEvent& Event)
{
	Publish(Event);
	TotalEventsPublished++;
}

void UAuraEventBus::PublishXPGained(const FXPGainedEvent& Event)
{
	Publish(Event);
	TotalEventsPublished++;
}

void UAuraEventBus::PublishLevelUp(const FLevelUpEvent& Event)
{
	Publish(Event);
	TotalEventsPublished++;
}

void UAuraEventBus::PublishCharacterDeath(const FCharacterDeathEvent& Event)
{
	Publish(Event);
	TotalEventsPublished++;
}

UAuraEventBus* UAuraEventBus::GetEventBus(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogTemp, Error, TEXT("[EventBus] GetEventBus: Invalid WorldContextObject"));
		return nullptr;
	}

	UGameInstance* GameInstance = WorldContextObject->GetWorld()->GetGameInstance();
	if (!GameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("[EventBus] GetEventBus: Invalid GameInstance"));
		return nullptr;
	}

	return GameInstance->GetSubsystem<UAuraEventBus>();
}
