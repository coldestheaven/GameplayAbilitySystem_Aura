# FireBolt (火球术)

## 技能概述

**FireBolt** 是一个投射物技能，发射一个或多个追踪火球，对目标造成火焰伤害。火球在命中目标时爆炸，有几率对目标施加燃烧（Burn）Debuff。

### 基本信息

- **技能类型**: 投射物技能 (Projectile Spell)
- **伤害类型**: 火焰 (Fire)
- **基础类**: `UAuraProjectileSpell`
- **技能标签**: `Abilities.Fire.FireBolt`

### 技能特点

- ✅ 支持多个火球（根据技能等级）
- ✅ 火球可以追踪目标（Homing）
- ✅ 碰撞时爆炸造成伤害
- ✅ 有几率造成燃烧 Debuff
- ✅ 伤害随等级提升

---

## 技能描述

### 等级 1

```
FIRE BOLT

Level: 1
ManaCost: [法力消耗]
Cooldown: [冷却时间]

Launches a bolt of fire, exploding on impact and dealing [伤害] fire damage with a chance to burn
```

### 等级 2+

```
FIRE BOLT

Level: [等级]
ManaCost: [法力消耗]
Cooldown: [冷却时间]

Launches [数量] bolts of fire, exploding on impact and dealing [伤害] fire damage with a chance to burn
```

### 下一级描述

```
NEXT LEVEL:

Level: [下一级等级]
ManaCost: [下一级法力消耗]
Cooldown: [下一级冷却时间]

Launches [下一级数量] bolts of fire, exploding on impact and dealing [下一级伤害] fire damage with a chance to burn
```

---

## 技能参数

### 基础参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `Damage` | FScalableFloat | - | 伤害值（可随等级缩放） |
| `DamageType` | FGameplayTag | `Damage_Fire` | 伤害类型标签 |
| `DebuffChance` | float | 20.0 | 燃烧触发几率（百分比） |
| `DebuffDamage` | float | 5.0 | 燃烧每秒伤害 |
| `DebuffDuration` | float | 5.0 | 燃烧持续时间（秒） |
| `DebuffFrequency` | float | 1.0 | 燃烧伤害频率（秒） |

### 投射物参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `ProjectileClass` | TSubclassOf<AAuraProjectile> | - | 投射物类 |
| `NumProjectiles` | int32 | 5 | 最大投射物数量 |
| `ProjectileSpread` | float | 90.0 | 投射物散布角度（度） |

### 追踪参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `bLaunchHomingProjectiles` | bool | true | 是否启用追踪 |
| `HomingAccelerationMin` | float | 1600.0 | 最小追踪加速度 |
| `HomingAccelerationMax` | float | 3200.0 | 最大追踪加速度 |

### 其他参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `DeathImpulseMagnitude` | float | 1000.0 | 死亡冲量大小 |
| `KnockbackForceMagnitude` | float | 1000.0 | 击退力度 |
| `KnockbackChance` | float | 0.0 | 击退触发几率 |

---

## 实现细节

### 技能激活流程

```cpp
void UAuraFireBolt::ActivateAbility(...)
{
    Super::ActivateAbility(...);
    
    // 1. 获取目标位置（通常从鼠标点击位置）
    FVector TargetLocation = GetTargetLocation();
    
    // 2. 生成多个火球
    SpawnProjectiles(TargetLocation, SocketTag, bOverridePitch, PitchOverride, HomingTarget);
    
    // 3. 结束技能
    EndAbility(...);
}
```

### 火球生成逻辑

```cpp
void UAuraFireBolt::SpawnProjectiles(
    const FVector& ProjectileTargetLocation,
    const FGameplayTag& SocketTag,
    bool bOverridePitch,
    float PitchOverride,
    AActor* HomingTarget
)
{
    // 1. 获取发射位置（从角色的战斗插槽）
    const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(
        GetAvatarActorFromActorInfo(),
        SocketTag
    );
    
    // 2. 计算基础旋转（朝向目标）
    FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
    if (bOverridePitch) Rotation.Pitch = PitchOverride;
    
    // 3. 计算有效投射物数量（不超过技能等级）
    const int32 EffectiveNumProjectiles = FMath::Min(NumProjectiles, GetAbilityLevel());
    
    // 4. 计算扇形分布的旋转
    const FVector Forward = Rotation.Vector();
    TArray<FRotator> Rotations = UAuraAbilitySystemLibrary::EvenlySpacedRotators(
        Forward,
        FVector::UpVector,
        ProjectileSpread,
        EffectiveNumProjectiles
    );
    
    // 5. 生成每个火球
    for (const FRotator& Rot : Rotations)
    {
        // 创建投射物
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
        
        // 设置伤害参数
        Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
        
        // 设置追踪目标
        if (HomingTarget && HomingTarget->Implements<UCombatInterface>())
        {
            Projectile->ProjectileMovement->HomingTargetComponent = 
                HomingTarget->GetRootComponent();
        }
        else
        {
            // 创建临时场景组件作为追踪目标
            Projectile->HomingTargetSceneComponent = NewObject<USceneComponent>(...);
            Projectile->HomingTargetSceneComponent->SetWorldLocation(ProjectileTargetLocation);
            Projectile->ProjectileMovement->HomingTargetComponent = 
                Projectile->HomingTargetSceneComponent;
        }
        
        // 设置追踪加速度（随机值）
        Projectile->ProjectileMovement->HomingAccelerationMagnitude = 
            FMath::FRandRange(HomingAccelerationMin, HomingAccelerationMax);
        
        // 启用追踪
        Projectile->ProjectileMovement->bIsHomingProjectile = bLaunchHomingProjectiles;
        
        // 完成生成
        Projectile->FinishSpawning(SpawnTransform);
    }
}
```

