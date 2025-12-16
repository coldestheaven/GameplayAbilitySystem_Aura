# 修正量计算器系统文档

## 概述

修正量计算器（Modifier Magnitude Calculation, ModMagCalc）用于在 GameplayEffect 中动态计算属性修正量。系统使用 `UGameplayModMagnitudeCalculation` 基类，可以根据其他属性的值来计算目标属性的修正量。

## 核心组件

### UMMC_MaxHealth

最大生命值修正量计算器，根据 Vigor（活力）属性计算最大生命值。

#### 类层次结构

```
UGameplayModMagnitudeCalculation (UE5 Base)
    ↓
UMMC_MaxHealth
```

#### 核心功能

根据 Vigor 属性计算最大生命值：

```
MaxHealth = BaseValue + (Vigor * Coefficient)
```

#### 关键实现

```cpp
UMMC_MaxHealth::UMMC_MaxHealth()
{
    // 定义属性捕获
    VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();
    VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
    VigorDef.bSnapshot = false;
    
    // 添加到捕获定义列表
    RelevantAttributesToCapture.Add(VigorDef);
}

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(
    const FGameplayEffectSpec& Spec
) const
{
    // 获取 Vigor 值
    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
    
    float Vigor = 0.f;
    GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluationParameters, Vigor);
    
    // 计算最大生命值
    // 可以从曲线表或数据资产中获取系数
    const float Coefficient = 10.f;  // 示例：每点 Vigor 增加 10 点生命值
    const float BaseValue = 100.f;   // 基础生命值
    
    return BaseValue + (Vigor * Coefficient);
}
```

---

### UMMC_MaxMana

最大法力值修正量计算器，根据 Intelligence（智力）属性计算最大法力值。

#### 实现

```cpp
UMMC_MaxMana::UMMC_MaxMana()
{
    // 定义属性捕获
    IntelligenceDef.AttributeToCapture = UAuraAttributeSet::GetIntelligenceAttribute();
    IntelligenceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
    IntelligenceDef.bSnapshot = false;
    
    RelevantAttributesToCapture.Add(IntelligenceDef);
}

float UMMC_MaxMana::CalculateBaseMagnitude_Implementation(
    const FGameplayEffectSpec& Spec
) const
{
    // 获取 Intelligence 值
    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
    
    float Intelligence = 0.f;
    GetCapturedAttributeMagnitude(IntelligenceDef, Spec, EvaluationParameters, Intelligence);
    
    // 计算最大法力值
    const float Coefficient = 10.f;
    const float BaseValue = 50.f;
    
    return BaseValue + (Intelligence * Coefficient);
}
```

---

## 属性捕获

### 捕获源

```cpp
enum class EGameplayEffectAttributeCaptureSource
{
    Source,  // 从效果来源捕获
    Target   // 从效果目标捕获
};
```

### 快照

```cpp
VigorDef.bSnapshot = false;  // 实时捕获
VigorDef.bSnapshot = true;   // 快照捕获（在效果应用时捕获）
```

**快照 vs 实时**:
- **快照**: 在效果应用时捕获属性值，之后不再更新
- **实时**: 每次计算时都重新捕获属性值

---

## 使用流程

### 1. 创建 ModMagCalc 类

```cpp
UCLASS()
class AURA_API UMMC_CustomAttribute : public UGameplayModMagnitudeCalculation
{
    GENERATED_BODY()
    
public:
    UMMC_CustomAttribute();
    
    virtual float CalculateBaseMagnitude_Implementation(
        const FGameplayEffectSpec& Spec
    ) const override;
    
private:
    FGameplayEffectAttributeCaptureDefinition AttributeDef;
};
```

### 2. 定义属性捕获

```cpp
UMMC_CustomAttribute::UMMC_CustomAttribute()
{
    AttributeDef.AttributeToCapture = UAuraAttributeSet::GetStrengthAttribute();
    AttributeDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
    AttributeDef.bSnapshot = false;
    
    RelevantAttributesToCapture.Add(AttributeDef);
}
```

### 3. 实现计算逻辑

```cpp
float UMMC_CustomAttribute::CalculateBaseMagnitude_Implementation(
    const FGameplayEffectSpec& Spec
) const
{
    // 获取捕获的属性值
    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
    
    float AttributeValue = 0.f;
    GetCapturedAttributeMagnitude(AttributeDef, Spec, EvaluationParameters, AttributeValue);
    
    // 执行计算
    return YourCalculation(AttributeValue);
}
```

