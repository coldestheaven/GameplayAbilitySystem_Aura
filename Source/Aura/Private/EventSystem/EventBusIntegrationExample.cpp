// Copyright Druid Mechanics

#include "EventSystem/EventBusIntegrationExample.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"

/**
 * 构造函数：初始化事件总线集成示例
 */
AEventBusIntegrationExample::AEventBusIntegrationExample()
{
	PrimaryActorTick.bCanEverTick = true;
}

/**
 * 游戏开始时初始化
 * 
 * 实现流程：
 * 1. 调用父类 BeginPlay
 * 2. 获取事件总线实例
 * 3. 如果成功，订阅所有事件
 */
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

/**
 * Actor 销毁时清理
 * 
 * 实现流程：
 * 1. 清空订阅句柄列表（自动取消订阅）
 * 2. 调用父类 EndPlay
 */
void AEventBusIntegrationExample::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 清理订阅
	SubscriptionHandles.Empty();
	
	Super::EndPlay(EndPlayReason);
}

/**
 * 每帧更新（空实现）
 */
void AEventBusIntegrationExample::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/**
 * 订阅所有事件类型
 * 
 * 实现流程：
 * 1. 订阅属性变化事件（OnAttributeChanged）
 * 2. 订阅技能激活事件（OnAbilityActivated）
 * 3. 订阅伤害事件（OnDamageReceived）
 * 4. 订阅经验值事件（OnXPGained）
 * 5. 订阅升级事件（OnLevelUp）
 * 6. 订阅死亡事件（OnCharacterDeath）
 * 7. 保存所有订阅句柄（用于取消订阅）
 * 
 * 使用场景：
 * - BeginPlay 时调用，监听所有游戏事件
 * 
 * 注意：
 * - 订阅句柄用于在 EndPlay 时自动取消订阅
 */
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

/**
 * 发布测试事件（用于测试事件总线）
 * 
 * 实现流程：
 * 1. 创建并发布属性变化事件（生命值从 100 变为 80）
 * 2. 创建并发布技能激活事件（FireBolt）
 * 3. 创建并发布伤害事件（25 伤害，暴击）
 * 
 * 使用场景：
 * - 测试事件总线功能时调用
 */
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

/**
 * 属性变化事件回调
 * 
 * @param Event 属性变化事件（包含属性标签、旧值、新值）
 * 
 * 使用场景：
 * - 属性变化时由事件总线调用
 */
void AEventBusIntegrationExample::OnAttributeChanged(const FAttributeChangedEvent& Event)
{
	UE_LOG(LogTemp, Log, TEXT("[EventBusExample] Attribute Changed: %s | %f -> %f"), 
		*Event.AttributeTag.ToString(), Event.OldValue, Event.NewValue);
}

/**
 * 技能激活事件回调
 * 
 * @param Event 技能激活事件（包含技能标签、触发者、目标）
 */
void AEventBusIntegrationExample::OnAbilityActivated(const FAbilityActivatedEvent& Event)
{
	UE_LOG(LogTemp, Log, TEXT("[EventBusExample] Ability Activated: %s"), 
		*Event.AbilityTag.ToString());
}

/**
 * 伤害事件回调
 * 
 * @param Event 伤害事件（包含伤害值、是否暴击、是否格挡）
 */
void AEventBusIntegrationExample::OnDamageReceived(const FAuraDamageEvent& Event)
{
	UE_LOG(LogTemp, Log, TEXT("[EventBusExample] Damage: %.1f | Critical: %s | Blocked: %s"), 
		Event.Damage, 
		Event.bCriticalHit ? TEXT("Yes") : TEXT("No"),
		Event.bBlockedHit ? TEXT("Yes") : TEXT("No"));
}

/**
 * 经验值获得事件回调
 * 
 * @param Event 经验值事件（包含获得的 XP 数量）
 */
void AEventBusIntegrationExample::OnXPGained(const FXPGainedEvent& Event)
{
	UE_LOG(LogTemp, Log, TEXT("[EventBusExample] XP Gained: %d"), Event.XPAmount);
}

/**
 * 升级事件回调
 * 
 * @param Event 升级事件（包含新等级、获得的属性点、技能点）
 */
void AEventBusIntegrationExample::OnLevelUp(const FLevelUpEvent& Event)
{
	UE_LOG(LogTemp, Log, TEXT("[EventBusExample] Level Up: %d | Attribute Points: %d | Spell Points: %d"), 
		Event.NewLevel, Event.AttributePointsGained, Event.SpellPointsGained);
}

/**
 * 角色死亡事件回调
 * 
 * @param Event 角色死亡事件
 */
void AEventBusIntegrationExample::OnCharacterDeath(const FCharacterDeathEvent& Event)
{
	UE_LOG(LogTemp, Log, TEXT("[EventBusExample] Character Death"));
}
