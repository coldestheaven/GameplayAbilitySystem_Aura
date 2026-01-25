// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ObjectPoolSubsystem.generated.h"

/**
 * 对象池配置
 */
USTRUCT(BlueprintType)
struct FObjectPoolConfig
{
	GENERATED_BODY()

	// 初始池大小
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
	int32 InitialSize = 10;

	// 最大池大小（0表示无限制）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
	int32 MaxSize = 100;

	// 是否允许动态扩展
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
	bool bAllowGrowth = true;

	// 自动回收时间（秒，0表示不自动回收）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ObjectPool")
	float AutoReleaseTime = 0.f;
};

/**
 * 对象池子系统
 * 管理可复用的Actor对象，减少频繁创建销毁的开销
 */
UCLASS()
class AURACORE_API UObjectPoolSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/**
	 * 创建对象池
	 * @param ActorClass 要池化的Actor类
	 * @param Config 池配置
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void CreatePool(TSubclassOf<AActor> ActorClass, const FObjectPoolConfig& Config);

	/**
	 * 从池中获取Actor
	 * @param ActorClass Actor类
	 * @param SpawnTransform 生成位置
	 * @return 获取的Actor，如果失败返回nullptr
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	AActor* AcquireActor(TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform);

	/**
	 * 归还Actor到池中
	 * @param Actor 要归还的Actor
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void ReleaseActor(AActor* Actor);

	/**
	 * 预热池（提前创建对象）
	 * @param ActorClass Actor类
	 * @param Count 要创建的数量
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void WarmupPool(TSubclassOf<AActor> ActorClass, int32 Count);

	/**
	 * 清空池
	 * @param ActorClass Actor类
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool")
	void ClearPool(TSubclassOf<AActor> ActorClass);

	/**
	 * 获取池统计信息
	 * @param ActorClass Actor类
	 * @param OutTotal 总数
	 * @param OutActive 活动数
	 * @param OutInactive 非活动数
	 */
	UFUNCTION(BlueprintCallable, Category = "ObjectPool|Debug")
	void GetPoolStats(TSubclassOf<AActor> ActorClass, int32& OutTotal, int32& OutActive, int32& OutInactive);

private:
	// 单个对象池
	struct FActorPool
	{
		TSubclassOf<AActor> ActorClass;
		FObjectPoolConfig Config;
		TArray<AActor*> InactiveActors;
		TArray<AActor*> ActiveActors;

		AActor* Acquire(UWorld* World, const FTransform& SpawnTransform);
		void Release(AActor* Actor);
		void Clear();
		void Warmup(UWorld* World, int32 Count);
	};

	// 对象池映射
	TMap<TSubclassOf<AActor>, TSharedPtr<FActorPool>> Pools;

	// 获取或创建池
	TSharedPtr<FActorPool> GetOrCreatePool(TSubclassOf<AActor> ActorClass);
};
