# Actor 系统详细文档

## 目录

1. [系统概述](#系统概述)
2. [效果 Actor](#效果-actor)
3. [敌人生成系统](#敌人生成系统)
4. [投射物系统](#投射物系统)
5. [配置指南](#配置指南)
6. [使用示例](#使用示例)

---

## 系统概述

Actor 系统提供了游戏世界中各种交互对象的实现，包括效果应用、敌人生成、投射物等。

### 核心组件

- **AAuraEffectActor**: 应用 GameplayEffect 的效果 Actor
- **AAuraEnemySpawnVolume**: 敌人生成体积
- **AAuraEnemySpawnPoint**: 敌人生成点
- **AAuraProjectile**: 投射物 Actor

### 系统特点

- ✅ 效果应用系统
- ✅ 敌人生成系统
- ✅ 投射物系统
- ✅ 动画和视觉效果
- ✅ 存档集成

---

## 效果 Actor

### AAuraEffectActor

效果 Actor 用于在玩家与对象重叠时应用 GameplayEffect。

#### 核心功能

- **效果应用**: 在重叠时应用 GameplayEffect
- **效果移除**: 在重叠结束时移除效果
- **动画效果**: 旋转和正弦运动
- **目标过滤**: 可选择是否对敌人应用效果

#### 效果应用策略

```cpp
enum class EEffectApplicationPolicy : uint8
{
    ApplyOnOverlap,      // 重叠时应用
    ApplyOnEndOverlap,   // 重叠结束时应用
    DoNotApply           // 不应用
};
```

#### 效果移除策略

```cpp
enum class EEffectRemovalPolicy : uint8
{
    RemoveOnEndOverlap,  // 重叠结束时移除
    DoNotRemove          // 不移除
};
```

#### 效果类型

- **InstantGameplayEffectClass**: 即时效果
- **DurationGameplayEffectClass**: 持续效果
- **InfiniteGameplayEffectClass**: 无限效果

#### 应用效果

```cpp
void AAuraEffectActor::ApplyEffectToTarget(
    AActor* TargetActor, 
    TSubclassOf<UGameplayEffect> GameplayEffectClass
)
{
    // 检查是否对敌人应用
    if (TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies) 
        return;
    
    // 获取目标 ASC
    UAbilitySystemComponent* TargetASC = 
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
    if (TargetASC == nullptr) return;
    
    // 创建效果上下文
    FGameplayEffectContextHandle EffectContextHandle = TargetASC->MakeEffectContext();
    EffectContextHandle.AddSourceObject(this);
    
    // 创建效果 Spec
    const FGameplayEffectSpecHandle EffectSpecHandle = 
        TargetASC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContextHandle);
    
    // 应用效果
    const FActiveGameplayEffectHandle ActiveEffectHandle = 
        TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpecHandle.Data.Get());
    
    // 如果是无限效果且需要移除，保存句柄
    const bool bIsInfinite = EffectSpecHandle.Data.Get()->Def.Get()->DurationPolicy 
        == EGameplayEffectDurationType::Infinite;
    if (bIsInfinite && InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
    {
        ActiveEffectHandles.Add(ActiveEffectHandle, TargetASC);
    }
    
    // 如果非无限效果，销毁 Actor
    if (!bIsInfinite && bDestroyOnEffectApplication)
    {
        Destroy();
    }
}
```

#### 重叠处理

```cpp
void AAuraEffectActor::OnOverlap(AActor* TargetActor)
{
    if (TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies) 
        return;
    
    // 根据策略应用效果
    if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
    {
        ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
    }
    if (DurationEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
    {
        ApplyEffectToTarget(TargetActor, DurationGameplayEffectClass);
    }
    if (InfiniteEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnOverlap)
    {
        ApplyEffectToTarget(TargetActor, InfiniteGameplayEffectClass);
    }
}
```

#### 重叠结束处理

```cpp
void AAuraEffectActor::OnEndOverlap(AActor* TargetActor)
{
    if (TargetActor->ActorHasTag(FName("Enemy")) && !bApplyEffectsToEnemies) 
        return;
    
    // 根据策略应用效果
    if (InstantEffectApplicationPolicy == EEffectApplicationPolicy::ApplyOnEndOverlap)
    {
        ApplyEffectToTarget(TargetActor, InstantGameplayEffectClass);
    }
    // ... 其他效果类型
    
    // 移除无限效果
    if (InfiniteEffectRemovalPolicy == EEffectRemovalPolicy::RemoveOnEndOverlap)
    {
        UAbilitySystemComponent* TargetASC = 
            UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
        if (!IsValid(TargetASC)) return;
        
        // 移除保存的效果句柄
        TArray<FActiveGameplayEffectHandle> HandlesToRemove;
        for (TTuple<FActiveGameplayEffectHandle, UAbilitySystemComponent*> HandlePair : ActiveEffectHandles)
        {
            if (TargetASC == HandlePair.Value)
            {
                TargetASC->RemoveActiveGameplayEffect(HandlePair.Key, 1);
                HandlesToRemove.Add(HandlePair.Key);
            }
        }
        for (FActiveGameplayEffectHandle& Handle : HandlesToRemove)
        {
            ActiveEffectHandles.FindAndRemoveChecked(Handle);
        }
    }
}
```

#### 动画效果

##### 旋转

```cpp
void AAuraEffectActor::ItemMovement(float DeltaTime)
{
    if (bRotates)
    {
        const FRotator DeltaRotation(0.f, DeltaTime * RotationRate, 0.f);
        CalculatedRotation = UKismetMathLibrary::ComposeRotators(
            CalculatedRotation, 
            DeltaRotation
        );
    }
}
```

##### 正弦运动

```cpp
void AAuraEffectActor::ItemMovement(float DeltaTime)
{
    if (bSinusoidalMovement)
    {
        const float Sine = SineAmplitude * FMath::Sin(RunningTime * SinePeriodConstant);
        CalculatedLocation = InitialLocation + FVector(0.f, 0.f, Sine);
    }
}
```

---

## 敌人生成系统

### AAuraEnemySpawnVolume

敌人生成体积，当玩家进入时生成敌人。

#### 核心功能

- **触发生成**: 玩家进入体积时触发
- **生成点管理**: 管理多个生成点
- **存档集成**: 支持存档系统

#### 实现

```cpp
void AAuraEnemySpawnVolume::OnBoxOverlap(
    UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex, 
    bool bFromSweep, 
    const FHitResult& SweepResult
)
{
    // 检查是否为玩家
    if (!OtherActor->Implements<UPlayerInterface>()) return;
    
    // 标记为已到达
    bReached = true;
    
    // 在所有生成点生成敌人
    for (AAuraEnemySpawnPoint* Point : SpawnPoints)
    {
        if (IsValid(Point))
        {
            Point->SpawnEnemy();
        }
    }
    
    // 禁用碰撞（只生成一次）
    Box->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
```

#### 存档集成

```cpp
void AAuraEnemySpawnVolume::LoadActor_Implementation()
{
    // 如果已经到达，销毁体积
    if (bReached)
    {
        Destroy();
    }
}
```

### AAuraEnemySpawnPoint

敌人生成点，定义敌人的生成位置和属性。

#### 核心属性

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Class")
TSubclassOf<AAuraEnemy> EnemyClass;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Class")
int32 EnemyLevel = 1;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Class")
ECharacterClass CharacterClass = ECharacterClass::Warrior;
```

#### 生成敌人

```cpp
void AAuraEnemySpawnPoint::SpawnEnemy()
{
    // 设置生成参数
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = 
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    // 延迟生成（允许设置属性）
    AAuraEnemy* Enemy = GetWorld()->SpawnActorDeferred<AAuraEnemy>(
        EnemyClass, 
        GetActorTransform()
    );
    
    // 设置敌人属性
    Enemy->SetLevel(EnemyLevel);
    Enemy->SetCharacterClass(CharacterClass);
    
    // 完成生成
    Enemy->FinishSpawning(GetActorTransform());
    
    // 生成 AI 控制器
    Enemy->SpawnDefaultController();
}
```

---

## 投射物系统

### AAuraProjectile

投射物 Actor，用于技能投射物。

#### 核心组件

```cpp
UPROPERTY(VisibleAnywhere)
TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
TObjectPtr<USphereComponent> Sphere;

UPROPERTY()
TObjectPtr<UAudioComponent> LoopingSoundComponent;
```

#### 伤害参数

```cpp
UPROPERTY(BlueprintReadWrite, meta = (ExposeOnSpawn = true))
FDamageEffectParams DamageEffectParams;
```

#### 碰撞处理

```cpp
void AAuraProjectile::OnSphereOverlap(
    UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex, 
    bool bFromSweep, 
    const FHitResult& SweepResult
)
{
    // 检查是否为有效碰撞
    if (!IsValidOverlap(OtherActor)) return;
    
    // 处理碰撞
    OnHit();
}
```

#### 碰撞验证

```cpp
bool AAuraProjectile::IsValidOverlap(AActor* OtherActor)
{
    // 检查是否已经碰撞
    if (bHit) return false;
    
    // 检查是否为有效 Actor
    if (!IsValid(OtherActor)) return false;
    
    // 检查是否为自身
    if (OtherActor == GetOwner()) return false;
    
    // 检查是否实现了 CombatInterface
    if (!OtherActor->Implements<UCombatInterface>()) return false;
    
    // 检查是否已死亡
    if (ICombatInterface::Execute_IsDead(OtherActor)) return false;
    
    return true;
}
```

#### 碰撞处理

```cpp
void AAuraProjectile::OnHit()
{
    // 标记已碰撞
    bHit = true;
    
    // 停止移动
    if (ProjectileMovement)
    {
        ProjectileMovement->StopMovementImmediately();
    }
    
    // 应用伤害
    if (UAbilitySystemComponent* TargetASC = 
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
    {
        // 创建伤害效果
        // ... 应用伤害
    }
    
    // 播放效果
    if (ImpactEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), 
            ImpactEffect, 
            GetActorLocation()
        );
    }
    
    // 播放音效
    if (ImpactSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(), 
            ImpactSound, 
            GetActorLocation()
        );
    }
    
    // 销毁投射物
    Destroy();
}
```

---

## 配置指南

### 创建效果 Actor

#### 步骤 1: 创建效果 Actor 蓝图

1. 创建继承自 `BP_AuraEffectActor` 的蓝图
2. 设置网格和视觉效果

#### 步骤 2: 配置效果

在蓝图中设置：

- **InstantGameplayEffectClass**: 即时效果类
- **DurationGameplayEffectClass**: 持续效果类
- **InfiniteGameplayEffectClass**: 无限效果类
- **EffectApplicationPolicy**: 效果应用策略
- **EffectRemovalPolicy**: 效果移除策略

#### 步骤 3: 配置动画

- **bRotates**: 是否旋转
- **RotationRate**: 旋转速度
- **bSinusoidalMovement**: 是否正弦运动
- **SineAmplitude**: 正弦幅度
- **SinePeriodConstant**: 正弦周期常数

### 创建敌人生成系统

#### 步骤 1: 创建生成体积

1. 创建 `AAuraEnemySpawnVolume` 蓝图
2. 设置 Box 组件大小和位置

#### 步骤 2: 创建生成点

1. 创建 `AAuraEnemySpawnPoint` 蓝图
2. 设置生成点位置
3. 配置敌人属性：
   - **EnemyClass**: 敌人类
   - **EnemyLevel**: 敌人等级
   - **CharacterClass**: 职业类型

#### 步骤 3: 关联生成点

在生成体积中设置 `SpawnPoints` 数组，添加所有生成点。

---

## 使用示例

### 创建治疗药水

```cpp
// 在蓝图中配置
// InstantGameplayEffectClass: GE_Heal
// InstantEffectApplicationPolicy: ApplyOnOverlap
// bDestroyOnEffectApplication: true
// bRotates: true
// RotationRate: 90.f
```

### 创建增益光环

```cpp
// 在蓝图中配置
// InfiniteGameplayEffectClass: GE_Buff
// InfiniteEffectApplicationPolicy: ApplyOnOverlap
// InfiniteEffectRemovalPolicy: RemoveOnEndOverlap
// bSinusoidalMovement: true
// SineAmplitude: 20.f
```

### 创建敌人生成区域

```cpp
// 1. 创建生成体积
// 2. 创建多个生成点
// 3. 在生成体积中关联生成点
// 4. 配置每个生成点的敌人类型和等级
```

---

## 最佳实践

### 1. 效果 Actor

- **性能优化**: 使用对象池管理效果 Actor
- **效果设计**: 合理使用即时、持续、无限效果
- **动画效果**: 使用动画增强视觉效果

### 2. 敌人生成

- **生成时机**: 合理控制生成时机
- **生成数量**: 避免一次性生成过多敌人
- **存档集成**: 确保生成状态正确保存

### 3. 投射物

- **碰撞检测**: 正确设置碰撞通道
- **性能优化**: 使用对象池管理投射物
- **视觉效果**: 添加适当的视觉效果和音效

---

## 总结

Actor 系统提供了完整的游戏世界交互功能：

- ✅ **效果应用**: 灵活的效果应用系统
- ✅ **敌人生成**: 自动化的敌人生成系统
- ✅ **投射物**: 完整的投射物系统
- ✅ **动画效果**: 丰富的动画和视觉效果
- ✅ **存档集成**: 与存档系统集成

通过这个系统，开发者可以创建丰富的游戏世界交互体验。

