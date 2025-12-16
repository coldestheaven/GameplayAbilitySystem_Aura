# 远程攻击技能实现指南

## 概述

本文档详细说明如何实现一个远程攻击技能（投射物技能）。远程技能发射投射物，对目标造成伤害，支持追踪、多投射物、扇形分布等功能。

### 技能特点

- ✅ 发射投射物攻击远程目标
- ✅ 支持追踪目标（Homing）
- ✅ 支持多个投射物
- ✅ 支持扇形分布
- ✅ 碰撞时造成伤害
- ✅ 支持各种伤害类型（火焰、冰霜、闪电等）

---

## 实现步骤概览

```
1. 创建投射物技能 C++ 类
   ↓
2. 创建投射物 Actor 类（如需要）
   ↓
3. 添加 GameplayTag
   ↓
4. 创建投射物蓝图
   ↓
5. 创建技能蓝图
   ↓
6. 创建 GameplayEffect（Cost、Cooldown、Damage）
   ↓
7. 配置 AbilityInfo
   ↓
8. 测试
```

---

## 步骤 1: 创建投射物技能 C++ 类

### 1.1 创建头文件

在 `Source/Aura/Public/AbilitySystem/Abilities/` 目录下创建 `AuraIceBolt.h`：

```cpp
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "AuraIceBolt.generated.h"

/**
 * 冰霜箭技能
 * 发射冰霜箭投射物，造成冰霜伤害，有几率减速敌人
 */
UCLASS()
class AURA_API UAuraIceBolt : public UAuraProjectileSpell
{
    GENERATED_BODY()
    
public:
    UAuraIceBolt();
    
    virtual FString GetDescription(int32 Level) override;
    virtual FString GetNextLevelDescription(int32 Level) override;
    
protected:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;
    
    // 生成多个投射物（重写基类方法）
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    void SpawnProjectiles(
        const FVector& ProjectileTargetLocation,
        const FGameplayTag& SocketTag,
        bool bOverridePitch = false,
        float PitchOverride = 0.f,
        AActor* HomingTarget = nullptr
    );
    
    // 投射物散布角度
    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    float ProjectileSpread = 45.f;
    
    // 最大投射物数量
    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    int32 MaxNumProjectiles = 3;
    
    // 是否启用追踪
    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    bool bLaunchHomingProjectiles = true;
    
    // 追踪加速度范围
    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    float HomingAccelerationMin = 1600.f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Projectile")
    float HomingAccelerationMax = 3200.f;
};
```

### 1.2 创建实现文件

在 `Source/Aura/Private/AbilitySystem/Abilities/` 目录下创建 `AuraIceBolt.cpp`：