### 关键实现点

1. **扇形分布**: 使用 `EvenlySpacedRotators` 函数计算多个投射物的旋转，形成扇形分布
2. **等级限制**: 投射物数量受技能等级限制，最多不超过 `NumProjectiles`
3. **追踪系统**: 支持追踪实际目标或目标位置
4. **随机加速度**: 每个火球的追踪加速度在范围内随机，增加视觉变化

---

## 伤害计算

### 基础伤害

伤害值通过 `FScalableFloat` 配置，可以随技能等级缩放：

```cpp
const int32 ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel());
```

### 伤害应用

伤害通过 `DamageEffectParams` 传递给投射物，投射物碰撞时应用伤害：

1. 投射物碰撞目标
2. 创建伤害 GameplayEffect
3. 使用 `ExecCalc_Damage` 计算最终伤害
4. 应用伤害到目标

### 伤害类型

- **基础伤害**: 火焰伤害
- **抗性减免**: 受目标的 `FireResistance` 影响
- **Debuff 伤害**: 如果触发燃烧，持续造成火焰伤害

---

## Debuff 效果

### Burn (燃烧)

当火球命中目标时，有几率触发燃烧 Debuff：

- **触发几率**: `DebuffChance` (默认 20%)
- **持续时间**: `DebuffDuration` (默认 5 秒)
- **伤害频率**: `DebuffFrequency` (默认 1 秒)
- **每次伤害**: `DebuffDamage` (默认 5 点)

#### 燃烧效果

- 持续造成火焰伤害
- 显示燃烧视觉效果（Niagara 特效）
- 可以通过抗性减少伤害

---

## 升级效果

### 等级 1

- 发射 **1 个**火球
- 基础伤害值

### 等级 2+

- 发射数量 = `Min(Level, NumProjectiles)` 个火球
- 伤害值随等级提升（通过 `FScalableFloat` 配置）

### 升级建议

- 优先升级以增加投射物数量
- 伤害提升通过 `Damage` 的 `FScalableFloat` 曲线配置

---

## 使用技巧

### 战斗技巧

1. **多目标**: 利用多个火球的扇形分布攻击多个敌人
2. **追踪优势**: 火球会自动追踪目标，适合攻击移动中的敌人
3. **燃烧叠加**: 多个火球可以同时触发燃烧，叠加伤害
4. **范围控制**: 通过 `ProjectileSpread` 调整火球分布范围

### 最佳使用场景

- ✅ 攻击单个重要目标
- ✅ 攻击多个聚集的敌人
- ✅ 攻击移动中的敌人
- ❌ 不适合攻击快速移动的小型目标（可能被躲避）

---

## 配置指南

### 在编辑器中配置

1. **创建技能蓝图**
   - 继承自 `UAuraFireBolt`
   - 设置 `ProjectileClass` 为火球投射物类

2. **配置伤害参数**
   - 设置 `Damage` 的 `FScalableFloat` 曲线
   - 配置 `DebuffChance`, `DebuffDamage` 等参数

3. **配置投射物参数**
   - 设置 `NumProjectiles`（最大数量）
   - 设置 `ProjectileSpread`（散布角度）

4. **配置追踪参数**
   - 设置 `bLaunchHomingProjectiles`
   - 设置 `HomingAccelerationMin/Max`

5. **创建 GameplayEffect**
   - Cost GameplayEffect（法力消耗）
   - Cooldown GameplayEffect（冷却时间）
   - Damage GameplayEffect（伤害效果）

6. **添加到 AbilityInfo**
   - 在 `AbilityInfo` 数据资产中添加技能信息
   - 设置技能标签、图标、等级要求等

---

## 代码示例

### 在蓝图中使用

```cpp
// 生成火球
UFUNCTION(BlueprintCallable)
void SpawnFireBolts()
{
    FVector TargetLocation = GetMouseHitLocation();
    FGameplayTag SocketTag = GetCombatSocketTag();
    AActor* HomingTarget = GetTargetActor();
    
    SpawnProjectiles(TargetLocation, SocketTag, false, 0.f, HomingTarget);
}
```

### 自定义火球数量

```cpp
// 在子类中重写
void UMyCustomFireBolt::SpawnProjectiles(...)
{
    // 自定义投射物数量逻辑
    int32 CustomNumProjectiles = CalculateCustomNumProjectiles();
    
    // 调用父类方法或自定义实现
    Super::SpawnProjectiles(...);
}
```

---

## 相关文件

- **头文件**: `Source/Aura/Public/AbilitySystem/Abilities/AuraFireBolt.h`
- **实现文件**: `Source/Aura/Private/AbilitySystem/Abilities/AuraFireBolt.cpp`
- **基类**: `UAuraProjectileSpell`
- **投射物类**: `AAuraProjectile`

---

## 总结

FireBolt 是一个灵活且强大的投射物技能，通过多火球和追踪系统提供了良好的战斗体验。技能的可配置性使其可以适应不同的游戏平衡需求。

