// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ObjectPoolSubsystem.generated.h"

/**
 * 对象池配置结构体
 * 定义单个对象池的行为参数
 */
USTRUCT(BlueprintType)
struct FObjectPoolConfig
{
	GENERATED_BODY()

	/**
	 * 初始池大小
	 * 创建池时预先生成的 Actor 数量
	 * 默认 10 个
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
	int32 InitialSize = 10;

	/**
	 * 最大池大小（0 表示无限制）
	 * 池中 Actor 的最大数量（活动 + 非活动）
	 * 超过此数量时，AcquireActor 将返回 nullptr（如果 bAllowGrowth=false）
	 * 默认 100 个
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
	int32 MaxSize = 100;

	/**
	 * 是否允许动态扩展
	 * true：当池中没有可用 Actor 时，自动创建新的 Actor
	 * false：当池中没有可用 Actor 时，AcquireActor 返回 nullptr
	 * 默认 true
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
	bool bAllowGrowth = true;

	/**
	 * 自动回收时间（秒，0 表示不自动回收）
	 * Actor 被取出后，超过此时间未归还则自动归还到池中
	 * 用于防止 Actor 泄漏（如投射物命中后未正确归还）
	 * 默认 0（不自动回收）
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
	float AutoReleaseTime = 0.f;
};

/**
 * 对象池子系统（World Subsystem）
 *
 * 管理可复用的 Actor 对象，减少频繁创建/销毁 Actor 的性能开销
 * 特别适用于高频生成的对象（如投射物、特效 Actor 等）
 *
 * 工作原理：
 * - 每种 Actor 类型维护一个独立的对象池（FActorPool）
 * - 取出（Acquire）：从非活动列表取出 Actor，激活并移到活动列表
 * - 归还（Release）：将 Actor 停用并移回非活动列表
 * - 预热（Warmup）：提前创建指定数量的 Actor，避免运行时卡顿
 *
 * 使用方式：
 *   // 获取子系统
 *   UObjectPoolSubsystem* Pool = GetWorld()->GetSubsystem<UObjectPoolSubsystem>();
 *   // 创建投射物池
 *   FObjectPoolConfig Config;
 *   Config.InitialSize = 20;
 *   Pool->CreatePool(AAuraProjectile::StaticClass(), Config);
 *   // 从池中取出投射物
 *   AActor* Projectile = Pool->AcquireActor(AAuraProjectile::StaticClass(), SpawnTransform);
 *   // 归还投射物到池中
 *   Pool->ReleaseActor(Projectile);
 *
 * 注意：
 * - Actor 必须实现 IPoolableObject 接口（OnAcquiredFromPool/OnReturnedToPool）
 * - 此子系统在关卡结束时自动清理所有池
 */
UCLASS()
class AURACORE_API UObjectPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** 初始化子系统（创建内部数据结构） */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** 反初始化子系统（清理所有对象池，销毁所有 Actor） */
	virtual void Deinitialize() override;

	/**
	 * 创建对象池（蓝图可调用）
	 * 为指定 Actor 类创建一个新的对象池，并根据 Config.InitialSize 预先生成 Actor
	 * 如果该类的池已存在，此函数不做任何操作
	 * @param ActorClass 要池化的 Actor 类（必须实现 IPoolableObject 接口）
	 * @param Config     池配置（大小、扩展策略等）
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void CreatePool(TSubclassOf<AActor> ActorClass, const FObjectPoolConfig& Config);

	/**
	 * 从池中获取 Actor（蓝图可调用）
	 * 从非活动列表取出一个 Actor，设置其变换，调用 OnAcquiredFromPool
	 * 如果池为空且 bAllowGrowth=true，则创建新的 Actor
	 * @param ActorClass     Actor 类
	 * @param SpawnTransform 生成位置和旋转
	 * @return 取出的 Actor，如果失败（池满且不允许扩展）则返回 nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	AActor* AcquireActor(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform);

	/**
	 * 归还 Actor 到池中（蓝图可调用）
	 * 调用 Actor 的 OnReturnedToPool，将其移回非活动列表
	 * @param Actor 要归还的 Actor（必须是从此池取出的）
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void ReleaseActor(AActor* Actor);

	/**
	 * 预热池（蓝图可调用）
	 * 提前创建指定数量的 Actor 并放入非活动列表
	 * 在关卡开始时调用，避免游戏过程中的卡顿
	 * @param ActorClass Actor 类
	 * @param Count      要预先创建的数量
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void WarmupPool(TSubclassOf<AActor> ActorClass, int32 Count);

	/**
	 * 清空池（蓝图可调用）
	 * 销毁指定类型的所有 Actor（活动和非活动），清空池
	 * @param ActorClass 要清空的 Actor 类
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void ClearPool(TSubclassOf<AActor> ActorClass);

	/**
	 * 获取池统计信息（蓝图可调用，用于调试）
	 * @param ActorClass    要查询的 Actor 类
	 * @param OutTotal      输出：池中 Actor 总数（活动 + 非活动）
	 * @param OutActive     输出：当前活动的 Actor 数量
	 * @param OutInactive   输出：当前非活动（可用）的 Actor 数量
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool|Debug")
	void GetPoolStats(TSubclassOf<AActor> ActorClass, int32& OutTotal, int32& OutActive, int32& OutInactive);

private:
	/**
	 * 单个对象池的内部数据结构
	 * 维护活动和非活动 Actor 列表，提供 Acquire/Release 操作
	 */
	struct FActorPool
	{
		/** 此池管理的 Actor 类 */
		TSubclassOf<AActor> ActorClass;

		/** 池配置（大小限制、扩展策略等） */
		FObjectPoolConfig Config;

		/** 非活动 Actor 列表（可以被取出使用） */
		TArray<AActor*> InactiveActors;

		/** 活动 Actor 列表（当前正在使用中） */
		TArray<AActor*> ActiveActors;

		/** 从池中取出一个 Actor（内部实现） */
		AActor* Acquire(UWorld* World, const FTransform& SpawnTransform);

		/** 将 Actor 归还到池中（内部实现） */
		void Release(AActor* Actor);

		/** 清空池（销毁所有 Actor） */
		void Clear();

		/** 预热池（创建指定数量的 Actor） */
		void Warmup(UWorld* World, int32 Count);
	};

	/**
	 * 对象池映射表
	 * Key: Actor 类
	 * Value: 对应的对象池（SharedPtr 管理生命周期）
	 */
	TMap<TSubclassOf<AActor>, TSharedPtr<FActorPool>> Pools;

	/**
	 * 获取或创建指定类型的对象池
	 * 如果池不存在，使用默认配置创建新池
	 * @param ActorClass 要获取/创建池的 Actor 类
	 * @return 对应的对象池 SharedPtr
	 */
	TSharedPtr<FActorPool> GetOrCreatePool(TSubclassOf<AActor> ActorClass);
};
