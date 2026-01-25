// Copyright Druid Mechanics

#include "EventSystem/EventBusIntegrationExample.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"

AEventBusIntegrationExample::AEventBusIntegrationExample()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AEventBusIntegrationExample::BeginPlay()
{
	Super::BeginPlay();
	
	// 获取事件总线实例
	EventBus = UAuraEventBus::GetEventBus(this);
	
	if (EventBus)
	{
		UE_LOG(LogTemp, Log, TEXT("[EventBusExample] EventBus obtained successfully"));
		
		// 自动订阅所有事件
		SubscribeToAllEvents();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[EventBusExample] Failed to get EventBus"));
	}
}

void AEventBusIntegrationExample::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 清理订阅
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

	// 订阅属性变化事件
	auto AttributeHandle = EventBus->Subscribe<FAttributeChangedEvent>(
		[this](const FAttributeChangedEvent& Event) {
			OnAttributeChanged(Event);
		}
	);
	SubscriptionHandles.Add(AttributeHandle);

	// 订阅技能激活事件
	auto AbilityHandle = EventBus->Subscribe<FAbilityActivatedEvent>(
		[this](const FAbilityActivatedEvent& Event) {
			OnAbilityActivated(Event);
		}
	);
	SubscriptionHandles.Add(AbilityHandle);

	// 订阅伤害事件
	auto DamageHandle = EventBus->Subscribe<FAuraDamageEvent>(
		[this](const FAuraDamageEvent& Event) {
			OnDamageReceived(Event);
		}
	);
	SubscriptionHandles.Add(DamageHandle);

	// 订阅经验值事件
	auto XPHandle = EventBus->Subscribe<FXPGainedEvent>(
		[this](const FXPGainedEvent& Event) {
			OnXPGained(Event);
		}
	);
	SubscriptionHandles.Add(XPHandle);

	// 订阅升级事件
	auto LevelUpHandle = EventBus->Subscribe<FLevelUpEvent>(
		[this](const FLevelUpEvent& Event) {
			OnLevelUp(Event);
		}
	);
	SubscriptionHandles.Add(LevelUpHandle);

	// 订阅死亡事件
	auto DeathHandle = EventBus->Subscribe<FCharacterDeathEvent>(
		[this](const FCharacterDeathEvent& Event) {
			OnCharacterDeath(Event);
		}
	);
	SubscriptionHandles.Add(DeathHandle);

	UE_LOG(LogTemp, Log, TEXT("[EventBusExample] Subscribed to %d events"), SubscriptionHandles.Num());
}

void AEventBusIntegrationExample::PublishTestEvents()
{
	if (!EventBus) return;

	// 测试：发布属性变化事件
	FAttributeChangedEvent AttrEvent;
	AttrEvent.AttributeTag = FGameplayTag::RequestGameplayTag(FName("Attributes.Vital.Health"));
	AttrEvent.OldValue = 100.f;
	AttrEvent.NewValue = 80.f;
	AttrEvent.Instigator = this;
	AttrEvent.Timestamp = GetWorld()->GetTimeSeconds();
	EventBus->Publish(AttrEvent);

	// 测试：发布技能激活事件
	FAbilityActivatedEvent AbilityEvent;
	AbilityEvent.AbilityTag = FGameplayTag::RequestGameplayTag(FName("Abilities.Fire.FireBolt"));
	AbilityEvent.Instigator = this;
	AbilityEvent.Target = nullptr;
	AbilityEvent.Timestamp = GetWorld()->GetTimeSeconds();
	EventBus->Publish(AbilityEvent);

	// 测试：发布伤害事件
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
