// Copyright Druid Mechanics

#include "Actor/ObjectPoolTest.h"
#include "ObjectPool/ObjectPoolSubsystem.h"
#include "TimerManager.h"
#include "Engine/World.h"

AObjectPoolTest::AObjectPoolTest()
{
	PrimaryActorTick.bCanEverTick = true;
	SpawnedCount = 0;
}

void AObjectPoolTest::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Log, TEXT("ObjectPoolTest: BeginPlay"));
}

void AObjectPoolTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AObjectPoolTest::StartTest()
{
	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolTest: ProjectileClass is not set!"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ObjectPoolTest: Starting test with %d projectiles"), NumProjectilesToSpawn);
	
	// 重置计数
	SpawnedCount = 0;
	
	// 获取对象池子系统
	UObjectPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
	if (PoolSubsystem)
	{
		// 创建并配置对象池
		FObjectPoolConfig Config;
		Config.InitialSize = 10;
		Config.MaxSize = 50;
		Config.bAllowGrowth = true;
		
		PoolSubsystem->CreatePool(ProjectileClass, Config);
		UE_LOG(LogTemp, Log, TEXT("ObjectPoolTest: Pool created"));
		
		// 预热池
		PoolSubsystem->WarmupPool(ProjectileClass, 15);
		UE_LOG(LogTemp, Log, TEXT("ObjectPoolTest: Pool warmed up"));
	}
	
	// 开始定时生成
	GetWorld()->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&AObjectPoolTest::SpawnTestProjectile,
		SpawnInterval,
		true
	);
}

void AObjectPoolTest::StopTest()
{
	UE_LOG(LogTemp, Log, TEXT("ObjectPoolTest: Stopping test"));
	
	// 停止定时器
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
	
	// 打印最终统计
	PrintPoolStats();
}

void AObjectPoolTest::PrintPoolStats()
{
	UObjectPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
	if (PoolSubsystem && ProjectileClass)
	{
		int32 Total, Active, Inactive;
		PoolSubsystem->GetPoolStats(ProjectileClass, Total, Active, Inactive);
		
		UE_LOG(LogTemp, Log, TEXT("=== Pool Statistics ==="));
		UE_LOG(LogTemp, Log, TEXT("Total Objects: %d"), Total);
		UE_LOG(LogTemp, Log, TEXT("Active Objects: %d"), Active);
		UE_LOG(LogTemp, Log, TEXT("Inactive Objects: %d"), Inactive);
		UE_LOG(LogTemp, Log, TEXT("Spawned Count: %d"), SpawnedCount);
		UE_LOG(LogTemp, Log, TEXT("======================"));
	}
}

void AObjectPoolTest::SpawnTestProjectile()
{
	if (SpawnedCount >= NumProjectilesToSpawn)
	{
		StopTest();
		return;
	}
	
	UObjectPoolSubsystem* PoolSubsystem = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
	if (!PoolSubsystem)
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolTest: PoolSubsystem not found!"));
		return;
	}
	
	// 随机生成位置
	FVector RandomOffset = FVector(
		FMath::RandRange(-SpawnRadius, SpawnRadius),
		FMath::RandRange(-SpawnRadius, SpawnRadius),
		FMath::RandRange(0.f, 200.f)
	);
	
	FVector SpawnLocation = GetActorLocation() + RandomOffset;
	FRotator SpawnRotation = FRotator(0.f, FMath::RandRange(0.f, 360.f), 0.f);
	
	FTransform SpawnTransform;
	SpawnTransform.SetLocation(SpawnLocation);
	SpawnTransform.SetRotation(SpawnRotation.Quaternion());
	
	// 从对象池获取Actor
	AActor* Actor = PoolSubsystem->AcquireActor(ProjectileClass, SpawnTransform);
	
	if (Actor)
	{
		SpawnedCount++;
		UE_LOG(LogTemp, Log, TEXT("ObjectPoolTest: Spawned projectile %d at %s"), 
			   SpawnedCount, *SpawnLocation.ToString());
		
		// 每5个打印一次统计
		if (SpawnedCount % 5 == 0)
		{
			PrintPoolStats();
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ObjectPoolTest: Failed to acquire actor from pool!"));
	}
}
