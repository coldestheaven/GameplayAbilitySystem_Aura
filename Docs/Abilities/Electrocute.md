# Electrocute (电击)

## 技能概述

**Electrocute** 是一个光束技能，发射闪电光束连接到目标，持续造成闪电伤害。光束可以连锁传播到多个目标，对每个目标造成伤害。有几率对目标施加眩晕（Stun）Debuff。

### 基本信息

- **技能类型**: 光束技能 (Beam Spell)
- **伤害类型**: 闪电 (Lightning)
- **基础类**: `UAuraBeamSpell`
- **技能标签**: `Abilities.Lightning.Electrocute`

### 技能特点

- ✅ 持续伤害光束
- ✅ 支持连锁传播（根据等级）
- ✅ 自动目标切换（目标死亡时）
- ✅ 有几率造成眩晕 Debuff
- ✅ 视觉效果震撼

---

## 技能描述

### 等级 1

```
ELECTROCUTE

Level: 1
ManaCost: [法力消耗]
Cooldown: [冷却时间]

Emits a beam of lightning, connecting with the target, repeatedly causing [伤害] lightning damage with a chance to stun
```

### 等级 2+

```
ELECTROCUTE

Level: [等级]
ManaCost: [法力消耗]
Cooldown: [冷却时间]

Emits a beam of lightning, propagating to [数量] additional targets nearby, causing [伤害] lightning damage with a chance to stun
```

### 下一级描述

```
NEXT LEVEL:

Level: [下一级等级]
ManaCost: [下一级法力消耗]
Cooldown: [下一级冷却时间]

Emits a beam of lightning, propagating to [下一级数量] additional targets nearby, causing [下一级伤害] lightning damage with a chance to stun
```

---

## 技能参数

### 基础参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `Damage` | FScalableFloat | - | 伤害值（可随等级缩放） |
| `DamageType` | FGameplayTag | `Damage_Lightning` | 伤害类型标签 |
| `DebuffChance` | float | 20.0 | 眩晕触发几率（百分比） |
| `DebuffDamage` | float | 5.0 | Debuff 伤害（如果适用） |
| `DebuffDuration` | float | 5.0 | 眩晕持续时间（秒） |
| `DebuffFrequency` | float | 1.0 | Debuff 伤害频率（秒） |

### 光束参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `MaxNumShockTargets` | int32 | 5 | 最大连锁目标数 |
| `MouseHitLocation` | FVector | - | 鼠标点击位置（运行时） |
| `MouseHitActor` | AActor* | - | 鼠标点击的 Actor（运行时） |
| `OwnerPlayerController` | APlayerController* | - | 所有者玩家控制器（运行时） |
| `OwnerCharacter` | ACharacter* | - | 所有者角色（运行时） |

---

## 实现细节

### 技能激活流程

```cpp
void UElectrocute::ActivateAbility(...)
{
    Super::ActivateAbility(...);
    
    // 1. 存储鼠标数据
    StoreMouseDataInfo(HitResult);
    
    // 2. 存储所有者变量
    StoreOwnerVariables();
    
    // 3. 追踪第一个目标
    TraceFirstTarget(BeamTargetLocation);
    
    // 4. 存储额外目标（连锁）
    TArray<AActor*> AdditionalTargets;
    StoreAdditionalTargets(AdditionalTargets);
    
    // 5. 在蓝图中实现光束视觉效果和伤害逻辑
    // ...
    
    EndAbility(...);
}
```

### 目标追踪

#### 存储鼠标数据

```cpp
void UAuraBeamSpell::StoreMouseDataInfo(const FHitResult& HitResult)
{
    if (HitResult.bBlockingHit)
    {
        MouseHitLocation = HitResult.ImpactPoint;
        MouseHitActor = HitResult.GetActor();
    }
    else
    {
        // 如果没有命中，取消技能
        CancelAbility(...);
    }
}
```

#### 追踪第一个目标

```cpp
void UAuraBeamSpell::TraceFirstTarget(const FVector& BeamTargetLocation)
{
    check(OwnerCharacter);
    
    if (OwnerCharacter->Implements<UCombatInterface>())
    {
        // 获取武器插槽位置
        USkeletalMeshComponent* Weapon = ICombatInterface::Execute_GetWeapon(OwnerCharacter);
        const FVector SocketLocation = Weapon->GetSocketLocation(FName("TipSocket"));
        
        // 球形追踪
        TArray<AActor*> ActorsToIgnore;
        ActorsToIgnore.Add(OwnerCharacter);
        
        FHitResult HitResult;
        UKismetSystemLibrary::SphereTraceSingle(
            OwnerCharacter,
            SocketLocation,
            BeamTargetLocation,
            10.f,  // 追踪半径
            TraceTypeQuery1,
            false,
            ActorsToIgnore,
            EDrawDebugTrace::None,
            HitResult,
            true
        );
        
        if (HitResult.bBlockingHit)
        {
            MouseHitLocation = HitResult.ImpactPoint;
            MouseHitActor = HitResult.GetActor();
        }
    }
    
    // 绑定目标死亡委托
    if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(MouseHitActor))
    {
        CombatInterface->GetOnDeathDelegate().AddDynamic(
            this,
            &UAuraBeamSpell::PrimaryTargetDied
        );
    }
}
```

### 连锁传播

#### 存储额外目标

