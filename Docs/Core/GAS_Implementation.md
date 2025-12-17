# GAS 实现文档

本文档详细介绍了 Aura 项目中 Gameplay Ability System (GAS) 的完整实现，包括架构设计、核心组件、实现细节和最佳实践。

## 目录

1. [GAS 概述](#gas-概述)
2. [GAS 架构](#gas-架构)
3. [核心组件](#核心组件)
4. [GameplayTags 实现](#gameplaytags-实现)
5. [AttributeSet 实现](#attributeset-实现)
6. [GameplayEffect 实现](#gameplayeffect-实现)
7. [GameplayAbility 实现](#gameplayability-实现)
8. [GameplayCue 实现](#gameplaycue-实现)
9. [AbilitySystemComponent 扩展](#abilitysystemcomponent-扩展)
10. [伤害计算系统](#伤害计算系统)
11. [Debuff 系统](#debuff-系统)
12. [被动技能系统](#被动技能系统)
13. [网络复制](#网络复制)
14. [最佳实践](#最佳实践)
15. [常见问题](#常见问题)

---

## GAS 概述

### 什么是 GAS

Gameplay Ability System (GAS) 是 Unreal Engine 提供的一个强大的游戏系统框架，用于实现：

- **技能系统**: 角色技能和能力
- **属性系统**: 角色属性和状态
- **效果系统**: Buff/Debuff 效果
- **伤害系统**: 伤害计算和应用
- **网络同步**: 多人游戏支持

### Aura 项目中的 GAS

Aura 项目基于 UE5.7 的 GAS 构建，实现了：

- ✅ 完整的技能系统（投射物、光束、近战、召唤、被动）
- ✅ 完整的属性系统（主属性、次属性、抗性、生命值）
- ✅ 伤害计算系统（护甲、暴击、抗性、格挡）
- ✅ Debuff 系统（持续伤害、状态效果）
- ✅ 被动技能系统（自动激活、持续效果）
- ✅ UI 集成（MVVM 架构）
- ✅ 网络支持（多人游戏）

---

## GAS 架构

### 系统层次结构

```
┌─────────────────────────────────────────┐
│         UI Layer (MVVM)                 │
│  WidgetController → Widget               │
│  - 属性显示                               │
│  - 技能菜单                               │
│  - 状态更新                               │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│      Gameplay Ability System             │
│  ┌───────────────────────────────────┐   │
│  │  AbilitySystemComponent (ASC)      │   │
│  │  - 能力管理                         │   │
│  │  - 输入绑定                         │   │
│  │  - 状态管理                         │   │
│  └───────────────────────────────────┘   │
│  ┌───────────────────────────────────┐   │
│  │  AttributeSet                      │   │
│  │  - 属性存储                         │   │
│  │  - 属性计算                         │   │
│  │  - 属性限制                         │   │
│  └───────────────────────────────────┘   │
│  ┌───────────────────────────────────┐   │
│  │  GameplayAbility                   │   │
│  │  - 技能逻辑                         │   │
│  │  - 激活流程                         │   │
│  │  - 生命周期                         │   │
│  └───────────────────────────────────┘   │
│  ┌───────────────────────────────────┐   │
│  │  GameplayEffect                   │   │
│  │  - 属性修改                         │   │
│  │  - 效果应用                         │   │
│  │  - 持续时间                         │   │
│  └───────────────────────────────────┘   │
│  ┌───────────────────────────────────┐   │
│  │  GameplayCue                       │   │
│  │  - 视觉效果                         │   │
│  │  - 音效                             │   │
│  └───────────────────────────────────┘   │
└─────────────────────────────────────────┘
                  ↓
┌─────────────────────────────────────────┐
│         Character System                │
│  PlayerState → Character → Enemy         │
│  - ASC 拥有者                             │
│  - 属性初始化                             │
│  - 能力授予                               │
└─────────────────────────────────────────┘
```

### 数据流

```
角色初始化
    ↓
初始化 AttributeSet
    ↓
应用默认属性 GameplayEffect
    ↓
授予能力 (GiveAbility)
    ↓
能力激活 (ActivateAbility)
    ↓
应用效果 GameplayEffect
    ↓
修改属性 (AttributeSet)
    ↓
触发回调 (OnAttributeChanged)
    ↓
更新 UI (WidgetController)
```

---

## 核心组件

### 1. UAuraAbilitySystemComponent

扩展的 AbilitySystemComponent，提供：

- **输入绑定**: 将输入动作绑定到能力
- **状态管理**: Locked → Eligible → Unlocked → Equipped
- **槽位管理**: 管理能力槽位分配
- **被动能力**: 被动能力的激活/停用
- **属性升级**: 属性点升级系统

**关键方法**:

```cpp
// 输入处理
void AbilityInputTagPressed(const FGameplayTag& InputTag);
void AbilityInputTagHeld(const FGameplayTag& InputTag);
void AbilityInputTagReleased(const FGameplayTag& InputTag);

// 能力管理
void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);
void AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities);

// 状态管理
FGameplayTag GetStatusFromAbilityTag(const FGameplayTag& AbilityTag);
FGameplayTag GetInputTagFromSpec(const FGameplayAbilitySpec& Spec);

// 槽位管理
void AssignSlotToAbility(FGameplayAbilitySpec& Spec, const FGameplayTag& Slot);
void ClearSlot(FGameplayAbilitySpec* Spec);
```

### 2. UAuraAttributeSet

属性集类，管理所有游戏属性：

- **主属性**: Strength, Intelligence, Resilience, Vigor
- **次属性**: Armor, CriticalHit, Health/Mana Regeneration
- **抗性**: Fire, Lightning, Arcane, Physical Resistance
- **生命值**: Health, Mana, MaxHealth, MaxMana

**关键特性**:

- 自动访问器生成（ATTRIBUTE_ACCESSORS 宏）
- 属性限制（PreAttributeChange）
- 属性变化回调（PostAttributeChange）
- GameplayTag 映射（TagsToAttributes）

### 3. UAuraGameplayAbility

所有能力的基础类：

- **描述系统**: GetDescription(), GetNextLevelDescription()
- **成本系统**: GetManaCost(), GetCooldown()
- **输入绑定**: StartupInputTag

### 4. UAuraDamageGameplayAbility

伤害能力基类：

- **伤害计算**: CauseDamage()
- **伤害参数**: DamageEffectParams
- **Debuff 支持**: DebuffChance, DebuffDamage

---

## GameplayTags 实现

### Tag 结构

GameplayTags 使用层次化结构：

```
Attributes.Primary.Strength
Attributes.Secondary.MaxHealth
Abilities.Fire.FireBolt
Damage.Fire
Debuff.Burn
```

### FAuraGameplayTags 单例

```cpp
struct FAuraGameplayTags
{
    static const FAuraGameplayTags& Get() { return GameplayTags; }
    static void InitializeNativeGameplayTags();
    
    // 属性 Tags
    FGameplayTag Attributes_Primary_Strength;
    FGameplayTag Attributes_Vital_Health;
    
    // 能力 Tags
    FGameplayTag Abilities_Fire_FireBolt;
    
    // 伤害 Tags
    FGameplayTag Damage_Fire;
    
    // Debuff Tags
    FGameplayTag Debuff_Burn;
    
private:
    static FAuraGameplayTags GameplayTags;
};
```

### Tag 初始化

```cpp
void FAuraGameplayTags::InitializeNativeGameplayTags()
{
    GameplayTags.Attributes_Primary_Strength = 
        UGameplayTagsManager::Get().AddNativeGameplayTag(
            FName("Attributes.Primary.Strength"),
            FString("Increases physical damage")
        );
}
```

### Tag 映射

在 AttributeSet 构造函数中建立映射：

```cpp
UAuraAttributeSet::UAuraAttributeSet()
{
    const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
    TagsToAttributes.Add(
        GameplayTags.Attributes_Primary_Strength,
        GetStrengthAttribute
    );
}
```

---

## AttributeSet 实现

### 属性定义

```cpp
UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "Primary Attributes")
FGameplayAttributeData Strength;
ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Strength);
```

### 网络复制

```cpp
void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    DOREPLIFETIME_CONDITION_NOTIFY(
        UAuraAttributeSet,
        Strength,
        COND_None,
        REPNOTIFY_Always
    );
}
```

### 属性限制

```cpp
void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
    }
}
```

### 属性变化回调

```cpp
void UAuraAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    // 处理属性变化
}
```

---

## GameplayEffect 实现

### Effect 类型

#### 1. Instant Effect（立即效果）

```cpp
Duration Policy: Instant
Modifiers:
  - Attribute: Mana
  - Modifier Op: Subtract
  - Magnitude: SetByCaller(Data.ManaCost)
```

#### 2. HasDuration Effect（持续效果）

```cpp
Duration Policy: HasDuration
Duration Magnitude: 5.0
Period: 1.0
Modifiers:
  - Attribute: IncomingDamage
  - Modifier Op: Additive
  - Magnitude: 10.0
```

#### 3. Infinite Effect（无限效果）

```cpp
Duration Policy: Infinite
Modifiers:
  - Attribute: Armor
  - Modifier Op: Additive
  - Magnitude: 10.0
```

### Magnitude 计算

#### Scalable Float

```cpp
Magnitude Calculation Type: Scalable Float
Magnitude: 100.0
```

#### Attribute Based

```cpp
Magnitude Calculation Type: Attribute Based
Attribute: Attributes.Primary.Strength
Coefficient: 2.5
PreMultiplyAdditiveValue: 0.0
PostMultiplyAdditiveValue: 0.0
```

#### SetByCaller

```cpp
Magnitude Calculation Type: SetByCaller
SetByCaller Magnitude Data Name: Damage.Fire

// 在代码中设置
SpecHandle.Data->SetSetByCallerMagnitude(
    FAuraGameplayTags::Get().Damage_Fire,
    100.f
);
```

### Execution Calculation

```cpp
UCLASS()
class AURA_API UExecCalc_Damage : public UGameplayEffectExecutionCalculation
{
    GENERATED_BODY()
    
public:
    UExecCalc_Damage();
    
    virtual void Execute_Implementation(
        const FGameplayEffectCustomExecutionParameters& ExecutionParams,
        FGameplayEffectCustomExecutionOutput& OutExecutionOutput
    ) const override;
};
```

---

## GameplayAbility 实现

### 能力生命周期

```
CanActivateAbility
    ↓
ActivateAbility
    ↓
CommitAbility
    ↓
ExecuteAbility
    ↓
EndAbility
```

### 能力激活流程

```cpp
void UAuraGameplayAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData
)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    // 1. 检查成本
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    // 2. 执行能力逻辑
    ExecuteAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}
```

### 能力类层次

```
UGameplayAbility
    ↓
UAuraGameplayAbility
    ├── UAuraDamageGameplayAbility
    │   ├── UAuraProjectileSpell
    │   ├── UAuraBeamSpell
    │   └── UAuraMeleeAttack
    ├── UAuraSummonAbility
    └── UAuraPassiveAbility
```

---

## GameplayCue 实现

### Cue 类型

#### Static Cue（静态 Cue）

```cpp
// 在蓝图中配置
GameplayCue Tag: GameplayCue.FireBlast
OnExecute:
  - Spawn Particle System
  - Play Sound
```

#### Actor Cue（Actor Cue）

```cpp
// 在蓝图中配置
GameplayCue Tag: GameplayCue.ShockLoop
OnActive:
  - Spawn Actor
  - Attach to Target
OnRemove:
  - Destroy Actor
```

### Cue 执行

```cpp
// 非复制执行
FGameplayCueParameters CueParams;
CueParams.Location = GetActorLocation();
UGameplayCueManager::ExecuteGameplayCue_NonReplicated(
    GetOwner(),
    FAuraGameplayTags::Get().GameplayCue_FireBlast,
    CueParams
);

// 复制执行
ASC->ExecuteGameplayCue(
    FAuraGameplayTags::Get().GameplayCue_FireBlast,
    CueParams
);
```

---

## AbilitySystemComponent 扩展

### 输入绑定

```cpp
void UAuraAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
    if (!InputTag.IsValid()) return;
    
    FScopedAbilityListLock ActiveScopeLoc(*this);
    for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        if (AbilitySpec.DynamicAbilityTags.HasTagExact(InputTag))
        {
            AbilitySpecInputPressed(AbilitySpec);
            if (AbilitySpec.IsActive())
            {
                InvokeReplicatedEvent(
                    EAbilityGenericReplicatedEvent::InputPressed,
                    AbilitySpec.Handle,
                    AbilitySpec.ActivationInfo.GetActivationPredictionKey()
                );
            }
        }
    }
}
```

### 状态管理

```cpp
FGameplayTag UAuraAbilitySystemComponent::GetStatusFromAbilityTag(const FGameplayTag& AbilityTag)
{
    FScopedAbilityListLock ActiveScopeLoc(*this);
    for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
    {
        if (AbilitySpec.Ability->AbilityTags.HasTagExact(AbilityTag))
        {
            if (AbilitySpec.DynamicAbilityTags.HasTagExact(
                FAuraGameplayTags::Get().Abilities_Status_Equipped))
            {
                return FAuraGameplayTags::Get().Abilities_Status_Equipped;
            }
            // ... 其他状态检查
        }
    }
    return FAuraGameplayTags::Get().Abilities_Status_Locked;
}
```

### 槽位管理

```cpp
void UAuraAbilitySystemComponent::AssignSlotToAbility(
    FGameplayAbilitySpec& Spec,
    const FGameplayTag& Slot
)
{
    ClearSlot(&Spec);
    Spec.DynamicAbilityTags.AddTag(Slot);
}
```

---

## 伤害计算系统

### ExecCalc_Damage

```cpp
void UExecCalc_Damage::Execute_Implementation(
    const FGameplayEffectCustomExecutionParameters& ExecutionParams,
    FGameplayEffectCustomExecutionOutput& OutExecutionOutput
) const
{
    // 1. 获取源和目标
    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
    const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
    
    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = SourceTags;
    EvaluationParameters.TargetTags = TargetTags;
    
    // 2. 获取基础伤害
    float Damage = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
        DamageStatics().DamageDef,
        EvaluationParameters,
        Damage
    );
    
    // 3. 计算护甲减免
    float TargetArmor = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
        DamageStatics().ArmorDef,
        EvaluationParameters,
        TargetArmor
    );
    
    // 4. 计算抗性
    float Resistance = 0.f;
    // ... 根据伤害类型获取对应抗性
    
    // 5. 计算最终伤害
    Damage = FMath::Max<float>(Damage - TargetArmor, 0.f);
    Damage *= (1.f - Resistance / 100.f);
    
    // 6. 输出伤害
    OutExecutionOutput.AddOutputModifier(
        FGameplayModifierEvaluatedData(
            DamageStatics().IncomingDamageProperty,
            EGameplayModOp::Additive,
            Damage
        )
    );
}
```

### 伤害流程

```
应用伤害 GameplayEffect
    ↓
ExecCalc_Damage 执行
    ↓
计算护甲、抗性、暴击
    ↓
输出 IncomingDamage
    ↓
PostGameplayEffectExecute
    ↓
处理伤害（减少 Health）
    ↓
触发 Debuff（如果满足条件）
```

---

## Debuff 系统

### Debuff 类型

- **Burn**: 火焰持续伤害
- **Stun**: 眩晕效果
- **Arcane**: 奥术持续伤害
- **Physical**: 物理持续伤害

### Debuff 应用

```cpp
void UAuraAttributeSet::Debuff(const FEffectProperties& Props)
{
    // 1. 创建动态 GameplayEffect
    FString DebuffName = FString::Printf(TEXT("DynamicDebuff_%s"), *DamageType.ToString());
    UGameplayEffect* Effect = NewObject<UGameplayEffect>(
        GetTransientPackage(),
        FName(DebuffName)
    );
    
    // 2. 配置 Effect
    Effect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
    Effect->Period = DebuffFrequency;
    Effect->DurationMagnitude = FScalableFloat(DebuffDuration);
    
    // 3. 添加标签
    const FGameplayTag DebuffTag = GameplayTags.DamageTypesToDebuffs[DamageType];
    Effect->InheritableOwnedTagsContainer.AddTag(DebuffTag);
    
    // 4. 添加伤害修改器
    FGameplayModifierInfo& ModifierInfo = Effect->Modifiers.AddDefaulted_GetRef();
    ModifierInfo.ModifierMagnitude = FScalableFloat(DebuffDamage);
    ModifierInfo.ModifierOp = EGameplayModOp::Additive;
    ModifierInfo.Attribute = UAuraAttributeSet::GetIncomingDamageAttribute();
    
    // 5. 应用 Effect
    FGameplayEffectSpec* MutableSpec = new FGameplayEffectSpec(Effect, EffectContext, 1.f);
    Props.TargetASC->ApplyGameplayEffectSpecToSelf(*MutableSpec);
}
```

---

## 被动技能系统

### 被动技能特点

- **自动激活**: 在角色初始化时自动激活
- **持续效果**: 持续生效直到被停用
- **不可手动触发**: 不绑定输入动作
- **通过 GameplayEffect 实现效果**: 效果通过 GameplayEffect 配置

### 被动技能实现

```cpp
void UAuraPassiveAbility::ActivateAbility(...)
{
    Super::ActivateAbility(...);
    
    // 绑定停用委托
    if (UAuraAbilitySystemComponent* AuraASC = ...)
    {
        AuraASC->DeactivatePassiveAbility.AddUObject(
            this,
            &UAuraPassiveAbility::ReceiveDeactivate
        );
    }
}
```

### 被动技能添加

```cpp
void UAuraAbilitySystemComponent::AddCharacterPassiveAbilities(
    const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities
)
{
    for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
    {
        FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
        AbilitySpec.DynamicAbilityTags.AddTag(
            FAuraGameplayTags::Get().Abilities_Status_Equipped
        );
        GiveAbilityAndActivateOnce(AbilitySpec);
    }
}
```

---

## 网络复制

### 复制策略

#### 服务器权威

- 属性修改在服务器执行
- 能力激活在服务器验证
- 伤害计算在服务器执行

#### 客户端预测

- 使用 GAS 的预测系统
- 客户端预测能力激活
- 服务器验证和修正

### 复制配置

```cpp
// AttributeSet 复制
DOREPLIFETIME_CONDITION_NOTIFY(
    UAuraAttributeSet,
    Health,
    COND_None,
    REPNOTIFY_Always
);

// ASC 复制模式
Mixed Replication Mode
- Owner: Full Replication
- Autonomous: Full Replication
- Simulated: Minimal Replication
```

---

## 最佳实践

### 1. 属性设计

- **分类清晰**: 明确区分主属性、次属性、抗性等
- **命名规范**: 使用清晰的命名
- **文档完善**: 为每个属性添加描述

### 2. 能力设计

- **单一职责**: 每个能力只实现一个功能
- **可复用**: 设计可复用的能力基类
- **配置驱动**: 使用数据资产配置能力

### 3. 效果设计

- **最小化 Effect**: 避免创建过多 Effect
- **使用 SetByCaller**: 对于动态值使用 SetByCaller
- **合理使用 Execution**: 只在必要时使用 Execution Calculation

### 4. 性能优化

- **最小化复制**: 只复制必要的属性
- **批量更新**: 使用 GameplayEffect 批量修改属性
- **缓存计算**: 缓存频繁计算的属性值

### 5. 网络优化

- **服务器权威**: 属性修改在服务器执行
- **客户端预测**: 使用 GAS 的预测系统
- **合理复制**: 避免不必要的网络复制

---

## 常见问题

### 问题 1: 属性未正确初始化

**原因**: GameplayEffect 未正确应用

**解决方案**: 
1. 检查 `InitializeDefaultAttributes()` 是否调用
2. 检查 GameplayEffect 配置是否正确
3. 检查角色蓝图中的默认值设置

### 问题 2: 能力无法激活

**原因**: 成本不足或冷却未完成

**解决方案**: 
1. 检查 `CheckCost()` 返回值
2. 检查 `CheckCooldown()` 返回值
3. 检查能力状态是否为 Equipped

### 问题 3: 伤害未正确计算

**原因**: ExecCalc 未正确配置

**解决方案**: 
1. 检查 Execution Calculation 是否设置
2. 检查属性捕获是否正确
3. 检查伤害输出是否正确

### 问题 4: Debuff 未应用

**原因**: Debuff 条件未满足

**解决方案**: 
1. 检查 DebuffChance 是否满足
2. 检查 Debuff 标签是否正确
3. 检查 Effect 创建是否正确

### 问题 5: 网络同步问题

**原因**: 复制未正确配置

**解决方案**: 
1. 检查 `GetLifetimeReplicatedProps()` 是否添加
2. 检查 `OnRep` 函数是否正确实现
3. 确保在服务器上修改属性

---

## 总结

GAS 是 Aura 项目的核心系统，提供了：

- ✅ **完整的技能系统**: 多种技能类型支持
- ✅ **完整的属性系统**: 主属性、次属性、抗性
- ✅ **伤害计算系统**: 护甲、抗性、暴击、格挡
- ✅ **Debuff 系统**: 持续伤害和状态效果
- ✅ **被动技能系统**: 自动激活和持续效果
- ✅ **网络支持**: 完整的多人游戏支持

通过合理使用 GAS，可以构建功能强大且易于维护的游戏系统。

---

## 相关文档

- [技能系统文档](./Ability_System.md) - 技能系统详细文档
- [属性系统文档](../Systems/Attribute_System.md) - 属性系统详细文档
- [GameplayTags 系统文档](../Systems/GameplayTags_System.md) - GameplayTags 详细文档
- [GameplayEffect 系统文档](../Systems/GameplayEffect_System.md) - GameplayEffect 详细文档
- [GameplayAbility 系统文档](../Systems/GameplayAbility_System.md) - GameplayAbility 详细文档
- [GameplayCue 系统文档](../Systems/GameplayCue_System.md) - GameplayCue 详细文档
- [被动技能系统文档](../Systems/Passive_Ability_System.md) - 被动技能系统详细文档
- [伤害计算系统文档](../Systems/Damage_Calculation.md) - 伤害计算详细文档
- [Debuff 系统文档](../Systems/Debuff_System.md) - Debuff 系统详细文档

