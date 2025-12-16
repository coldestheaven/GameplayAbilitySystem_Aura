# ManaSiphon (法力汲取)

## 技能概述

**ManaSiphon** 是一个被动技能，当角色对敌人造成伤害时，会汲取一部分伤害转化为法力值。技能激活时显示视觉效果（Niagara 特效）。

### 基本信息

- **技能类型**: 被动技能 (Passive Ability)
- **基础类**: `UAuraPassiveAbility`
- **技能标签**: `Abilities.Passive.ManaSiphon`

### 技能特点

- ✅ 自动激活
- ✅ 持续效果
- ✅ 伤害转化为法力值
- ✅ 视觉效果（汲取特效）
- ✅ 不可手动触发

---

## 技能描述

被动技能通常没有详细的技能描述，效果在游戏中持续生效。

---

## 技能参数

被动技能的参数通常通过 GameplayEffect 配置。

### 视觉效果参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `PassiveSpellTag` | FGameplayTag | `Abilities.Passive.ManaSiphon` | 被动技能标签 |

### 汲取参数（通过 GameplayEffect 配置）

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `SiphonPercentage` | float | 10.0 | 汲取百分比（伤害的百分比转化为法力） |

---

## 实现细节

### 技能激活流程

```cpp
void UAuraPassiveAbility::ActivateAbility(...)
{
    Super::ActivateAbility(...);
    
    // 1. 绑定停用委托
    if (UAuraAbilitySystemComponent* AuraASC = ...)
    {
        AuraASC->DeactivatePassiveAbility.AddUObject(
            this,
            &UAuraPassiveAbility::ReceiveDeactivate
        );
    }
    
    // 2. 应用持续效果（通过 GameplayEffect）
    // GameplayEffect 会监听伤害事件并触发法力汲取
}
```

### 法力汲取机制

法力汲取通常通过以下方式实现：

1. **监听伤害事件**: 通过 GameplayEffect 监听角色造成的伤害
2. **计算汲取量**: 根据伤害值计算汲取的法力值
3. **恢复法力**: 将汲取的法力值添加到角色的 Mana 属性

### 实现方式

#### 方式 1: 通过 GameplayEffect 的 Event

```cpp
// 在 GameplayEffect 中
// 监听伤害事件
EventTriggers.Add(FGameplayEffectExecutionScopedModifierInfo());
EventTriggers[0].EventTag = FGameplayTag::RequestGameplayTag("Event.Damage");
EventTriggers[0].ModifierOp = EGameplayModOp::Override;
EventTriggers[0].ModifierMagnitude = FScalableFloat(0.1f);  // 10% 汲取
```

#### 方式 2: 在伤害计算中处理

```cpp
// 在 ExecCalc_Damage 中
void UExecCalc_Damage::Execute_Implementation(...)
{
    // 计算伤害
    float FinalDamage = ...;
    
    // 检查是否有法力汲取
    if (SourceASC->HasMatchingGameplayTag(Abilities_Passive_ManaSiphon))
    {
        float SiphonAmount = FinalDamage * SiphonPercentage;
        
        // 恢复法力值
        FGameplayEffectSpecHandle ManaSpec = ...;
        SourceASC->ApplyGameplayEffectSpecToSelf(*ManaSpec.Data.Get());
    }
}
```

### 视觉效果组件

```cpp
// 在 AuraCharacterBase 中
UPassiveNiagaraComponent* ManaSiphonNiagaraComponent;

// 组件初始化
ManaSiphonNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>(
    "ManaSiphonNiagaraComponent"
);
ManaSiphonNiagaraComponent->SetupAttachment(EffectAttachComponent);
ManaSiphonNiagaraComponent->PassiveSpellTag = 
    GameplayTags.Abilities_Passive_ManaSiphon;
```

---

## 效果机制

### 汲取计算

法力汲取通常按以下方式计算：

```
汲取法力值 = 造成的伤害 × 汲取百分比
```

例如：
- 造成 100 点伤害
- 汲取百分比 10%
- 汲取法力值 = 100 × 0.1 = 10 点

### 汲取限制

- **最大法力值限制**: 汲取的法力值不能超过最大法力值
- **最小汲取值**: 可以设置最小汲取值（例如至少 1 点）

---

## 升级效果

### 等级提升

- 汲取百分比可以随等级提升
- 可以通过 GameplayEffect 的等级系统实现

### 升级建议

- 优先提升汲取百分比
- 可以解锁额外效果（例如汲取上限提升）

---

## 使用技巧

### 战斗技巧

1. **持续恢复**: 在战斗中持续恢复法力值
2. **高伤害技能**: 配合高伤害技能可以快速恢复法力
3. **技能循环**: 提高技能的持续使用能力

### 最佳使用场景

- ✅ 需要持续恢复法力值
- ✅ 高伤害输出角色
- ✅ 频繁使用技能的角色

---

## 配置指南

### 在编辑器中配置

1. **创建技能蓝图**
   - 继承自 `UAuraPassiveAbility`
   - 配置技能标签

2. **创建 GameplayEffect**
   - 创建持续效果 GameplayEffect
   - 配置法力汲取逻辑
   - 设置持续时间为无限

3. **配置视觉效果**
   - 创建 Niagara 特效
   - 在角色上添加 `PassiveNiagaraComponent`
   - 设置 `PassiveSpellTag`

4. **添加到角色**
   - 添加到角色的 `StartupPassiveAbilities` 数组

---

## 代码示例

### 自定义汲取百分比

```cpp
// 在 GameplayEffect 中配置
// 例如：20% 汲取
Modifiers.Add(FGameplayModifierInfo());
Modifiers[0].Attribute = UAuraAttributeSet::GetManaAttribute();
Modifiers[0].ModifierOp = EGameplayModOp::Add;
Modifiers[0].ModifierMagnitude = FScalableFloat(0.2f);  // 20% 汲取
```

---

## 相关文件

- **头文件**: `Source/Aura/Public/AbilitySystem/Abilities/AuraPassiveAbility.h`
- **实现文件**: `Source/Aura/Private/AbilitySystem/Abilities/AuraPassiveAbility.cpp`
- **基类**: `UAuraGameplayAbility`
- **视觉效果组件**: `UPassiveNiagaraComponent`

---

## 总结

ManaSiphon 是一个实用的被动技能，通过将伤害转化为法力值提供了持续的恢复能力。技能适合需要持续恢复法力的场景，特别适合频繁使用技能的角色。

