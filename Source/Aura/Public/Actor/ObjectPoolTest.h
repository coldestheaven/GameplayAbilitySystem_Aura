// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ObjectPoolTest.generated.h"

/**
 * 对象池测试Actor
 * 用于验证对象池系统的功能
 */
UCLASS()
class AURA_API AObjectPoolTest : public AActor
{
	GENERATED_BODY()
	
public:	
	AObjectPoolTest();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	// 测试投射物类
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	TSubclassOf<AActor> ProjectileClass;

	// 测试参数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	int32 NumProjectilesToSpawn = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	float SpawnInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Test")
	float SpawnRadius = 500.f;

	// 测试函数
	UFUNCTION(BlueprintCallable, Category = "Test")
	void StartTest();

	UFUNCTION(BlueprintCallable, Category = "Test")
	void StopTest();

	UFUNCTION(BlueprintCallable, Category = "Test")
	void PrintPoolStats();

private:
	FTimerHandle SpawnTimerHandle;
	int32 SpawnedCount;

	void SpawnTestProjectile();
};
