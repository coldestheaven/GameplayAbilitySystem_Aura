# 架构优化建议文档

## 概述

本文档基于对 Aura 项目代码的深入分析，提供了系统性的架构优化建议，涵盖性能、代码结构、网络和内存等方面。

---

## 1. 性能优化

### 1.1 Tick 优化

**问题**: 项目中大量使用 `Tick`，16 个文件包含 Tick 相关代码。

**影响**: 
- 每帧都会执行，即使不需要更新
- 增加 CPU 开销
- 影响性能

**优化建议**:

#### 1.1.1 使用事件驱动替代 Tick

```cpp
// ❌ 当前实现（AuraCharacterBase.cpp）
void AAuraCharacterBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    EffectAttachComponent->SetWorldRotation(FRotator::ZeroRotator);
}

// ✅ 优化后
void AAuraCharacterBase::OnPassiveEffectActivated(const FGameplayTag& AbilityTag, bool bActivate)
{
    if (bActivate && AbilityTag == HaloOfProtectionTag)
    {
        // 只在需要时更新
        EffectAttachComponent->SetWorldRotation(FRotator::ZeroRotator);
    }
}
```

#### 1.1.2 使用定时器替代 Tick

```cpp
// ✅ 对于需要定期更新的逻辑
void AAuraCharacterBase::BeginPlay()
{
    Super::BeginPlay();
    
    // 只在需要时启动定时器
    if (bNeedsPeriodicUpdate)
    {
        GetWorldTimerManager().SetTimer(
            UpdateTimerHandle,
            this,
            &AAuraCharacterBase::UpdateEffectRotation,
            0.1f,  // 降低更新频率
            true
        );
    }
}
```

#### 1.1.3 条件性 Tick

```cpp
// ✅ 只在必要时启用 Tick
void AAuraCharacterBase::SetNeedsTick(bool bNeeds)
{
    PrimaryActorTick.bCanEverTick = bNeeds;
    PrimaryActorTick.SetTickFunctionEnable(bNeeds);
}
```

**优化文件**:
- `AuraCharacterBase.cpp` - EffectAttachComponent 旋转
- `AuraEffectActor.cpp` - 旋转和正弦运动
- `AuraProjectile.cpp` - 投射物更新
- `BTService_FindNearestPlayer.cpp` - AI 查找玩家

---

### 1.2 对象池系统

**问题**: 投射物、效果 Actor 等频繁创建和销毁，导致内存分配开销。

**影响**:
- 频繁的内存分配/释放
- GC 压力增加
- 性能下降

**优化建议**:

#### 1.2.1 投射物对象池

```cpp
// ✅ 创建对象池管理器
UCLASS()
class AURA_API AProjectilePoolManager : public AActor
{
    GENERATED_BODY()
    
public:
    AAuraProjectile* GetPooledProjectile(TSubclassOf<AAuraProjectile> ProjectileClass);
    void ReturnProjectileToPool(AAuraProjectile* Projectile);
    
private:
    UPROPERTY()
    TMap<TSubclassOf<AAuraProjectile>, TArray<AAuraProjectile*>> ProjectilePools;
    
    UPROPERTY()
    TMap<TSubclassOf<AAuraProjectile>, int32> PoolSizes;
};

// ✅ 在技能中使用
void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation, 
    const FGameplayTag& SocketTag)
{
    // 从对象池获取
    AAuraProjectile* Projectile = ProjectilePoolManager->GetPooledProjectile(ProjectileClass);
    
    if (Projectile)
    {
        Projectile->SetActorLocation(SpawnLocation);
        Projectile->SetActorRotation(SpawnRotation);
        Projectile->SetActive(true);
    }
}
```

#### 1.2.2 效果 Actor 对象池

```cpp
// ✅ 效果 Actor 对象池
class AURA_API AEffectActorPoolManager : public AActor
{
    GENERATED_BODY()
    
public:
    AAuraEffectActor* GetPooledEffectActor(TSubclassOf<AAuraEffectActor> EffectActorClass);
    void ReturnEffectActorToPool(AAuraEffectActor* EffectActor);
};
```

**优化文件**:
- `AuraProjectileSpell.cpp` - 投射物生成
- `AuraFireBlast.cpp` - 多投射物生成
- `AuraEffectActor.cpp` - 效果 Actor 生成

---

### 1.3 能力查找优化

**问题**: `AuraAbilitySystemComponent` 中多次使用线性搜索查找能力。

**影响**:
- O(n) 时间复杂度
- 频繁查找时性能下降

**优化建议**:

#### 1.3.1 使用 Map 缓存

