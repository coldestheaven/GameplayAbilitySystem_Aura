# GameplayEffect 系统详细文档

## 目录

1. [系统概述](#系统概述)
2. [GameplayEffect 基础](#gameplayeffect-基础)
3. [持续时间策略](#持续时间策略)
4. [修改器（Modifiers）](#修改器modifiers)
5. [修改量计算（Magnitude Calculation）](#修改量计算magnitude-calculation)
6. [执行计算（Execution Calculation）](#执行计算execution-calculation)
7. [标签系统](#标签系统)
8. [堆叠系统](#堆叠系统)
9. [项目中的 GameplayEffect 类型](#项目中的-gameplayeffect-类型)
10. [创建 GameplayEffect](#创建-gameplayeffect)
11. [应用 GameplayEffect](#应用-gameplayeffect)
12. [动态创建 GameplayEffect](#动态创建-gameplayeffect)
13. [最佳实践](#最佳实践)
14. [常见问题](#常见问题)

---

## 系统概述

GameplayEffect 是 GAS 系统中用于修改属性、应用状态效果的核心组件。在 Aura 项目中，GameplayEffect 用于：

- **属性修改**: 修改角色的各种属性（生命值、法力值、护甲等）
- **技能消耗**: 实现技能的消耗（法力消耗）
- **技能冷却**: 实现技能的冷却时间
- **伤害应用**: 对目标造成伤害
- **Debuff 应用**: 应用持续伤害和状态效果
- **Buff 应用**: 应用增益效果

### 核心组件

- **UGameplayEffect**: GameplayEffect 基类
- **FGameplayEffectSpec**: Effect 规格，包含应用时的具体参数
- **FGameplayModifierInfo**: 修改器信息
- **FGameplayEffectContextHandle**: Effect 上下文句柄
- **FActiveGameplayEffectHandle**: 激活的 Effect 句柄

---

## GameplayEffect 基础

### GameplayEffect 结构

GameplayEffect 包含以下主要部分：

1. **持续时间策略** (Duration Policy): 定义 Effect 的持续时间类型
2. **修改器** (Modifiers): 定义如何修改属性
3. **标签** (Tags): 定义 Effect 的标签
4. **条件** (Conditions): 定义应用条件
5. **堆叠** (Stacking): 定义堆叠行为

### Effect 生命周期

```
创建 Effect Spec
    ↓
检查条件
    ↓
应用修改器
    ↓
执行计算（如果有）
    ↓
更新属性
    ↓
触发回调
    ↓
持续/周期执行（如果有）
    ↓
移除 Effect
```

---

## 持续时间策略

### 持续时间类型

GameplayEffect 支持三种持续时间策略：

#### 1. Instant（立即生效）

- **特点**: 立即应用并移除，不持续
- **用途**: 伤害、消耗、一次性效果
- **配置**:
  - `Duration Policy`: `Instant`
  - `Period`: 0

**示例**: 法力消耗 Effect

```cpp
// 在蓝图中配置
Duration Policy: Instant
Period: 0
Modifiers:
  - Attribute: Mana
  - Modifier Op: Subtract
  - Magnitude: SetByCaller(Data.ManaCost)
```

#### 2. HasDuration（有持续时间）

- **特点**: 持续一段时间后自动移除
- **用途**: 冷却、临时 Buff/Debuff
- **配置**:
  - `Duration Policy`: `HasDuration`
  - `Duration Magnitude`: 持续时间值
  - `Period`: 周期执行间隔（可选）

**示例**: 冷却 Effect

```cpp
// 在蓝图中配置
Duration Policy: HasDuration
Duration Magnitude: 5.0 (5秒)
Period: 0
Granted Tags: Cooldown.Fire.FireBolt
```

#### 3. Infinite（无限持续）

- **特点**: 持续存在直到手动移除
- **用途**: 永久 Buff、光环效果
- **配置**:
  - `Duration Policy`: `Infinite`
  - `Period`: 周期执行间隔（可选）

**示例**: 光环保护 Effect

```cpp
// 在蓝图中配置
Duration Policy: Infinite
Period: 0
Modifiers:
  - Attribute: Armor
  - Modifier Op: Additive
  - Magnitude: 10.0
```

### 周期执行（Period）

周期执行允许 Effect 定期触发：

- **Period > 0**: Effect 每隔 Period 秒执行一次
- **Period = 0**: Effect 只在应用时执行一次

**示例**: Debuff 持续伤害

```cpp
Duration Policy: HasDuration
Duration Magnitude: 10.0 (持续10秒)
Period: 1.0 (每秒触发一次)
Modifiers:
  - Attribute: IncomingDamage
  - Modifier Op: Additive
  - Magnitude: 5.0 (每次5点伤害)
```

---

## 修改器（Modifiers）

### 修改器结构

修改器定义如何修改属性：

```cpp
struct FGameplayModifierInfo
{
    FGameplayAttribute Attribute;        // 要修改的属性
    EGameplayModOp ModifierOp;          // 修改操作类型
    FGameplayEffectModifierMagnitude ModifierMagnitude;  // 修改量
    FGameplayModEvaluationChannelSettings EvaluationChannelSettings;  // 评估通道
};
```

### 修改操作类型

#### 1. Additive（加法）

- **公式**: `NewValue = CurrentValue + Magnitude`
- **用途**: 增加属性值（如增加生命值、护甲）

**示例**: 增加护甲

```cpp
Attribute: Armor
Modifier Op: Additive
Magnitude: 10.0
// 结果: Armor = Armor + 10
```

#### 2. Multiply（乘法）

- **公式**: `NewValue = CurrentValue * Magnitude`
- **用途**: 按比例修改属性（如增加 50% 伤害）

**示例**: 增加 50% 伤害

```cpp
Attribute: Damage
Modifier Op: Multiply
Magnitude: 1.5
// 结果: Damage = Damage * 1.5
```

#### 3. Override（覆盖）

- **公式**: `NewValue = Magnitude`
- **用途**: 直接设置属性值（如设置最大生命值）

**示例**: 设置最大生命值

```cpp
Attribute: MaxHealth
Modifier Op: Override
Magnitude: 100.0
// 结果: MaxHealth = 100
```

#### 4. Divide（除法）

- **公式**: `NewValue = CurrentValue / Magnitude`
- **用途**: 按比例减少属性

**示例**: 减少 50% 移动速度

```cpp
Attribute: MovementSpeed
Modifier Op: Divide
Magnitude: 2.0
// 结果: MovementSpeed = MovementSpeed / 2
```

---

## 修改量计算（Magnitude Calculation）

修改量可以通过多种方式计算：

### 1. Scalable Float（可扩展浮点数）

- **特点**: 基于等级的值
- **用途**: 随等级变化的属性修改
- **配置**: 使用曲线表（Curve Table）定义不同等级的值

**示例**: 伤害随等级增加

```cpp
Magnitude Calculation Type: Scalable Float
Scalable Float Value: 
  - Level 1: 50
  - Level 2: 75
  - Level 3: 100
  - ...
```

### 2. Attribute Based（基于属性）

- **特点**: 基于另一个属性的值
- **用途**: 基于属性计算修改量（如基于力量计算伤害）

**示例**: 伤害基于力量

```cpp
Magnitude Calculation Type: Attribute Based
Attribute: Strength
Coefficient: 1.0
Pre Multiply Additive Value: 0.0
Post Multiply Additive Value: 0.0
// 结果: Damage = Strength * 1.0
```

### 3. Custom Calculation Class（自定义计算类）

- **特点**: 使用自定义的 `UGameplayModMagnitudeCalculation` 类
- **用途**: 复杂的计算逻辑

**示例**: 基于多个属性计算

```cpp
// 在 UMMC_MaxHealth 中
float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(
    const FGameplayEffectSpec& Spec) const
{
    const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
    
    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = SourceTags;
    EvaluationParameters.TargetTags = TargetTags;
    
    float Vigor = 0.f;
    GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluationParameters, Vigor);
    Vigor = FMath::Max<float>(Vigor, 0.f);
    
    float Strength = 0.f;
    GetCapturedAttributeMagnitude(StrengthDef, Spec, EvaluationParameters, Strength);
    Strength = FMath::Max<float>(Strength, 0.f);
    
    return 80.f + 2.5f * Vigor + 10.f * Strength;
}
```

### 4. SetByCaller（由调用者设置）

- **特点**: 在运行时由调用者设置值
- **用途**: 动态值（如技能伤害、消耗）

**示例**: 技能伤害

```cpp
Magnitude Calculation Type: SetByCaller
SetByCaller Magnitude: Damage.Fire
// 在技能中设置:
SpecHandle.Data->SetSetByCallerMagnitude(
    FAuraGameplayTags::Get().Damage_Fire,
    100.f
);
```

---

## 执行计算（Execution Calculation）

执行计算允许在 Effect 应用时执行自定义逻辑。

### ExecCalc_Damage

项目中的自定义伤害计算器，处理：

- 护甲计算
- 暴击计算
- 抗性计算
- 格挡计算
- Debuff 触发

**使用示例**:

```cpp
// 在伤害 GameplayEffect 中
Execution Calculation: ExecCalc_Damage
Modifiers:
  - Attribute: IncomingDamage
  - Modifier Op: Additive
  - Magnitude: SetByCaller(Damage.Fire)
```

### 创建自定义执行计算

```cpp
// MyExecCalc.h
UCLASS()
class AURA_API UMyExecCalc : public UGameplayEffectExecutionCalculation
{
    GENERATED_BODY()

public:
    UMyExecCalc();

    virtual void Execute_Implementation(
        const FGameplayEffectCustomExecutionParameters& ExecutionParams,
        FGameplayEffectCustomExecutionOutput& OutExecutionOutput
    ) const override;
};
```

---

## 标签系统

### Asset Tags（资产标签）

Effect 自身的标签，用于标识 Effect 类型：

```cpp
Asset Tags:
  - Damage.Fire
  - Effects.HitReact
```

### Granted Tags（授予标签）

Effect 应用时授予目标的标签：

```cpp
Granted Tags:
  - Cooldown.Fire.FireBolt
  - Debuff.Burn
```

### Ongoing Tag Requirements（持续标签要求）

Effect 持续期间需要的标签：

```cpp
Ongoing Tag Requirements:
  - Require: Abilities.Status.Equipped
  - Ignore: Abilities.Status.Locked
```

### Application Tag Requirements（应用标签要求）

Effect 应用时需要的标签：

```cpp
Application Tag Requirements:
  - Require: Attributes.Vital.Health > 0
  - Ignore: Debuff.Stun
```

---

## 堆叠系统

### 堆叠类型

#### 1. None（不堆叠）

- **特点**: 同一 Effect 只能存在一个实例
- **行为**: 新实例会替换旧实例

#### 2. Aggregate by Source（按来源聚合）

- **特点**: 来自同一来源的 Effect 聚合
- **行为**: 多个来源的 Effect 分别计算

**示例**: Debuff

```cpp
Stacking Type: Aggregate by Source
Stack Limit Count: 1
// 来自不同来源的 Debuff 可以同时存在
```

#### 3. Aggregate by Target（按目标聚合）

- **特点**: 所有来源的 Effect 聚合
- **行为**: 所有 Effect 合并计算

### 堆叠限制

```cpp
Stack Limit Count: 5
// 最多堆叠 5 层
```

---

## 项目中的 GameplayEffect 类型

### 1. Cost GameplayEffect（消耗 Effect）

**用途**: 技能激活时的消耗（如法力消耗）

**配置**:
- Duration Policy: `Instant`
- Modifiers:
  - Attribute: `Mana`
  - Modifier Op: `Subtract`
  - Magnitude: `SetByCaller(Data.ManaCost)`

**示例**: `GE_FireBolt_Cost`

### 2. Cooldown GameplayEffect（冷却 Effect）

**用途**: 技能冷却时间

**配置**:
- Duration Policy: `HasDuration`
- Duration Magnitude: 冷却时间（如 5.0 秒）
- Granted Tags: 冷却标签（如 `Cooldown.Fire.FireBolt`）

**示例**: `GE_FireBolt_Cooldown`

### 3. Damage GameplayEffect（伤害 Effect）

**用途**: 对目标造成伤害

**配置**:
- Duration Policy: `Instant`
- Execution Calculation: `ExecCalc_Damage`
- Modifiers:
  - Attribute: `IncomingDamage`
  - Modifier Op: `Additive`
  - Magnitude: `SetByCaller(Damage.Fire)`

**示例**: `GE_FireBolt_Damage`

### 4. Debuff GameplayEffect（Debuff Effect）

**用途**: 持续伤害和状态效果

**配置**:
- Duration Policy: `HasDuration`
- Duration Magnitude: Debuff 持续时间
- Period: Debuff 触发频率
- Granted Tags: Debuff 标签（如 `Debuff.Burn`）
- Modifiers:
  - Attribute: `IncomingDamage`
  - Modifier Op: `Additive`
  - Magnitude: Debuff 伤害值

**示例**: 动态创建的 Debuff Effect

### 5. Buff GameplayEffect（Buff Effect）

**用途**: 增益效果

**配置**:
- Duration Policy: `HasDuration` 或 `Infinite`
- Modifiers: 根据 Buff 类型配置

**示例**: `GE_HaloOfProtection`

---

## 创建 GameplayEffect

### 在编辑器中创建

1. **创建 GameplayEffect 资产**:
   - 右键点击内容浏览器
   - 选择 `Gameplay` → `Gameplay Effect`
   - 命名为 `GE_MyEffect`

2. **配置基本属性**:
   - Duration Policy
   - Duration Magnitude
   - Period

3. **添加修改器**:
   - 点击 "Add Modifier"
   - 选择属性
   - 设置修改操作和修改量

4. **配置标签**:
   - Asset Tags
   - Granted Tags
   - Ongoing Tag Requirements

5. **配置堆叠**（如果需要）:
   - Stacking Type
   - Stack Limit Count

### 在 C++ 中创建

```cpp
// 创建动态 Effect
UGameplayEffect* Effect = NewObject<UGameplayEffect>(
    GetTransientPackage(),
    FName("MyDynamicEffect")
);

// 配置持续时间
Effect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
Effect->DurationMagnitude = FScalableFloat(10.0f);
Effect->Period = 1.0f;

// 添加修改器
FGameplayModifierInfo& ModifierInfo = Effect->Modifiers.AddDefaulted_GetRef();
ModifierInfo.Attribute = UAuraAttributeSet::GetHealthAttribute();
ModifierInfo.ModifierOp = EGameplayModOp::Additive;
ModifierInfo.ModifierMagnitude = FScalableFloat(10.0f);

// 添加标签
Effect->InheritableOwnedTagsContainer.AddTag(
    FAuraGameplayTags::Get().Debuff_Burn
);
```

---

## 应用 GameplayEffect

### 基本应用

```cpp
// 创建 Effect Spec
FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
EffectContext.AddSourceObject(SourceActor);

FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
    GameplayEffectClass,
    Level,
    EffectContext
);

// 设置 SetByCaller 值（如果需要）
SpecHandle.Data->SetSetByCallerMagnitude(
    FAuraGameplayTags::Get().Damage_Fire,
    100.f
);

// 应用 Effect
FActiveGameplayEffectHandle ActiveHandle = ASC->ApplyGameplayEffectSpecToSelf(
    *SpecHandle.Data.Get()
);
```

### 应用到目标

```cpp
// 应用到其他目标
FActiveGameplayEffectHandle ActiveHandle = TargetASC->ApplyGameplayEffectSpecToTarget(
    *SpecHandle.Data.Get(),
    TargetASC
);
```

### 移除 Effect

```cpp
// 移除 Effect
ASC->RemoveActiveGameplayEffect(ActiveHandle);
```

---

## 动态创建 GameplayEffect

项目中的 Debuff 系统使用动态创建的 GameplayEffect：

```cpp
void UAuraAttributeSet::Debuff(const FEffectProperties& Props)
{
    // 创建动态 Effect
    FString DebuffName = FString::Printf(TEXT("DynamicDebuff_%s"), *DamageType.ToString());
    UGameplayEffect* Effect = NewObject<UGameplayEffect>(
        GetTransientPackage(),
        FName(DebuffName)
    );
    
    // 配置持续时间
    Effect->DurationPolicy = EGameplayEffectDurationType::HasDuration;
    Effect->Period = DebuffFrequency;
    Effect->DurationMagnitude = FScalableFloat(DebuffDuration);
    
    // 添加 Debuff Tag
    const FGameplayTag DebuffTag = GameplayTags.DamageTypesToDebuffs[DamageType];
    Effect->InheritableOwnedTagsContainer.AddTag(DebuffTag);
    
    // 配置堆叠
    Effect->StackingType = EGameplayEffectStackingType::AggregateBySource;
    Effect->StackLimitCount = 1;
    
    // 添加修改器
    FGameplayModifierInfo& ModifierInfo = Effect->Modifiers.AddDefaulted_GetRef();
    ModifierInfo.ModifierMagnitude = FScalableFloat(DebuffDamage);
    ModifierInfo.ModifierOp = EGameplayModOp::Additive;
    ModifierInfo.Attribute = UAuraAttributeSet::GetIncomingDamageAttribute();
    
    // 创建 Spec 并应用
    FGameplayEffectContextHandle EffectContext = Props.SourceASC->MakeEffectContext();
    EffectContext.AddSourceObject(Props.SourceAvatarActor);
    
    if (FGameplayEffectSpec* MutableSpec = new FGameplayEffectSpec(Effect, EffectContext, 1.f))
    {
        // 设置伤害类型
        FAuraGameplayEffectContext* AuraContext = 
            static_cast<FAuraGameplayEffectContext*>(MutableSpec->GetContext().Get());
        TSharedPtr<FGameplayTag> DebuffDamageType = MakeShareable(new FGameplayTag(DamageType));
        AuraContext->SetDamageType(DebuffDamageType);
        
        // 应用效果
        Props.TargetASC->ApplyGameplayEffectSpecToSelf(*MutableSpec);
    }
}
```

---

## 最佳实践

### 1. Effect 设计

- **单一职责**: 每个 Effect 只负责一个功能
- **可复用性**: 设计可复用的 Effect
- **清晰命名**: 使用清晰的命名（如 `GE_FireBolt_Cost`）

### 2. 修改器配置

- **选择合适的操作**: 根据需求选择 Additive、Multiply、Override
- **使用 SetByCaller**: 对于动态值，使用 SetByCaller
- **使用 Scalable Float**: 对于等级相关值，使用 Scalable Float

### 3. 标签使用

- **合理使用标签**: 使用标签标识 Effect 类型
- **避免标签冲突**: 确保标签不冲突
- **使用 Granted Tags**: 使用 Granted Tags 标识状态

### 4. 性能考虑

- **避免频繁创建**: 缓存 Effect Spec
- **使用对象池**: 对于动态 Effect，考虑使用对象池
- **优化修改器**: 减少不必要的修改器

---

## 常见问题

### 问题 1: Effect 未应用

**原因**: 条件不满足或标签冲突

**解决方案**:
1. 检查 Application Tag Requirements
2. 检查 Ongoing Tag Requirements
3. 检查目标是否有阻塞标签

### 问题 2: 修改量不正确

**原因**: 修改量计算配置错误

**解决方案**:
1. 检查 Magnitude Calculation Type
2. 检查 SetByCaller 标签是否正确
3. 检查 Scalable Float 曲线表

### 问题 3: Effect 未移除

**原因**: 持续时间配置错误或手动移除失败

**解决方案**:
1. 检查 Duration Policy
2. 检查 Duration Magnitude
3. 确保正确调用 RemoveActiveGameplayEffect

### 问题 4: 堆叠行为不符合预期

**原因**: 堆叠配置错误

**解决方案**:
1. 检查 Stacking Type
2. 检查 Stack Limit Count
3. 检查 Granted Tags 是否相同

---

## 总结

GameplayEffect 是 GAS 系统的核心组件，用于：

- ✅ **修改属性**: 通过各种修改器修改属性
- ✅ **应用效果**: 应用 Buff、Debuff、伤害等效果
- ✅ **技能系统**: 实现技能消耗和冷却
- ✅ **动态效果**: 动态创建和应用效果

通过合理使用 GameplayEffect，可以实现灵活、可扩展的游戏系统。

---

## 相关文档

- [GameplayTags 系统文档](./GameplayTags_System.md) - GameplayTags 详细文档
- [GameplayAbility 系统文档](./GameplayAbility_System.md) - GameplayAbility 详细文档
- [属性系统文档](./Attribute_System.md) - 属性系统实现
- [伤害计算系统文档](./Damage_Calculation.md) - 伤害计算实现
- [Debuff 系统文档](./Debuff_System.md) - Debuff 系统实现