### 4. 在 GameplayEffect 中使用

1. 创建 GameplayEffect
2. 添加 Modifier
3. 设置 **Magnitude Calculation Type** 为 `Attribute Based`
4. 选择创建的 ModMagCalc 类

---

## 高级用法

### 捕获多个属性

```cpp
UMMC_Advanced::UMMC_Advanced()
{
    // 捕获第一个属性
    StrengthDef.AttributeToCapture = UAuraAttributeSet::GetStrengthAttribute();
    StrengthDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
    RelevantAttributesToCapture.Add(StrengthDef);
    
    // 捕获第二个属性
    IntelligenceDef.AttributeToCapture = UAuraAttributeSet::GetIntelligenceAttribute();
    IntelligenceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
    RelevantAttributesToCapture.Add(IntelligenceDef);
}

float UMMC_Advanced::CalculateBaseMagnitude_Implementation(
    const FGameplayEffectSpec& Spec
) const
{
    float Strength = 0.f;
    float Intelligence = 0.f;
    
    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
    
    GetCapturedAttributeMagnitude(StrengthDef, Spec, EvaluationParameters, Strength);
    GetCapturedAttributeMagnitude(IntelligenceDef, Spec, EvaluationParameters, Intelligence);
    
    // 使用两个属性计算
    return (Strength + Intelligence) * 0.5f;
}
```

### 使用曲线表

```cpp
float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(
    const FGameplayEffectSpec& Spec
) const
{
    float Vigor = 0.f;
    // ... 获取 Vigor 值 ...
    
    // 从曲线表获取系数
    UCharacterClassInfo* CharacterClassInfo = 
        UAuraAbilitySystemLibrary::GetCharacterClassInfo(GetWorld());
    
    if (CharacterClassInfo && CharacterClassInfo->DamageCalculationCoefficients)
    {
        const float Coefficient = CharacterClassInfo->DamageCalculationCoefficients
            ->Eval(Vigor, 0.f);
        
        return BaseValue + (Vigor * Coefficient);
    }
    
    return BaseValue;
}
```

### 使用 GameplayTag

```cpp
float UMMC_Tagged::CalculateBaseMagnitude_Implementation(
    const FGameplayEffectSpec& Spec
) const
{
    // 检查标签
    if (Spec.CapturedSourceTags.GetAggregatedTags().HasTag(
        FGameplayTag::RequestGameplayTag(FName("Status.Buff.PowerUp"))
    ))
    {
        // 有 Buff 时使用不同的计算
        return EnhancedCalculation();
    }
    
    return NormalCalculation();
}
```

---

## 性能考虑

### 快照 vs 实时

- **快照**: 性能更好，但值不会更新
- **实时**: 值会更新，但性能开销更大

**建议**:
- 对于不经常变化的属性，使用快照
- 对于需要实时更新的属性，使用实时捕获

### 缓存计算结果

如果计算很复杂，可以考虑缓存结果：

```cpp
// 在 ModMagCalc 中
mutable float CachedValue = 0.f;
mutable float CachedVigor = -1.f;

float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(
    const FGameplayEffectSpec& Spec
) const
{
    float Vigor = 0.f;
    // ... 获取 Vigor 值 ...
    
    // 如果 Vigor 值没变，返回缓存值
    if (FMath::IsNearlyEqual(Vigor, CachedVigor))
    {
        return CachedValue;
    }
    
    // 计算新值
    CachedValue = BaseValue + (Vigor * Coefficient);
    CachedVigor = Vigor;
    
    return CachedValue;
}
```

---

## 相关文档

- [属性系统](./Attribute_System.md) - 属性定义和使用
- [技能系统](../Core/Ability_System.md) - GameplayEffect 使用

---

## 总结

ModMagCalc 系统提供了：

1. ✅ **动态计算** - 根据其他属性值计算修正量
2. ✅ **属性捕获** - 从来源或目标捕获属性
3. ✅ **灵活配置** - 支持快照和实时捕获
4. ✅ **复杂计算** - 支持多属性、曲线表、标签等
5. ✅ **性能优化** - 支持快照和缓存

通过这个系统，可以创建复杂的属性依赖关系，实现数据驱动的游戏平衡。