```cpp
#include "AbilitySystem/Abilities/AuraIceBolt.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Actor/AuraProjectile.h"
#include "AuraGameplayTags.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Interaction/CombatInterface.h"

UAuraIceBolt::UAuraIceBolt()
{
    const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
    AbilityTags.AddTag(Tags.Abilities_Ice_IceBolt);
    StartupInputTag = Tags.InputTag_RMB;
}

FString UAuraIceBolt::GetDescription(int32 Level)
{
    const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
    const float ManaCost = FMath::Abs(GetManaCost(Level));
    const float Cooldown = GetCooldown(Level);
    const int32 NumBolts = FMath::Min(Level, MaxNumProjectiles);
    
    return FString::Printf(
        TEXT(
            "<Title>ICE BOLT</>\n\n"
            "<Small>Level: </><Level>%d</>\n"
            "<Small>ManaCost: </><ManaCost>%.1f</>\n"
            "<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
            "<Default>Launches %d bolt(s) of ice, "
            "dealing </>"
            "<Damage>%d</>"
            "<Default> ice damage with a chance to slow enemies.</>"
        ),
        Level,
        ManaCost,
        Cooldown,
        NumBolts,
        ScaledDamage
    );
}

FString UAuraIceBolt::GetNextLevelDescription(int32 Level)
{
    const int32 NextDamage = Damage.GetValueAtLevel(Level + 1);
    const int32 CurrentDamage = Damage.GetValueAtLevel(Level);
    const int32 DamageIncrease = NextDamage - CurrentDamage;
    const int32 NextNumBolts = FMath::Min(Level + 1, MaxNumProjectiles);
    
    return FString::Printf(
        TEXT(
            "<Title>NEXT LEVEL: </>\n\n"
            "<Small>Level: </><Level>%d</>\n\n"
            "<Default>Launches %d bolt(s), dealing </>"
            "<Damage>%d</>"
            "<Default> (+%d) ice damage.</>"
        ),
        Level + 1,
        NextNumBolts,
        NextDamage,
        DamageIncrease
    );
}

void UAuraIceBolt::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData
)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    // 获取目标位置（从鼠标或目标）
    FVector TargetLocation = GetAvatarActorFromActorInfo()->GetActorLocation() + 
        GetAvatarActorFromActorInfo()->GetActorForwardVector() * 1000.f;
    
    // 可以从 CombatInterface 获取目标位置
    // 或从鼠标位置获取
    
    // 生成投射物
    const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
    SpawnProjectiles(
        TargetLocation,
        Tags.CombatSocket_RightHand,
        false,  // bOverridePitch
        0.f,    // PitchOverride
        nullptr // HomingTarget
    );
    
    // 结束技能
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UAuraIceBolt::SpawnProjectiles(
    const FVector& ProjectileTargetLocation,
    const FGameplayTag& SocketTag,
    bool bOverridePitch,
    float PitchOverride,
    AActor* HomingTarget
)
{
    const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
    if (!bIsServer || !ProjectileClass)
    {
        return;
    }
    
    // 获取发射位置
    const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
        GetAvatarActorFromActorInfo(),
        SocketTag
    );
    
    // 计算方向
    FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
    if (bOverridePitch)
    {
        Rotation.Pitch = PitchOverride;
    }
    
    // 计算投射物数量（受等级限制）
    const int32 EffectiveNumProjectiles = FMath::Min(MaxNumProjectiles, GetAbilityLevel());
    
    // 如果只有一个投射物，直接生成
    if (EffectiveNumProjectiles == 1)
    {
        SpawnProjectile(ProjectileTargetLocation, SocketTag, bOverridePitch, PitchOverride);
        return;
    }
    
    // 多个投射物：计算扇形分布
    const FVector Forward = Rotation.Vector();
    TArray<FRotator> Rotations = UAuraAbilitySystemLibrary::EvenlySpacedRotators(
        Forward,
        FVector::UpVector,
        ProjectileSpread,
        EffectiveNumProjectiles
    );
    
    // 生成每个投射物
    for (const FRotator& Rot : Rotations)
    {
        FTransform SpawnTransform;
        SpawnTransform.SetLocation(SocketLocation);
        SpawnTransform.SetRotation(Rot.Quaternion());
        
        AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
            ProjectileClass,
            SpawnTransform,
            GetOwningActorFromActorInfo(),
            Cast<APawn>(GetOwningActorFromActorInfo()),
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn
        );
        
        if (Projectile)
        {
            // 设置伤害参数
            Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
            
            // 设置追踪目标
            if (bLaunchHomingProjectiles)
            {
                if (HomingTarget && HomingTarget->Implements<UCombatInterface>())
                {
                    Projectile->ProjectileMovement->HomingTargetComponent = 
                        HomingTarget->GetRootComponent();
                }
                else
                {
                    // 创建临时场景组件作为追踪目标
                    Projectile->HomingTargetSceneComponent = NewObject<USceneComponent>(
                        USceneComponent::StaticClass()
                    );
                    Projectile->HomingTargetSceneComponent->SetWorldLocation(ProjectileTargetLocation);
                    Projectile->ProjectileMovement->HomingTargetComponent = 
                        Projectile->HomingTargetSceneComponent;
                }
                
                // 设置追踪加速度（随机值）
                Projectile->ProjectileMovement->HomingAccelerationMagnitude = 
                    FMath::FRandRange(HomingAccelerationMin, HomingAccelerationMax);
                
                Projectile->ProjectileMovement->bIsHomingProjectile = true;
            }
            
            // 完成生成
            Projectile->FinishSpawning(SpawnTransform);
        }
    }
}
```

---

## 步骤 2: 创建投射物 Actor 类（可选）

如果不需要自定义逻辑，可以直接使用 `AAuraProjectile`。如果需要自定义，可以创建子类：

### 2.1 创建头文件

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Actor/AuraProjectile.h"
#include "AuraIceBoltProjectile.generated.h"

/**
 * 冰霜箭投射物
 */
UCLASS()
class AURA_API AAuraIceBoltProjectile : public AAuraProjectile
{
    GENERATED_BODY()
    
public:
    AAuraIceBoltProjectile();
    
protected:
    virtual void OnSphereOverlap(...) override;
    
    // 减速效果参数
    UPROPERTY(EditDefaultsOnly, Category = "IceBolt")
    float SlowPercentage = 0.3f;
    
    UPROPERTY(EditDefaultsOnly, Category = "IceBolt")
    float SlowDuration = 3.f;
};
```

### 2.2 创建实现文件

```cpp
#include "Actor/AuraIceBoltProjectile.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"

AAuraIceBoltProjectile::AAuraIceBoltProjectile()
{
    // 设置投射物特定参数
}

void AAuraIceBoltProjectile::OnSphereOverlap(...)
{
    Super::OnSphereOverlap(...);
    
    // 可以在这里添加额外的逻辑
    // 例如：应用减速效果
}
```

---

## 步骤 3: 添加 GameplayTag

### 3.1 在 AuraGameplayTags.h 中添加

```cpp
struct FAuraGameplayTags
{
    // ... 现有 Tags ...
    
