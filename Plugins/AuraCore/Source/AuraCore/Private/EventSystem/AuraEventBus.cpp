// Copyright Druid Mechanics
// UAuraEventBus：C++ 走 Publish<T>() 模板；蓝图走 PublishEvent 通配符节点（CustomThunk）。
// 两条路径最终都汇入 PublishInternal（统一遍历 + 计数），新增事件类型零样板代码。

#include "EventSystem/AuraEventBus.h"
#include "Engine/GameInstance.h"
#include "UObject/Stack.h"

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

void UAuraEventBus::PublishEvent(const FAuraEvent& Event)
{
	// 不会被执行：蓝图调用走 execPublishEvent 自定义 thunk。
	// 防御性兜底：若被 C++ 以基类引用直接调用，按基类名发布（派生数据经引用切片仍完整，
	// 但订阅方以派生布局读取，故此处仅警告提示调用方改用 Publish<派生类型>）。
	UE_LOG(LogTemp, Warning, TEXT("[EventBus] PublishEvent should be called from Blueprint (wildcard node); use Publish<T>() in C++."));
	PublishInternal(FAuraEvent::StaticStruct()->GetFName(), &Event);
}

// 自定义 thunk：从蓝图虚拟机栈提取通配符结构体实参
// （CustomStructureParam 让蓝图侧"事件"引脚可接任意结构体，此处拿到其实际类型与地址）
DEFINE_FUNCTION(UAuraEventBus::execPublishEvent)
{
	Stack.MostRecentProperty = nullptr;
	Stack.StepCompiledIn<FStructProperty>(nullptr);
	const FStructProperty* StructProperty = CastField<FStructProperty>(Stack.MostRecentProperty);
	void* StructData = Stack.MostRecentPropertyAddress;
	P_FINISH;
	P_NATIVE_BEGIN;
	if (StructProperty && StructData)
	{
		const UScriptStruct* ScriptStruct = StructProperty->Struct;

		// 校验：实参结构体必须派生自 FAuraEvent（沿 SuperStruct 链向上查找）
		bool bIsAuraEvent = false;
		for (const UStruct* S = ScriptStruct; S != nullptr; S = S->GetSuperStruct())
		{
			if (S == FAuraEvent::StaticStruct())
			{
				bIsAuraEvent = true;
				break;
			}
		}

		if (bIsAuraEvent)
		{
			P_THIS->PublishInternal(ScriptStruct->GetFName(), StructData);
		}
		else
		{
			UE_LOG(LogTemp, Warning,
				TEXT("[EventBus] PublishEvent: struct '%s' does not derive from FAuraEvent, ignored."),
				*ScriptStruct->GetName());
		}
	}
	P_NATIVE_END;
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
