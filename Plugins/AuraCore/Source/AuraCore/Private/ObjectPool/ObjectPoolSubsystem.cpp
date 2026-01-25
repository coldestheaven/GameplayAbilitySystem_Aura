// Copyright Druid Mechanics

#include "ObjectPool/ObjectPoolSubsystem.h"
#include "ObjectPool/PoolableObject.h"
#include "Engine/World.h"

void UObjectPoolSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTemp, Log, TEXT("ObjectPoolSubsystem Initialized"));
}

void UObjectPoolSubsystem::Deinitialize()
{
	// 清空所有池
	for (auto& Pair : Pools)
	{
		if (Pair.Value.IsValid())
		{
			Pair.Value->Clear();
		}
	}
	Pools.Empty();

	Super::Deinitialize();
	UE_LOG(LogTemp, Log, TEXT("ObjectPoolSubsystem Deinitialized"));
}

void UObjectPoolSubsystem::CreatePool(TSubclassOf<AActor> ActorClass, const FObjectPoolConfig& Config)
{
	if (!ActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("CreatePool: ActorClass is null"));
		return;
	}

	TSharedPtr<FActorPool> Pool = MakeShared<FActorPool>();
	Pool->ActorClass = ActorClass;
	Pool->Config = Config;

	// 预创建对象
	Pool->Warmup(GetWorld(), Config.InitialSize);

	Pools.Add(ActorClass, Pool);

	UE_LOG(LogTemp, Log, TEXT("Created pool for %s with %d objects"),
		*ActorClass->GetName(), Config.InitialSize);
}

AActor* UObjectPoolSubsystem::AcquireActor(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform)
{
	if (!ActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("AcquireActor: ActorClass is null"));
		return nullptr;
	}

	TSharedPtr<FActorPool> Pool = GetOrCreatePool(ActorClass);
	if (!Pool.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("AcquireActor: Failed to get or create pool for %s"), *ActorClass->GetName());
		return nullptr;
	}

	return Pool->Acquire(GetWorld(), SpawnTransform);
}

void UObjectPoolSubsystem::ReleaseActor(AActor* Actor)
{
	if (!Actor)
	{
		UE_LOG(LogTemp, Warning, TEXT("ReleaseActor: Actor is null"));
		return;
	}

	TSubclassOf<AActor> ActorClass = Actor->GetClass();
	TSharedPtr<FActorPool> Pool = GetOrCreatePool(ActorClass);
	if (Pool.IsValid())
	{
		Pool->Release(Actor);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ReleaseActor: Failed to find pool for %s"), *ActorClass->GetName());
	}
}

void UObjectPoolSubsystem::WarmupPool(TSubclassOf<AActor> ActorClass, int32 Count)
{
	if (!ActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("WarmupPool: ActorClass is null"));
		return;
	}

	TSharedPtr<FActorPool> Pool = GetOrCreatePool(ActorClass);
	if (Pool.IsValid())
	{
		Pool->Warmup(GetWorld(), Count);
	}
}

void UObjectPoolSubsystem::ClearPool(TSubclassOf<AActor> ActorClass)
{
	if (!ActorClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ClearPool: ActorClass is null"));
		return;
	}

	if (Pools.Contains(ActorClass))
	{
		TSharedPtr<FActorPool> Pool = Pools[ActorClass];
		if (Pool.IsValid())
		{
			Pool->Clear();
		}
		Pools.Remove(ActorClass);
		UE_LOG(LogTemp, Log, TEXT("Cleared pool for %s"), *ActorClass->GetName());
	}
}

void UObjectPoolSubsystem::GetPoolStats(TSubclassOf<AActor> ActorClass, int32& OutTotal, int32& OutActive, int32& OutInactive)
{
	OutTotal = 0;
	OutActive = 0;
	OutInactive = 0;

	if (!ActorClass || !Pools.Contains(ActorClass))
	{
		return;
	}

	TSharedPtr<FActorPool> Pool = Pools[ActorClass];
	if (Pool.IsValid())
	{
		OutActive = Pool->ActiveActors.Num();
		OutInactive = Pool->InactiveActors.Num();
		OutTotal = OutActive + OutInactive;
	}
}