    // 冰霜箭技能 Tag
    FGameplayTag Abilities_Ice_IceBolt;
    
    // 冷却 Tag
    FGameplayTag Cooldown_Ice_IceBolt;
};
```

### 3.2 在 AuraGameplayTags.cpp 中初始化

```cpp
void FAuraGameplayTags::InitializeNativeGameplayTags()
{
    // ... 现有初始化 ...
    
    GameplayTags.Abilities_Ice_IceBolt = UGameplayTagsManager::Get()
        .AddNativeGameplayTag(
            FName("Abilities.Ice.IceBolt"),
            FString("Ice Bolt projectile ability")
        );
    
    GameplayTags.Cooldown_Ice_IceBolt = UGameplayTagsManager::Get()
        .AddNativeGameplayTag(
            FName("Cooldown.Ice.IceBolt"),
            FString("Ice Bolt cooldown tag")
        );
}
```

---

## 步骤 4: 创建投射物蓝图

### 4.1 创建蓝图类

1. 基于 `BP_Projectile`（或 `AAuraProjectile`）创建蓝图
2. 命名为 `BP_Projectile_IceBolt`
3. 保存到 `Content/Blueprints/Actor/Projectiles/`

### 4.2 配置投射物

1. **碰撞设置**
   - **Sphere Component**:
     - **Collision Object Type**: `Projectile`
     - **Collision Enabled**: `QueryOnly`
     - **Collision Responses**:
       - `WorldDynamic`: Overlap
       - `WorldStatic`: Overlap
       - `Pawn`: Overlap

2. **移动设置**
   - **Projectile Movement**:
     - **Initial Speed**: 550
     - **Max Speed**: 550
     - **Projectile Gravity Scale**: 0（无重力）

3. **视觉效果**
   - **Impact Effect**: 冰霜碰撞特效（Niagara）
   - **Impact Sound**: 冰霜碰撞音效
   - **Looping Sound**: 飞行时的循环音效

4. **生命周期**
   - **Life Span**: 15.0（15秒后自动销毁）

---

## 步骤 5: 创建技能蓝图

### 5.1 创建蓝图类

1. 在编辑器中，右键点击 `AuraIceBolt` C++ 类
2. 选择 "Create Blueprint class based on AuraIceBolt"
3. 命名为 `BP_GA_IceBolt`
4. 保存到 `Content/Blueprints/AbilitySystem/Abilities/Ice/`

### 5.2 配置技能参数

1. **基础设置**
   - **Ability Tags**: 添加 `Abilities.Ice.IceBolt`
   - **Startup Input Tag**: `InputTag.RMB`

2. **投射物设置**
   - **Projectile Class**: `BP_Projectile_IceBolt`
   - **Max Num Projectiles**: 3
   - **Projectile Spread**: 45.0（45度扇形）
   - **Launch Homing Projectiles**: true
   - **Homing Acceleration Min**: 1600.0
   - **Homing Acceleration Max**: 3200.0

3. **伤害设置**
   - **Damage**: 配置曲线表（Level 1: 50, Level 2: 75, ...）
   - **Damage Type**: `Damage.Ice`
   - **Damage Effect Class**: `GE_IceBolt_Damage`
   - **Debuff Chance**: 25.0（25% 几率减速）
   - **Debuff Damage**: 5.0
   - **Debuff Duration**: 3.0
   - **Debuff Frequency**: 1.0

---

## 步骤 6: 创建 GameplayEffect

### 6.1 创建 Cost GameplayEffect

1. 创建 `GE_IceBolt_Cost`
2. 设置：
   - **Duration Policy**: `Instant`
   - **Modifier**: `Mana`, `Subtract`, `SetByCaller(Data.ManaCost)`

### 6.2 创建 Cooldown GameplayEffect

1. 创建 `GE_IceBolt_Cooldown`
2. 设置：
   - **Duration Policy**: `HasDuration`
   - **Duration Magnitude**: 3.0（3秒冷却）
   - **Granted Tags**: `Cooldown.Ice.IceBolt`
   - **Ignore Tags**: `Cooldown.Ice.IceBolt`

### 6.3 创建 Damage GameplayEffect

1. 创建 `GE_IceBolt_Damage`
2. 设置：
   - **Duration Policy**: `Instant`
   - **Execution Calculation**: `ExecCalc_Damage`
   - **Modifier**: `IncomingDamage`, `Additive`, `SetByCaller(Damage.Ice)`

---

## 步骤 7: 配置 AbilityInfo

在 `DA_AbilityInfo` 数据资产中添加：

- **Ability Tag**: `Abilities.Ice.IceBolt`
- **Input Tag**: `InputTag.RMB`
- **Status Tag**: `Abilities.Status.Locked`
- **Cooldown Tag**: `Cooldown.Ice.IceBolt`
- **Ability Type**: `Abilities.Type.Offensive`
- **Icon**: 冰霜箭技能图标
- **Level Requirement**: 5
- **Ability**: `BP_GA_IceBolt`

---

## 步骤 8: 优化和改进

### 8.1 改进目标位置获取

在技能蓝图中实现更精确的目标位置获取：

```cpp
// 在蓝图中或 C++ 中
void UAuraIceBolt::ActivateAbility(...)
{
    Super::ActivateAbility(...);
    
    // 从鼠标位置获取目标
    FHitResult HitResult;
    GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
    
    FVector TargetLocation;
    if (HitResult.bBlockingHit)
    {
        TargetLocation = HitResult.ImpactPoint;
    }
    else
    {
        // 使用默认位置
        TargetLocation = GetAvatarActorFromActorInfo()->GetActorLocation() + 
            GetAvatarActorFromActorInfo()->GetActorForwardVector() * 1000.f;
    }
    
    // 生成投射物
    SpawnProjectiles(TargetLocation, ...);
}
```

### 8.2 支持锁定目标

可以添加目标锁定功能：

```cpp
// 在技能类中添加
UPROPERTY(EditDefaultsOnly, Category = "Targeting")
float LockRange = 1000.f;

