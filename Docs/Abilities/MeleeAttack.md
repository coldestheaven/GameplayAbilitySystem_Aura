# MeleeAttack (近战攻击)

## 技能概述

**MeleeAttack** 是一个近战攻击技能，使用武器对目标造成物理伤害。技能通过动画蒙太奇触发，使用武器碰撞检测来判定命中。

### 基本信息

- **技能类型**: 近战攻击 (Melee Attack)
- **伤害类型**: 物理 (Physical)
- **基础类**: `UAuraDamageGameplayAbility`
- **技能标签**: `Abilities.Attack`

### 技能特点

- ✅ 使用动画蒙太奇
- ✅ 武器碰撞检测
- ✅ 近战范围攻击
- ✅ 造成物理伤害
- ✅ 支持多种攻击动画

---

## 技能描述

近战攻击通常没有详细的技能描述，因为它是基础攻击技能。

---

## 技能参数

### 基础参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `Damage` | FScalableFloat | - | 伤害值（可随等级缩放） |
| `DamageType` | FGameplayTag | `Damage_Physical` | 伤害类型标签 |
| `DebuffChance` | float | 0.0 | Debuff 触发几率（通常为 0） |
| `DeathImpulseMagnitude` | float | 1000.0 | 死亡冲量大小 |
| `KnockbackForceMagnitude` | float | 1000.0 | 击退力度 |
| `KnockbackChance` | float | 0.0 | 击退触发几率 |

### 动画参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `AttackMontages` | TArray<FTaggedMontage> | - | 攻击动画蒙太奇数组 |

---

## 实现细节

### 技能激活流程

```cpp
void UAuraMeleeAttack::ActivateAbility(...)
{
    Super::ActivateAbility(...);
    
    // 1. 获取攻击动画
    FTaggedMontage AttackMontage = GetRandomTaggedMontageFromArray(AttackMontages);
    
    // 2. 播放动画蒙太奇
    PlayMontage(AttackMontage.Montage);
    
    // 3. 通过动画通知触发伤害检测
    // （在动画蒙太奇中设置通知）
    
    EndAbility(...);
}
```

### 动画蒙太奇

近战攻击使用 `FTaggedMontage` 结构：

```cpp
struct FTaggedMontage
{
    UAnimMontage* Montage;        // 动画蒙太奇
    FGameplayTag MontageTag;      // 蒙太奇标签
    FGameplayTag SocketTag;       // 插槽标签
    USoundBase* ImpactSound;      // 碰撞音效
};
```

### 伤害检测

伤害检测通过动画通知触发：

1. **动画通知**: 在动画蒙太奇中设置 `GameplayEvent` 通知
2. **碰撞检测**: 通知触发时进行武器碰撞检测
3. **应用伤害**: 对命中的目标应用伤害

### 武器碰撞

使用武器的碰撞体进行检测：

```cpp
// 在动画通知中
void OnMeleeHit()
{
    // 1. 获取武器
    USkeletalMeshComponent* Weapon = ICombatInterface::Execute_GetWeapon(Character);
    
    // 2. 检测碰撞
    TArray<FHitResult> HitResults;
    // ... 碰撞检测逻辑
    
    // 3. 对每个命中的目标造成伤害
    for (const FHitResult& Hit : HitResults)
    {
        if (AActor* HitActor = Hit.GetActor())
        {
            CauseDamage(HitActor);
        }
    }
}
```

---

## 伤害计算

### 基础伤害

伤害值通过 `FScalableFloat` 配置：

```cpp
const float ScaledDamage = Damage.GetValueAtLevel(GetAbilityLevel());
```

### 物理伤害

- **伤害类型**: 物理伤害
- **抗性减免**: 受目标的 `PhysicalResistance` 影响
- **护甲减免**: 受目标的 `Armor` 影响

### 伤害应用

1. 武器碰撞检测到目标
2. 创建伤害 GameplayEffect
3. 使用 `ExecCalc_Damage` 计算最终伤害
4. 应用伤害到目标

---

## Debuff 效果

近战攻击通常不触发 Debuff，但可以通过配置添加：

- **DebuffChance**: 可以设置为非零值来触发 Debuff
- **Debuff 类型**: 通常是物理 Debuff

---

## 升级效果

### 等级提升

- 伤害值随等级提升（通过 `FScalableFloat` 配置）
- 可以解锁更多攻击动画

### 升级建议

- 优先提升伤害值
- 可以通过添加更多攻击动画增加变化

---

## 使用技巧

### 战斗技巧

1. **连击**: 使用不同的攻击动画形成连击
2. **时机**: 掌握攻击时机，避免被敌人打断
3. **范围**: 了解武器的攻击范围
4. **取消**: 可以通过其他技能取消攻击后摇

### 最佳使用场景

- ✅ 近距离战斗
- ✅ 基础伤害输出
- ✅ 配合其他技能使用
- ❌ 不适合攻击远程敌人

---

## 配置指南

### 在编辑器中配置

1. **创建技能蓝图**
   - 继承自 `UAuraMeleeAttack`
   - 配置攻击动画数组

2. **配置攻击动画**
   - 创建多个 `FTaggedMontage` 条目
   - 设置不同的攻击动画
   - 配置插槽标签和音效

3. **配置伤害参数**
   - 设置 `Damage` 的 `FScalableFloat` 曲线
   - 配置击退和死亡冲量参数

4. **设置动画通知**
   - 在动画蒙太奇中添加 `GameplayEvent` 通知
   - 配置通知触发时机

5. **创建 GameplayEffect**
   - Cost GameplayEffect（通常为 0）
   - Cooldown GameplayEffect（通常很短或为 0）
   - Damage GameplayEffect

---

## 代码示例

### 获取随机攻击动画

```cpp
FTaggedMontage UAuraDamageGameplayAbility::GetRandomTaggedMontageFromArray(
    const TArray<FTaggedMontage>& TaggedMontages
) const
{
    if (TaggedMontages.Num() > 0)
    {
        const int32 Selection = FMath::RandRange(0, TaggedMontages.Num() - 1);
        return TaggedMontages[Selection];
    }
    return FTaggedMontage();
}
```

### 自定义伤害检测

```cpp
// 在动画通知中
void UMeleeAttackNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    // 获取角色
    AActor* Owner = MeshComp->GetOwner();
    
    // 获取武器
    if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Owner))
    {
        USkeletalMeshComponent* Weapon = CombatInterface->GetWeapon();
        
        // 碰撞检测
        // ... 检测逻辑
        
        // 应用伤害
        // ...
    }
}
```

---

## 相关文件

- **头文件**: `Source/Aura/Public/AbilitySystem/Abilities/AuraMeleeAttack.h`
- **实现文件**: `Source/Aura/Private/AbilitySystem/Abilities/AuraMeleeAttack.cpp`
- **基类**: `UAuraDamageGameplayAbility`
- **战斗接口**: `ICombatInterface`

---

## 总结

MeleeAttack 是基础的近战攻击技能，通过动画蒙太奇和武器碰撞检测提供了流畅的近战战斗体验。技能适合作为主要伤害输出手段，可以配合其他技能形成连击。

