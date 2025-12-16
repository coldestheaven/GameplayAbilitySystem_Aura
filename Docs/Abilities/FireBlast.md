# FireBlast (火球爆炸)

## 技能概述

**FireBlast** 是一个特殊的投射物技能，向四周发射多个火球，火球会飞出一段距离后返回并爆炸，造成范围火焰伤害。这是一个强大的范围攻击技能。

### 基本信息

- **技能类型**: 特殊投射物技能 (Special Projectile Spell)
- **伤害类型**: 火焰 (Fire)
- **基础类**: `UAuraDamageGameplayAbility`
- **技能标签**: `Abilities.Fire.FireBlast`

### 技能特点

- ✅ 向四周 360 度发射火球
- ✅ 火球返回并爆炸
- ✅ 造成范围伤害（Radial Damage）
- ✅ 有几率造成燃烧 Debuff
- ✅ 视觉效果震撼

---

## 技能描述

```
FIRE BLAST

Level: [等级]
ManaCost: [法力消耗]
Cooldown: [冷却时间]

Launches [数量] fire balls in all directions, each coming back and exploding upon return, causing [伤害] radial fire damage with a chance to burn
```

### 下一级描述

```
NEXT LEVEL:

Level: [下一级等级]
ManaCost: [下一级法力消耗]
Cooldown: [下一级冷却时间]

Launches [下一级数量] fire balls in all directions, each coming back and exploding upon return, causing [下一级伤害] radial fire damage with a chance to burn
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

### 火球参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `FireBallClass` | TSubclassOf<AAuraFireBall> | - | 火球类 |
| `NumFireBalls` | int32 | 12 | 火球数量 |

### 范围伤害参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `bIsRadialDamage` | bool | true | 是否为范围伤害 |
| `RadialDamageInnerRadius` | float | - | 范围伤害内半径 |
| `RadialDamageOuterRadius` | float | - | 范围伤害外半径 |

---

## 实现细节

### 技能激活流程

```cpp
void UAuraFireBlast::ActivateAbility(...)
{
    Super::ActivateAbility(...);
    
    // 生成火球
    TArray<AAuraFireBall*> FireBalls = SpawnFireBalls();
    
    // 火球会自动处理返回和爆炸逻辑
    EndAbility(...);
}
```

### 火球生成逻辑

```cpp
TArray<AAuraFireBall*> UAuraFireBlast::SpawnFireBalls()
{
    TArray<AAuraFireBall*> FireBalls;
    
    // 1. 获取角色位置和朝向
    const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
    const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
    
    // 2. 计算 360 度均匀分布的旋转
    TArray<FRotator> Rotators = UAuraAbilitySystemLibrary::EvenlySpacedRotators(
        Forward,
        FVector::UpVector,
        360.f,  // 360 度全方向
        NumFireBalls
    );
    
    // 3. 生成每个火球
    for (const FRotator& Rotator : Rotators)
    {
        // 创建火球
        FTransform SpawnTransform;
        SpawnTransform.SetLocation(Location);
        SpawnTransform.SetRotation(Rotator.Quaternion());
        
        AAuraFireBall* FireBall = GetWorld()->SpawnActorDeferred<AAuraFireBall>(
            FireBallClass,
            SpawnTransform,
            GetOwningActorFromActorInfo(),
            CurrentActorInfo->PlayerController->GetPawn(),
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn
        );
        
        // 设置伤害参数（飞行伤害）
        FireBall->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
        
        // 设置返回目标（角色自身）
        FireBall->ReturnToActor = GetAvatarActorFromActorInfo();
        FireBall->SetOwner(GetAvatarActorFromActorInfo());
        
        // 设置爆炸伤害参数
        FireBall->ExplosionDamageParams = MakeDamageEffectParamsFromClassDefaults();
        
        // 完成生成
        FireBalls.Add(FireBall);
        FireBall->FinishSpawning(SpawnTransform);
    }
    
    return FireBalls;
}
```

### 火球行为

1. **发射阶段**: 火球从角色位置向四周发射
2. **飞行阶段**: 火球飞出一段距离
3. **返回阶段**: 火球返回角色位置
4. **爆炸阶段**: 火球返回时在角色位置爆炸，造成范围伤害

### 关键实现点

1. **360 度分布**: 使用 `EvenlySpacedRotators` 计算 360 度均匀分布
2. **双重伤害**: 火球飞行和爆炸都有伤害参数
3. **返回机制**: 火球通过 `ReturnToActor` 设置返回目标
4. **范围伤害**: 爆炸时使用范围伤害系统

---

## 伤害计算

### 伤害类型

- **飞行伤害**: 火球飞行过程中碰撞造成的伤害
- **爆炸伤害**: 火球返回爆炸时的范围伤害

### 范围伤害

爆炸伤害是范围伤害（Radial Damage），使用 `UGameplayStatics::ApplyRadialDamageWithFalloff`：

- **内半径**: `RadialDamageInnerRadius` - 内圈伤害
- **外半径**: `RadialDamageOuterRadius` - 外圈伤害
- **伤害衰减**: 从内圈到外圈伤害逐渐衰减

### 伤害应用

1. 火球返回角色位置
2. 触发爆炸
3. 应用范围伤害到范围内的所有目标
4. 使用 `ExecCalc_Damage` 计算最终伤害

---

## Debuff 效果

### Burn (燃烧)

爆炸时有几率触发燃烧 Debuff：

- **触发几率**: `DebuffChance` (默认 20%)
- **持续时间**: `DebuffDuration` (默认 5 秒)
- **伤害频率**: `DebuffFrequency` (默认 1 秒)
- **每次伤害**: `DebuffDamage` (默认 5 点)

---

## 升级效果

### 等级提升

- 火球数量固定为 `NumFireBalls`（默认 12 个）
- 伤害值随等级提升（通过 `FScalableFloat` 配置）

### 升级建议

- 优先提升伤害值
- 可以通过调整 `NumFireBalls` 增加火球数量

---

## 使用技巧

### 战斗技巧

1. **范围清怪**: 适合清理大量聚集的敌人
2. **中心位置**: 在敌人中心使用效果最佳
3. **返回时机**: 注意火球返回的时机，避免误伤
4. **配合移动**: 可以在移动中使用，火球会跟随返回

### 最佳使用场景

- ✅ 清理大量聚集的敌人
- ✅ 在敌人中心位置使用
- ✅ 需要范围伤害的场景
- ❌ 不适合攻击单个目标（伤害分散）

---

## 配置指南

### 在编辑器中配置

1. **创建技能蓝图**
   - 继承自 `UAuraFireBlast`
   - 设置 `FireBallClass` 为火球类

2. **配置火球数量**
   - 设置 `NumFireBalls`（默认 12）

3. **配置范围伤害**
   - 设置 `bIsRadialDamage = true`
   - 设置 `RadialDamageInnerRadius`
   - 设置 `RadialDamageOuterRadius`

4. **配置伤害参数**
   - 设置 `Damage` 的 `FScalableFloat` 曲线
   - 配置 Debuff 相关参数

5. **创建 GameplayEffect**
   - Cost GameplayEffect
   - Cooldown GameplayEffect
   - Damage GameplayEffect（范围伤害）

---

## 代码示例

### 自定义火球数量

```cpp
// 根据等级动态调整火球数量
int32 GetEffectiveNumFireBalls() const
{
    return FMath::Min(GetAbilityLevel() * 2, NumFireBalls);
}
```

### 自定义返回距离

```cpp
// 在 FireBall 类中设置返回距离
void AAuraFireBall::SetReturnDistance(float Distance)
{
    ReturnDistance = Distance;
}
```

---

## 相关文件

- **头文件**: `Source/Aura/Public/AbilitySystem/Abilities/AuraFireBlast.h`
- **实现文件**: `Source/Aura/Private/AbilitySystem/Abilities/AuraFireBlast.cpp`
- **基类**: `UAuraDamageGameplayAbility`
- **火球类**: `AAuraFireBall`

---

## 总结

FireBlast 是一个强大的范围攻击技能，通过向四周发射火球并返回爆炸，提供了独特的战斗体验。技能适合在敌人聚集时使用，能够造成大量范围伤害。