// 获取锁定目标
AActor* GetLockedTarget()
{
    // 检测范围内的敌人
    // 返回最近的敌人
}
```

### 8.3 添加穿透效果

可以添加穿透多个目标的功能：

```cpp
// 在投射物类中
UPROPERTY(EditDefaultsOnly, Category = "Projectile")
int32 MaxPenetrations = 0;  // 0 = 不穿透

UPROPERTY()
int32 CurrentPenetrations = 0;

// 在碰撞检测中
void AAuraIceBoltProjectile::OnSphereOverlap(...)
{
    // 造成伤害
    // ...
    
    // 检查是否可以穿透
    if (CurrentPenetrations < MaxPenetrations)
    {
        CurrentPenetrations++;
        // 继续飞行
    }
    else
    {
        // 销毁投射物
        OnHit();
    }
}
```

---

## 测试清单

- ✅ 技能可以激活
- ✅ 投射物正确生成
- ✅ 投射物移动正常
- ✅ 投射物碰撞检测正确
- ✅ 对敌人造成伤害
- ✅ 友方不受伤害
- ✅ 追踪功能正常（如启用）
- ✅ 多个投射物正确分布
- ✅ 视觉效果正确
- ✅ 音效正确
- ✅ 冷却时间正确
- ✅ 法力消耗正确

---

## 常见问题

### Q1: 投射物不生成

**可能原因**:
- `ProjectileClass` 未设置
- 不在服务器上执行
- 生成位置无效

**解决方法**:
- 检查 `ProjectileClass` 是否在技能蓝图中设置
- 确保 `SpawnProjectiles` 在服务器上执行
- 检查 Socket 位置是否正确

### Q2: 投射物不追踪目标

**可能原因**:
- `bLaunchHomingProjectiles` 未启用
- `HomingTargetComponent` 未设置
- 追踪加速度为 0

**解决方法**:
- 检查 `bLaunchHomingProjectiles` 是否为 true
- 确保 `HomingTargetComponent` 已设置
- 检查 `HomingAccelerationMin/Max` 值

### Q3: 多个投射物分布不正确

**可能原因**:
- `ProjectileSpread` 设置错误
- `EvenlySpacedRotators` 计算错误

**解决方法**:
- 检查 `ProjectileSpread` 值（建议 30-90 度）
- 确保使用 `UAuraAbilitySystemLibrary::EvenlySpacedRotators`

### Q4: 投射物不造成伤害

**可能原因**:
- `DamageEffectParams` 未设置
- `DamageEffectClass` 未配置
- SetByCaller 标签不匹配

**解决方法**:
- 确保在生成投射物时设置 `DamageEffectParams`
- 检查 `DamageEffectClass` 是否配置
- 确保 SetByCaller 标签为 `Damage.Ice`（或对应伤害类型）

---

## 总结

实现远程攻击技能需要：

1. ✅ **创建技能类** - 继承自 `UAuraProjectileSpell`
2. ✅ **创建投射物类** - 基于 `AAuraProjectile`
3. ✅ **配置投射物参数** - 速度、追踪、视觉效果
4. ✅ **实现多投射物** - 扇形分布
5. ✅ **创建 GameplayEffect** - Cost、Cooldown、Damage
6. ✅ **配置 AbilityInfo** - 技能信息
7. ✅ **测试** - 全面测试所有功能

遵循这些步骤，可以成功实现一个功能完整的远程攻击技能。

