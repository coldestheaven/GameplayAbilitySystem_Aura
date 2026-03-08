// Copyright Druid Mechanics

#include "Actor/ObjectPoolTest.h"
#include "ObjectPool/ObjectPoolSubsystem.h"
#include "TimerManager.h"
#include "Engine/World.h"

/**
 * 构造函数：初始化对象池测试 Actor
 * 
 * 实现流程：
 * 1. 启用 Tick
 * 2. 初始化生成计数为 0
 */
AObjectPoolTest::AObjectPoolTest()
{
	PrimaryActorTick.bCanEverTick = true;
	SpawnedCount = 0;
}

/**
 * 游戏开始时初始化
 */
void AObjectPoolTest::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogTemp, Log, TEXT("ObjectPoolTest: BeginPlay"));
}

/**
 * 每帧更新（空实现）
 */
void AObjectPoolTest::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/**
 * 开始对象池测试
 * 
 * 实现流程：
 * 1. 校验 ProjectileClass 已配置
 * 2. 重置生成计数
 * 3. 获取对象池子系统
 * 4. 创建对象池（初始大小 10，最大大小 50，允许增长）
 * 5. 预热对象池（预生成 15 个对象）
 * 6. 启动定时器，定期生成投射物（SpawnTestProjectile）
 * 
 * 使用场景：
 * - 测试对象池系统性能时调用
 * 
 * 注意：
 * - 此函数用于测试和性能分析
 */
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
		
		// 预热池（预生成对象）
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

/**
 * 停止测试
 * 
 * 实现流程：
 * 1. 停止生成定时器
 * 2. 打印最终统计信息
 */
void AObjectPoolTest::StopTest()
{
	UE_LOG(LogTemp, Log, TEXT("ObjectPoolTest: Stopping test"));
	
	// 停止定时器
	GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
	
	// 打印最终统计
	PrintPoolStats();
}

/**
 * 打印对象池统计信息
 * 
 * 实现流程：
 * 1. 获取对象池子系统
 * 2. 获取对象池统计（总数、活跃数、非活跃数）
 * 3. 打印统计信息（包括生成计数）
 * 
 * 使用场景：
 * - 测试过程中定期调用
 * - 测试结束时调用
 */
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

/**
 * 生成测试投射物（定时器回调）
 * 
 * 实现流程：
 * 1. 检查是否达到生成数量上限，如果达到则停止测试
 * 2. 获取对象池子系统
 * 3. 计算随机生成位置（在 SpawnRadius 范围内）
 * 4. 计算随机旋转
 * 5. 从对象池获取 Actor（AcquireActor）
 * 6. 如果成功，递增计数并记录日志
 * 7. 每 5 个打印一次统计信息
 * 
 * 使用场景：
 * - 测试定时器定期调用
 * 
 * 注意：
 * - 使用对象池获取 Actor 而非直接生成（提高性能）
 */
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
	
	// 随机生成位置（在 SpawnRadius 范围内）
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
	
	// 从对象池获取 Actor（而非直接生成）
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