TSharedPtr<UObjectPoolSubsystem::FActorPool> UObjectPoolSubsystem::GetOrCreatePool(TSubclassOf<AActor> ActorClass)
{
	if (!ActorClass)
	{
		return nullptr;
	}

	if (Pools.Contains(ActorClass))
	{
		return Pools[ActorClass];
	}

	// 创建默认配置的池
	FObjectPoolConfig DefaultConfig;
	DefaultConfig.InitialSize = 5;
	DefaultConfig.MaxSize = 50;
	DefaultConfig.bAllowGrowth = true;

	TSharedPtr<FActorPool> Pool = MakeShared<FActorPool>();
	Pool->ActorClass = ActorClass;
	Pool->Config = DefaultConfig;
	Pool->Warmup(GetWorld(), DefaultConfig.InitialSize);

	Pools.Add(ActorClass, Pool);

	UE_LOG(LogTemp, Log, TEXT("Auto-created pool for %s with default config"), *ActorClass->GetName());

	return Pool;
}

// FActorPool 实现

AActor* UObjectPoolSubsystem::FActorPool::Acquire(UWorld* World, const FTransform& SpawnTransform)
{
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("FActorPool::Acquire: World is null"));
		return nullptr;
	}

	AActor* Actor = nullptr;

	// 从非活动池中获取
	if (InactiveActors.Num() > 0)
	{
		Actor = InactiveActors.Pop();
	}
	// 如果允许扩展且未达到最大值，创建新对象
	else if (Config.bAllowGrowth && (Config.MaxSize == 0 || (ActiveActors.Num() + InactiveActors.Num()) < Config.MaxSize))
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		Actor = World->SpawnActor<AActor>(ActorClass, SpawnTransform, SpawnParams);

		if (Actor)
		{
			UE_LOG(LogTemp, Warning, TEXT("Pool expanded for %s (Total: %d)"),
				*ActorClass->GetName(), ActiveActors.Num() + InactiveActors.Num() + 1);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Pool exhausted for %s (Max: %d)"), *ActorClass->GetName(), Config.MaxSize);
		return nullptr;
	}

	if (Actor)
	{
		// 重置Actor状态
		Actor->SetActorTransform(SpawnTransform);
		Actor->SetActorHiddenInGame(false);
		Actor->SetActorEnableCollision(true);
		Actor->SetActorTickEnabled(true);

		// 调用接口方法
		if (Actor->Implements<UPoolableObject>())
		{
			IPoolableObject::Execute_OnAcquiredFromPool(Actor);
		}

		ActiveActors.Add(Actor);
	}

	return Actor;
}

void UObjectPoolSubsystem::FActorPool::Release(AActor* Actor)
{
	if (!Actor)
	{
		return;
	}

	// 检查是否可以归还
	if (Actor->Implements<UPoolableObject>())
	{
		if (!IPoolableObject::Execute_CanReturnToPool(Actor))
		{
			Actor->Destroy();
			ActiveActors.Remove(Actor);
			UE_LOG(LogTemp, Log, TEXT("Actor %s cannot return to pool, destroyed"), *Actor->GetName());
			return;
		}
	}

	// 重置Actor状态
	Actor->SetActorHiddenInGame(true);
	Actor->SetActorEnableCollision(false);
	Actor->SetActorTickEnabled(false);

	// 调用接口方法
	if (Actor->Implements<UPoolableObject>())
	{
		IPoolableObject::Execute_OnReturnedToPool(Actor);
	}

	// 移动到非活动池
	ActiveActors.Remove(Actor);
	InactiveActors.Add(Actor);
}

void UObjectPoolSubsystem::FActorPool::Clear()
{
	// 销毁所有Actor
	for (AActor* Actor : InactiveActors)
	{
		if (Actor && IsValid(Actor))
		{
			Actor->Destroy();
		}
	}

	for (AActor* Actor : ActiveActors)
	{
		if (Actor && IsValid(Actor))
		{
			Actor->Destroy();
		}
	}

	InactiveActors.Empty();
	ActiveActors.Empty();
}

void UObjectPoolSubsystem::FActorPool::Warmup(UWorld* World, int32 Count)
{
	if (!World || !ActorClass)
	{
		return;
	}

	for (int32 i = 0; i < Count; ++i)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* Actor = World->SpawnActor<AActor>(ActorClass, FTransform::Identity, SpawnParams);
		if (Actor)
		{
			Actor->SetActorHiddenInGame(true);
			Actor->SetActorEnableCollision(false);
			Actor->SetActorTickEnabled(false);
			InactiveActors.Add(Actor);
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Warmed up pool for %s with %d objects"), *ActorClass->GetName(), Count);
}