```cpp
// ✅ 在 AuraAbilitySystemComponent 中添加缓存
class UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
private:
    // 缓存映射
    UPROPERTY()
    TMap<FGameplayTag, FGameplayAbilitySpecHandle> AbilityTagToHandleMap;
    
    UPROPERTY()
    TMap<FGameplayTag, FGameplayAbilitySpecHandle> InputTagToHandleMap;
    
    UPROPERTY()
    TMap<FGameplayTag, FGameplayAbilitySpecHandle> SlotToHandleMap;
    
public:
    // ✅ 优化后的查找方法
    FGameplayAbilitySpec* GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
    {
        // O(1) 查找
        if (FGameplayAbilitySpecHandle* Handle = AbilityTagToHandleMap.Find(AbilityTag))
        {
            return FindAbilitySpecFromHandle(*Handle);
        }
        return nullptr;
    }
    
    // ✅ 在 GiveAbility 时更新缓存
    virtual FGameplayAbilitySpecHandle GiveAbility(const FGameplayAbilitySpec& Spec) override
    {
        FGameplayAbilitySpecHandle Handle = Super::GiveAbility(Spec);
        
        // 更新缓存
        FGameplayTag AbilityTag = GetAbilityTagFromSpec(Spec);
        if (AbilityTag.IsValid())
        {
            AbilityTagToHandleMap.Add(AbilityTag, Handle);
        }
        
        return Handle;
    }
};
```

**优化文件**:
- `AuraAbilitySystemComponent.cpp` - 所有能力查找方法

---

### 1.4 GetWorld() 调用优化

**问题**: 代码中大量使用 `GetWorld()`，22 次调用。

**影响**:
- 每次调用都有开销
- 可能返回 nullptr

**优化建议**:

#### 1.4.1 缓存 World 指针

```cpp
// ✅ 在类中缓存
class UAuraAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
private:
    static UWorld* CachedWorld;
    
public:
    static UWorld* GetCachedWorld(const UObject* WorldContextObject)
    {
        if (!CachedWorld || !IsValid(CachedWorld))
        {
            CachedWorld = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
        }
        return CachedWorld;
    }
};
```

#### 1.4.2 使用参数传递

```cpp
// ✅ 在函数参数中传递 World
void SomeFunction(UWorld* World)
{
    // 直接使用，无需调用 GetWorld()
}
```

**优化文件**:
- `AuraAbilitySystemLibrary.cpp` - 所有静态函数
- `AuraGameModeBase.cpp` - GameMode 方法

---

## 2. 代码结构优化

### 2.1 伤害计算优化

**问题**: `ExecCalc_Damage` 中使用循环查找伤害类型和抗性。

**影响**:
- 每次伤害计算都要遍历
- 性能开销

**优化建议**:

#### 2.1.1 使用 Map 替代循环

```cpp
// ✅ 优化后的伤害计算
void UExecCalc_Damage::Execute_Implementation(...)
{
    // 使用 Map 直接查找，O(1)
    const FGameplayTag& DamageType = GetDamageType(Spec);
    const FGameplayTag& ResistanceTag = DamageTypeToResistanceMap[DamageType];
    
    // 直接获取抗性定义
    const FGameplayEffectAttributeCaptureDefinition& ResistanceDef = 
        ResistanceTagToDefMap[ResistanceTag];
    
    // 计算抗性
    float Resistance = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
        ResistanceDef, EvaluationParameters, Resistance
    );
}
```

**优化文件**:
- `ExecCalc_Damage.cpp` - DetermineDebuff 和 Execute 方法

---

### 2.2 委托绑定优化

**问题**: 委托绑定可能没有及时清理，导致内存泄漏。

**影响**:
- 内存泄漏
- 悬空指针

**优化建议**:

#### 2.2.1 使用弱引用

```cpp
// ✅ 使用弱引用避免循环引用
void UAuraWidgetController::BindCallbacksToDependencies()
{
    // 使用 AddWeakLambda
    AttributeSet->OnHealthChanged.AddWeakLambda(
        this,
        [this](const FOnAttributeChangeData& Data)
        {
            OnHealthChanged.Broadcast(Data.NewValue);
        }
    );
}
```

#### 2.2.2 及时清理委托

```cpp
// ✅ 在析构或 EndPlay 时清理
void UAuraWidgetController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    // 清理所有委托绑定
    if (AttributeSet)
    {
        AttributeSet->OnHealthChanged.RemoveAll(this);
        AttributeSet->OnManaChanged.RemoveAll(this);
        // ...
    }
    
    Super::EndPlay(EndPlayReason);
}
```

