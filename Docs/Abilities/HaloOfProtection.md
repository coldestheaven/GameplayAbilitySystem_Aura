# HaloOfProtection (光环保护)

## 技能概述

**HaloOfProtection** 是一个被动技能，在角色周围生成保护光环，提供持续的增益效果。技能激活时显示视觉效果（Niagara 特效）。

### 基本信息

- **技能类型**: 被动技能 (Passive Ability)
- **基础类**: `UAuraPassiveAbility`
- **技能标签**: `Abilities.Passive.HaloOfProtection`

### 技能特点

- ✅ 自动激活
- ✅ 持续效果
- ✅ 视觉效果（光环特效）
- ✅ 提供保护增益
- ✅ 不可手动触发

---

## 技能描述

被动技能通常没有详细的技能描述，效果在游戏中持续生效。

---

## 技能参数

被动技能的参数通常通过 GameplayEffect 配置，而不是在技能类中。

### 视觉效果参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `PassiveSpellTag` | FGameplayTag | `Abilities.Passive.HaloOfProtection` | 被动技能标签 |

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
    // ...
    
    // 技能不会自动结束，持续生效
}
```

### 视觉效果组件

```cpp
// 在 AuraCharacterBase 中
UPassiveNiagaraComponent* HaloOfProtectionNiagaraComponent;

// 组件初始化
HaloOfProtectionNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>(
    "HaloOfProtectionComponent"
);
HaloOfProtectionNiagaraComponent->SetupAttachment(EffectAttachComponent);
HaloOfProtectionNiagaraComponent->PassiveSpellTag = 
    GameplayTags.Abilities_Passive_HaloOfProtection;
```

### 视觉效果激活

```cpp
// 在 PassiveNiagaraComponent 中
void UPassiveNiagaraComponent::OnPassiveActivate(
    const FGameplayTag& AbilityTag, 
    bool bActivate
)
{
    if (AbilityTag.MatchesTagExact(PassiveSpellTag))
    {
        if (bActivate)
        {
            Activate();
        }
        else
        {
            Deactivate();
        }
    }
}
```

### 技能停用

```cpp
void UAuraPassiveAbility::ReceiveDeactivate(const FGameplayTag& AbilityTag)
{
    if (AbilityTags.HasTagExact(AbilityTag))
    {
        // 结束技能
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
    }
}
```

---

## 效果机制

### 保护效果

光环保护通常提供以下效果（通过 GameplayEffect 配置）：

- **护甲提升**: 增加角色的护甲值
- **抗性提升**: 增加各种抗性
- **伤害减免**: 减少受到的伤害
- **其他增益**: 根据配置的其他效果

### 持续效果

- **持续时间**: 无限（直到技能被停用）
- **效果范围**: 通常只影响角色自身
- **视觉效果**: 持续显示光环特效

---

## 升级效果

### 等级提升

- 效果强度可以随等级提升
- 可以通过 GameplayEffect 的等级系统实现

### 升级建议

- 优先提升效果强度
- 可以解锁额外效果

---

## 使用技巧

### 战斗技巧

1. **持续保护**: 光环提供持续的保护，适合长期战斗
2. **配合其他技能**: 可以与其他技能配合使用
3. **视觉效果**: 光环特效可以显示技能状态

### 最佳使用场景

- ✅ 需要持续保护
- ✅ 长期战斗
- ✅ 需要额外防御

---

## 配置指南

### 在编辑器中配置

1. **创建技能蓝图**
   - 继承自 `UAuraPassiveAbility`
   - 配置技能标签

2. **创建 GameplayEffect**
   - 创建持续效果 GameplayEffect
   - 配置增益效果（护甲、抗性等）
   - 设置持续时间为无限

3. **配置视觉效果**
   - 创建 Niagara 特效
   - 在角色上添加 `PassiveNiagaraComponent`
   - 设置 `PassiveSpellTag`

4. **添加到角色**
   - 添加到角色的 `StartupPassiveAbilities` 数组
   - 技能会在角色初始化时自动激活

---

## 代码示例

### 自定义保护效果

```cpp
// 在 GameplayEffect 中配置
// 例如：增加护甲
Modifiers.Add(FGameplayModifierInfo());
Modifiers[0].Attribute = UAuraAttributeSet::GetArmorAttribute();
Modifiers[0].ModifierOp = EGameplayModOp::Add;
Modifiers[0].ModifierMagnitude = FScalableFloat(10.f);  // 增加 10 点护甲
```

---

## 相关文件

- **头文件**: `Source/Aura/Public/AbilitySystem/Abilities/AuraPassiveAbility.h`
- **实现文件**: `Source/Aura/Private/AbilitySystem/Abilities/AuraPassiveAbility.cpp`
- **基类**: `UAuraGameplayAbility`
- **视觉效果组件**: `UPassiveNiagaraComponent`

---

## 总结

HaloOfProtection 是一个实用的被动技能，通过持续的保护效果提供了额外的生存能力。技能适合需要持续防御的场景，可以通过配置不同的 GameplayEffect 实现各种保护效果。