```cpp
void UAuraBeamSpell::StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets)
{
    // 1. 准备忽略列表
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(GetAvatarActorFromActorInfo());
    ActorsToIgnore.Add(MouseHitActor);
    
    // 2. 获取范围内的活体玩家
    TArray<AActor*> OverlappingActors;
    UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(
        GetAvatarActorFromActorInfo(),
        OverlappingActors,
        ActorsToIgnore,
        850.f,  // 连锁范围
        MouseHitActor->GetActorLocation()
    );
    
    // 3. 计算额外目标数量（根据技能等级）
    int32 NumAdditionalTargets = FMath::Min(
        GetAbilityLevel() - 1,
        MaxNumShockTargets
    );
    
    // 4. 获取最近的目标
    UAuraAbilitySystemLibrary::GetClosestTargets(
        NumAdditionalTargets,
        OverlappingActors,
        OutAdditionalTargets,
        MouseHitActor->GetActorLocation()
    );
    
    // 5. 绑定每个目标的死亡委托
    for (AActor* Target : OutAdditionalTargets)
    {
        if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Target))
        {
            CombatInterface->GetOnDeathDelegate().AddDynamic(
                this,
                &UAuraBeamSpell::AdditionalTargetDied
            );
        }
    }
}
```

### 目标死亡处理

```cpp
// 主目标死亡
UFUNCTION(BlueprintImplementableEvent)
void UAuraBeamSpell::PrimaryTargetDied(AActor* DeadActor)
{
    // 在蓝图中实现：切换到新目标或结束技能
}

// 额外目标死亡
UFUNCTION(BlueprintImplementableEvent)
void UAuraBeamSpell::AdditionalTargetDied(AActor* DeadActor)
{
    // 在蓝图中实现：从连锁列表中移除目标
}
```

---

## 伤害计算

### 持续伤害

光束对每个目标持续造成伤害：

- **伤害频率**: 通过 GameplayEffect 的周期设置
- **伤害值**: 通过 `Damage.GetValueAtLevel(Level)` 计算
- **伤害类型**: 闪电伤害

### 连锁伤害

- **主目标**: 持续受到伤害
- **连锁目标**: 每个连锁目标也持续受到伤害
- **连锁数量**: `Min(Level - 1, MaxNumShockTargets)`

### 伤害应用

1. 光束连接到目标
2. 周期性应用伤害 GameplayEffect
3. 使用 `ExecCalc_Damage` 计算最终伤害
4. 应用伤害到目标

---

## Debuff 效果

### Stun (眩晕)

光束有几率对目标施加眩晕 Debuff：

- **触发几率**: `DebuffChance` (默认 20%)
- **持续时间**: `DebuffDuration` (默认 5 秒)
- **效果**: 目标无法移动和行动

#### 眩晕效果

- 目标移动速度设为 0
- 显示眩晕视觉效果（Niagara 特效）
- 目标无法使用技能

---

## 升级效果

### 等级 1

- 只攻击 **1 个**目标（主目标）
- 基础伤害值

### 等级 2+

- 连锁目标数量 = `Min(Level - 1, MaxNumShockTargets)`
- 伤害值随等级提升（通过 `FScalableFloat` 配置）

### 升级建议

- 优先升级以增加连锁目标数量
- 伤害提升通过 `Damage` 的 `FScalableFloat` 曲线配置

---

## 使用技巧

### 战斗技巧

1. **多目标攻击**: 利用连锁攻击多个敌人
2. **目标选择**: 优先攻击敌人中心的目标，最大化连锁效果
3. **持续输出**: 光束持续伤害，适合攻击高血量目标
4. **眩晕控制**: 利用眩晕效果控制重要目标

### 最佳使用场景

- ✅ 攻击多个聚集的敌人
- ✅ 攻击高血量目标
- ✅ 需要控制效果的场景
- ❌ 不适合攻击快速移动的目标（光束需要持续连接）

---

## 配置指南

### 在编辑器中配置

1. **创建技能蓝图**
   - 继承自 `UElectrocute`
   - 实现光束视觉效果（Niagara 或自定义）

2. **配置连锁参数**
   - 设置 `MaxNumShockTargets`（最大连锁数）
   - 设置连锁范围（在 `StoreAdditionalTargets` 中）

3. **配置伤害参数**
   - 设置 `Damage` 的 `FScalableFloat` 曲线
   - 配置 Debuff 相关参数

4. **创建 GameplayEffect**
   - Cost GameplayEffect
   - Cooldown GameplayEffect
   - Damage GameplayEffect（周期性伤害）

5. **实现蓝图事件**
   - `PrimaryTargetDied`: 主目标死亡处理
   - `AdditionalTargetDied`: 额外目标死亡处理

---

## 代码示例

### 自定义连锁范围

```cpp
// 在子类中重写
void UMyElectrocute::StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets)
{
    // 自定义连锁范围
    float CustomChainRadius = 1000.f;
    
    UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(
        GetAvatarActorFromActorInfo(),
        OverlappingActors,
        ActorsToIgnore,
        CustomChainRadius,  // 自定义范围
        MouseHitActor->GetActorLocation()
    );
    
    // ... 其余逻辑
}
```

---

## 相关文件

- **头文件**: `Source/Aura/Public/AbilitySystem/Abilities/Electrocute.h`
- **实现文件**: `Source/Aura/Private/AbilitySystem/Abilities/Electrocute.cpp`
- **基类**: `UAuraBeamSpell`
- **光束基类**: `UAuraBeamSpell`

---

## 总结

Electrocute 是一个强大的连锁光束技能，通过持续伤害和连锁传播提供了独特的战斗体验。技能适合在敌人聚集时使用，能够同时对多个目标造成伤害和控制效果。