**优化文件**:
- 所有 WidgetController 类
- `AuraAbilitySystemComponent.cpp` - 委托绑定

---

### 2.3 字符串操作优化

**问题**: 技能描述生成时频繁进行字符串操作。

**影响**:
- 内存分配开销
- 性能下降

**优化建议**:

#### 2.3.1 使用 FStringBuilderBase

```cpp
// ✅ 使用 StringBuilder 优化字符串拼接
FString UAuraGameplayAbility::GetDescription(int32 Level)
{
    TStringBuilder<256> DescriptionBuilder;
    
    DescriptionBuilder.Append(TEXT("Damage: "));
    DescriptionBuilder.Append(FString::SanitizeFloat(GetDamage(Level)));
    DescriptionBuilder.Append(TEXT("\nCost: "));
    DescriptionBuilder.Append(FString::SanitizeFloat(GetManaCost(Level)));
    
    return FString(DescriptionBuilder);
}
```

#### 2.3.2 缓存格式化字符串

```cpp
// ✅ 缓存格式化后的字符串
class UAuraGameplayAbility
{
private:
    mutable FString CachedDescription;
    mutable int32 CachedDescriptionLevel = -1;
    
public:
    FString GetDescription(int32 Level)
    {
        if (CachedDescriptionLevel == Level && !CachedDescription.IsEmpty())
        {
            return CachedDescription;
        }
        
        // 生成新描述
        CachedDescription = GenerateDescription(Level);
        CachedDescriptionLevel = Level;
        
        return CachedDescription;
    }
};
```

**优化文件**:
- 所有 Ability 类的 `GetDescription` 方法

---

## 3. 网络优化

### 3.1 RPC 合并

**问题**: 某些情况下多次调用 RPC，可以合并。

**影响**:
- 网络带宽浪费
- 延迟增加

**优化建议**:

#### 3.1.1 批量更新

```cpp
// ✅ 合并多个属性更新
UFUNCTION(Server, Reliable)
void ServerUpdateMultipleAttributes(const TArray<FAttributeUpdate>& Updates);

struct FAttributeUpdate
{
    FGameplayTag AttributeTag;
    float NewValue;
};
```

#### 3.1.2 使用结构体传递数据

```cpp
// ✅ 使用结构体减少 RPC 调用
UFUNCTION(Server, Reliable)
void ServerEquipAbilities(const TArray<FAbilityEquipData>& EquipData);

struct FAbilityEquipData
{
    FGameplayTag AbilityTag;
    FGameplayTag Slot;
    FGameplayTag Status;
};
```

**优化文件**:
- `AuraAbilitySystemComponent.cpp` - 能力装备和升级

---

### 3.2 复制频率优化

**问题**: 某些属性可能复制过于频繁。

**影响**:
- 网络带宽浪费
- 性能下降

**优化建议**:

#### 3.2.1 使用条件复制

```cpp
// ✅ 只在变化超过阈值时复制
void AAuraCharacterBase::GetLifetimeReplicatedProps(...) const
{
    // 使用 COND_OwnerOnly 减少复制
    DOREPLIFETIME_CONDITION(AAuraCharacterBase, bIsStunned, COND_OwnerOnly);
    
    // 使用 COND_SkipOwner 跳过拥有者
    DOREPLIFETIME_CONDITION(AAuraCharacterBase, bIsBurned, COND_SkipOwner);
}
```

#### 3.2.2 使用 NotifyOnRep

```cpp
// ✅ 使用 NotifyOnRep 减少不必要的更新
UPROPERTY(ReplicatedUsing=OnRep_Level, NotifyOnRep)
int32 Level = 1;

// 只在客户端需要时更新
void AAuraPlayerState::OnRep_Level(int32 OldLevel)
{
    // 只在真正变化时更新 UI
    if (Level != OldLevel)
    {
        OnLevelChangedDelegate.Broadcast(Level, Level > OldLevel);
    }
}
```

**优化文件**:
- `AuraCharacterBase.cpp` - 状态标志复制
- `AuraPlayerState.cpp` - 玩家进度复制

---

## 4. 内存优化

### 4.1 临时对象重用

**问题**: 频繁创建临时对象（如 TArray、TMap）。

**影响**:
- 内存分配开销
- GC 压力

**优化建议**:

#### 4.1.1 重用容器

```cpp
// ✅ 在类中缓存容器
class UAuraAbilitySystemComponent
{
private:
    // 重用临时数组
    mutable TArray<FGameplayAbilitySpec> TempAbilitySpecs;
    
public:
    void ForEachAbility(const FForEachAbility& Delegate)
    {
        // 重用数组，避免每次分配
        TempAbilitySpecs.Reset();
        TempAbilitySpecs.Append(GetActivatableAbilities());
        
        for (const FGameplayAbilitySpec& Spec : TempAbilitySpecs)
        {
            Delegate.ExecuteIfBound(Spec);
        }
    }
};
```

