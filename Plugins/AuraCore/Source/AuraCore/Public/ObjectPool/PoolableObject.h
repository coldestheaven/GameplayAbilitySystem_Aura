// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PoolableObject.generated.h"

/**
 * 可池化对象接口
 * 实现此接口的Actor可以被对象池管理
 */
UINTERFACE(MinimalAPI, Blueprintable)
class UPoolableObject : public UInterface
{
	GENERATED_BODY()
};

class AURACORE_API IPoolableObject
{
	GENERATED_BODY()

public:
	/**
	 * 从对象池中获取时调用
	 * 用于重置对象状态，准备复用
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectPool")
	void OnAcquiredFromPool();

	/**
	 * 归还到对象池时调用
	 * 用于清理对象状态，准备回收
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectPool")
	void OnReturnedToPool();

	/**
	 * 检查对象是否可以归还到池中
	 * @return true表示可以归还，false表示应该销毁
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "ObjectPool")
	bool CanReturnToPool() const;
};
