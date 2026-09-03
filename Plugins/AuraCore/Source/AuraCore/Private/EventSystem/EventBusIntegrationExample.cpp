// Copyright Druid Mechanics

#include "EventSystem/EventBusIntegrationExample.h"
#include "GameplayTags/AuraGameplayTags.h"

AEventBusIntegrationExample::AEventBusIntegrationExample()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEventBusIntegrationExample::BeginPlay()
{
	Super::BeginPlay();

	EventBus = UAuraEventBus::GetEventBus(this);

	if (EventBus)
	{
		UE_LOG(LogTemp, Log, TEXT("[EventBusExample] EventBus obtained successfully"));
		SubscribeToAllEvents();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[EventBusExample] Failed to get EventBus"));
	}
}

void AEventBusIntegrationExample::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SubscriptionHandles.Empty();
	Super::EndPlay(EndPlayReason);
}

void AEventBusIntegrationExample::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AEventBusIntegrationExample::SubscribeToAllEvents()
{
	if (!EventBus) return;

	SubscriptionHandles.Add(EventBus->Subscribe<FAttributeChangedEvent>(
		[this](const FAttributeChangedEvent& Event) { OnAttributeChanged(Event); }));

	SubscriptionHandles.Add(EventBus->Subscribe<FAbilityActivatedEvent>(
		[this](const FAbilityActivatedEvent& Event) { OnAbilityActivated(Event); }));

	SubscriptionHandles.Add(EventBus->Subscribe<FAuraDamageEvent>(
		[this](const FAuraDamageEvent& Event) { OnDamageReceived(Event); }));

	SubscriptionHandles.Add(EventBus->Subscribe<FXPGainedEvent>(
		[this](const FXPGainedEvent& Event) { OnXPGained(Event); }));

	SubscriptionHandles.Add(EventBus->Subscribe<FLevelUpEvent>(
		[this](const FLevelUpEvent& Event) { OnLevelUp(Event); }));

	SubscriptionHandles.Add(EventBus->Subscribe<FCharacterDeathEvent>(
		[this](const FCharacterDeathEvent& Event) { OnCharacterDeath(Event); }));

	UE_LOG(LogTemp, Log, TEXT("[EventBusExample] Subscribed to %d events"), SubscriptionHandles.Num());
}

void AEventBusIntegrationExample::PublishTestEvents()
{
	if (!EventBus) return;

	FAttributeChangedEvent AttrEvent;
	AttrEvent.AttributeTag = FAuraGameplayTags::Get().Attributes_Secondary_MaxHealth;
	AttrEvent.OldValue = 100.f;
	AttrEvent.NewValue = 80.f;
	AttrEvent.Instigator = this;
	AttrEvent.Timestamp = GetWorld()->GetTimeSeconds();
	EventBus->Publish(AttrEvent);

	FAbilityActivatedEvent AbilityEvent;
	AbilityEvent.AbilityTag = FAuraGameplayTags::Get().Abilities_Fire_FireBolt;
	AbilityEvent.Instigator = this;
	AbilityEvent.Target = nullptr;
	AbilityEvent.Timestamp = GetWorld()->GetTimeSeconds();
	EventBus->Publish(AbilityEvent);

	FAuraDamageEvent DmgEvent;
	DmgEvent.Damage = 25.f;
	DmgEvent.bCriticalHit = true;
	DmgEvent.bBlockedHit = false;
	DmgEvent.Attacker = this;
	DmgEvent.Victim = nullptr;
	DmgEvent.Timestamp = GetWorld()->GetTimeSeconds();
	EventBus->Publish(DmgEvent);

	UE_LOG(LogTemp, Log, TEXT("[EventBusExample] Published test events"));
}

void AEventBusIntegrationExample::OnAttributeChanged(const FAttributeChangedEvent& Event)
{
	UE_LOG(LogTemp, Log, TEXT("[EventBusExample] Attribute Changed: %s | %f -> %f"),
		*Event.AttributeTag.ToString(), Event.OldValue, Event.NewValue);
}

void AEventBusIntegrationExample::OnAbilityActivated(const FAbilityActivatedEvent& Event)
{
	UE_LOG(LogTemp, Log, TEXT("[EventBusExample] Ability Activated: %s"),
		*Event.AbilityTag.ToString());
}

void AEventBusIntegrationExample::OnDamageReceived(const FAuraDamageEvent& Event)
{
	UE_LOG(LogTemp, Log, TEXT("[EventBusExample] Damage: %.1f | Critical: %s | Blocked: %s"),
		Event.Damage,
		Event.bCriticalHit ? TEXT("Yes") : TEXT("No"),
		Event.bBlockedHit ? TEXT("Yes") : TEXT("No"));
}

void AEventBusIntegrationExample::OnXPGained(const FXPGainedEvent& Event)
{
	UE_LOG(LogTemp, Log, TEXT("[EventBusExample] XP Gained: %d"), Event.XPAmount);
}

void AEventBusIntegrationExample::OnLevelUp(const FLevelUpEvent& Event)
{
	UE_LOG(LogTemp, Log, TEXT("[EventBusExample] Level Up: %d | Attribute Points: %d | Spell Points: %d"),
		Event.NewLevel, Event.AttributePointsGained, Event.SpellPointsGained);
}

void AEventBusIntegrationExample::OnCharacterDeath(const FCharacterDeathEvent& Event)
{
	UE_LOG(LogTemp, Log, TEXT("[EventBusExample] Character Death"));
}