#### 4.1.2 使用对象池

```cpp
// ✅ 对于频繁创建的对象使用对象池
class AProjectilePool
{
private:
    TArray<AAuraProjectile*> Pool;
    
public:
    AAuraProjectile* Acquire()
    {
        if (Pool.Num() > 0)
        {
            AAuraProjectile* Projectile = Pool.Pop();
            Projectile->Reset();
            return Projectile;
        }
        return NewObject<AAuraProjectile>();
    }
    
    void Release(AAuraProjectile* Projectile)
    {
        Projectile->Deactivate();
        Pool.Add(Projectile);
    }
};
```

**优化文件**:
- `AuraAbilitySystemComponent.cpp` - 临时数组
- `ExecCalc_Damage.cpp` - 临时容器

---

### 4.2 数据结构优化

**问题**: 某些数据结构可能不是最优选择。

**优化建议**:

#### 4.2.1 使用 TSet 替代 TArray（查找场景）

```cpp
// ✅ 如果需要频繁查找，使用 TSet
TSet<FGameplayTag> ActiveAbilityTags;  // O(1) 查找

// ❌ 而不是
TArray<FGameplayTag> ActiveAbilityTags;  // O(n) 查找
```

#### 4.2.2 使用 TMap 替代多个变量

```cpp
// ✅ 使用 Map 管理多个相关数据
TMap<FGameplayTag, FAbilityData> AbilityDataMap;

// ❌ 而不是多个独立变量
FGameplayTag AbilityTag1;
FGameplayTag AbilityTag2;
// ...
```

**优化文件**:
- `AuraAbilitySystemComponent.cpp` - 能力管理
- `AuraAttributeSet.cpp` - 属性管理

---

## 5. 架构设计优化

### 5.1 单一职责原则

**问题**: 某些类承担过多职责。

**优化建议**:

#### 5.1.1 拆分大型类

```cpp
// ✅ 将 AuraAbilitySystemComponent 拆分为多个职责
class UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
    // 只负责能力激活和管理
};

class UAuraAbilityManager
{
    // 负责能力查找和缓存
};

class UAuraAbilityStatusManager
{
    // 负责能力状态管理
};
```

---

### 5.2 依赖注入

**问题**: 某些类直接依赖全局对象。

**优化建议**:

#### 5.2.1 使用依赖注入

```cpp
// ✅ 通过构造函数注入依赖
class UAuraWidgetController
{
public:
    UAuraWidgetController(const FWidgetControllerParams& Params)
        : PlayerController(Params.PlayerController)
        , PlayerState(Params.PlayerState)
        , AbilitySystemComponent(Params.AbilitySystemComponent)
        , AttributeSet(Params.AttributeSet)
    {
    }
};
```

---

## 6. 实施优先级

### 高优先级（立即实施）

1. **Tick 优化** - 影响最大，实施简单
2. **能力查找优化** - 频繁调用，影响明显
3. **对象池系统** - 对性能提升显著

### 中优先级（近期实施）

1. **GetWorld() 优化** - 减少调用开销
2. **委托绑定优化** - 防止内存泄漏
3. **网络优化** - 提升多人游戏性能

### 低优先级（长期优化）

1. **代码结构重构** - 需要更多测试
2. **架构设计优化** - 需要规划

---

## 7. 性能测试建议

### 7.1 基准测试

在实施优化前后进行性能测试：

```cpp
// ✅ 性能测试工具
class AURA_API APerformanceProfiler : public AActor
{
public:
    void StartProfiling();
    void StopProfiling();
    void LogResults();
};
```

### 7.2 性能指标

监控以下指标：
- **帧率** (FPS)
- **内存使用** (MB)
- **网络带宽** (KB/s)
- **CPU 使用率** (%)
- **GC 频率** (次/秒)

---

## 8. 总结

通过实施这些优化建议，预期可以获得：

1. **性能提升**: 20-30% 帧率提升
2. **内存优化**: 减少 15-25% 内存使用
3. **网络优化**: 减少 30-40% 网络带宽
4. **代码质量**: 更易维护和扩展

建议按照优先级逐步实施，并在每个阶段进行测试验证。

---

## 相关文档

- [架构文档](./Core/Architecture.md) - 当前架构
- [系统设计](./Core/System_Design.md) - 系统设计
- [性能考虑](./Core/Architecture.md#9-性能考虑) - 现有性能考虑

