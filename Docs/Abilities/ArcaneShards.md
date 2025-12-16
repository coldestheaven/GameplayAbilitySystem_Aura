# ArcaneShards (奥术碎片)

## 技能概述

**ArcaneShards** 是一个投射物技能，召唤一个或多个奥术碎片，在目标位置造成范围奥术伤害。碎片数量随技能等级增加。

### 基本信息

- **技能类型**: 投射物技能 (Projectile Spell)
- **伤害类型**: 奥术 (Arcane)
- **基础类**: `UAuraDamageGameplayAbility`
- **技能标签**: `Abilities.Arcane.ArcaneShards`

### 技能特点

- ✅ 召唤奥术碎片
- ✅ 造成范围伤害（Radial Damage）
- ✅ 碎片数量随等级增加
- ✅ 有几率造成奥术 Debuff
- ✅ 视觉效果华丽

---

## 技能描述

### 等级 1

```
ARCANE SHARDS

Level: 1
ManaCost: [法力消耗]
Cooldown: [冷却时间]

Summon a shard of arcane energy, causing radial arcane damage of [伤害] at the shard origin.
```

### 等级 2+

```
ARCANE SHARDS

Level: [等级]
ManaCost: [法力消耗]
Cooldown: [冷却时间]

Summon [数量] shards of arcane energy, causing radial arcane damage of [伤害] at the shard origins.
```

### 下一级描述

```
NEXT LEVEL:

Level: [下一级等级]
ManaCost: [下一级法力消耗]
Cooldown: [下一级冷却时间]

Summon [下一级数量] shards of arcane energy, causing radial arcane damage of [下一级伤害] at the shard origins.
```

---

## 技能参数

### 基础参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `Damage` | FScalableFloat | - | 伤害值（可随等级缩放） |
| `DamageType` | FGameplayTag | `Damage_Arcane` | 伤害类型标签 |
| `DebuffChance` | float | 20.0 | 奥术 Debuff 触发几率（百分比） |
| `DebuffDamage` | float | 5.0 | Debuff 伤害 |
| `DebuffDuration` | float | 5.0 | Debuff 持续时间（秒） |
| `DebuffFrequency` | float | 1.0 | Debuff 伤害频率（秒） |

### 碎片参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `MaxNumShards` | int32 | 11 | 最大碎片数量 |

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
void UArcaneShards::ActivateAbility(...)
{
    Super::ActivateAbility(...);
    
    // 1. 获取目标位置（通常从鼠标点击位置）
    FVector TargetLocation = GetTargetLocation();
    
    // 2. 计算碎片数量
    int32 NumShards = FMath::Min(GetAbilityLevel(), MaxNumShards);
    
    // 3. 在目标位置生成碎片
    // （具体实现可能在蓝图中）
    
    // 4. 应用范围伤害
    ApplyRadialDamage(TargetLocation);
    
    EndAbility(...);
}
```

### 碎片生成

碎片通常在目标位置生成，每个碎片都会造成范围伤害：

1. **确定碎片数量**: `Min(Level, MaxNumShards)`
2. **生成碎片**: 在目标位置或周围生成碎片
3. **应用伤害**: 每个碎片造成范围伤害

### 范围伤害

使用范围伤害系统：

- **伤害类型**: 奥术伤害
- **伤害范围**: 通过 `RadialDamageInnerRadius` 和 `RadialDamageOuterRadius` 配置
- **伤害衰减**: 从内圈到外圈伤害逐渐衰减

---

## 伤害计算

### 基础伤害

伤害值通过 `FScalableFloat` 配置，可以随技能等级缩放：

```cpp
const int32 ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel());
```

### 范围伤害

每个碎片造成范围伤害：

1. 碎片在目标位置生成
2. 应用范围伤害到范围内的所有目标
3. 使用 `ExecCalc_Damage` 计算最终伤害
4. 考虑目标的奥术抗性

### 伤害叠加

如果多个碎片在同一位置或重叠区域：

- 每个碎片独立计算伤害
- 目标可能受到多次伤害
- 总伤害 = 碎片数量 × 单次伤害（在重叠区域）

---

## Debuff 效果

### Arcane Debuff (奥术 Debuff)

碎片有几率对目标施加奥术 Debuff：

- **触发几率**: `DebuffChance` (默认 20%)
- **持续时间**: `DebuffDuration` (默认 5 秒)
- **伤害频率**: `DebuffFrequency` (默认 1 秒)
- **每次伤害**: `DebuffDamage` (默认 5 点)

#### 奥术 Debuff 效果

- 持续造成奥术伤害
- 显示奥术视觉效果（Niagara 特效）
- 可以通过抗性减少伤害

---

## 升级效果

### 等级 1

- 召唤 **1 个**碎片
- 基础伤害值

### 等级 2+

- 碎片数量 = `Min(Level, MaxNumShards)`
- 伤害值随等级提升（通过 `FScalableFloat` 配置）

### 升级建议

- 优先升级以增加碎片数量
- 伤害提升通过 `Damage` 的 `FScalableFloat` 曲线配置
- 碎片数量上限为 `MaxNumShards`（默认 11）

---

## 使用技巧

### 战斗技巧

1. **范围控制**: 利用多个碎片覆盖更大范围
2. **目标选择**: 在敌人聚集位置使用效果最佳
3. **伤害叠加**: 多个碎片重叠可以造成更高伤害
4. **预判位置**: 预判敌人移动位置放置碎片

### 最佳使用场景

- ✅ 攻击多个聚集的敌人
- ✅ 在固定位置造成持续威胁
- ✅ 需要范围伤害的场景
- ❌ 不适合攻击快速移动的目标

---

## 配置指南

### 在编辑器中配置

1. **创建技能蓝图**
   - 继承自 `UArcaneShards`
   - 实现碎片生成逻辑（可能在蓝图中）

2. **配置碎片参数**
   - 设置 `MaxNumShards`（最大碎片数）

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

### 自定义碎片分布

```cpp
// 在子类中实现自定义碎片分布
void UMyArcaneShards::SpawnShards(const FVector& TargetLocation)
{
    int32 NumShards = FMath::Min(GetAbilityLevel(), MaxNumShards);
    
    // 圆形分布
    for (int32 i = 0; i < NumShards; i++)
    {
        float Angle = (360.f / NumShards) * i;
        FVector Offset = FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Radius,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Radius,
            0.f
        );
        
        FVector ShardLocation = TargetLocation + Offset;
        SpawnShard(ShardLocation);
    }
}
```

---

## 相关文件

- **头文件**: `Source/Aura/Public/AbilitySystem/Abilities/ArcaneShards.h`
- **实现文件**: `Source/Aura/Private/AbilitySystem/Abilities/ArcaneShards.cpp`
- **基类**: `UAuraDamageGameplayAbility`

---

## 总结

ArcaneShards 是一个灵活的范围伤害技能，通过多个碎片提供了良好的区域控制能力。技能适合在敌人聚集时使用，能够造成大量范围伤害。

