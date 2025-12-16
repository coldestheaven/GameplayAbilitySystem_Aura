# 伤害计算系统详细文档

## 目录

1. [系统概述](#系统概述)
2. [伤害计算流程](#伤害计算流程)
3. [ExecCalc_Damage 实现](#execcalc_damage-实现)
4. [属性捕获](#属性捕获)
5. [伤害类型系统](#伤害类型系统)
6. [护甲计算](#护甲计算)
7. [暴击计算](#暴击计算)
8. [抗性计算](#抗性计算)
9. [格挡计算](#格挡计算)
10. [范围伤害](#范围伤害)
11. [等级系数](#等级系数)
12. [使用示例](#使用示例)

---

## 系统概述

伤害计算系统使用自定义的 `UExecCalc_Damage` 类来实现复杂的伤害计算逻辑，包括护甲、暴击、抗性、格挡等多种因素。

### 核心组件

- **UExecCalc_Damage**: 自定义伤害计算执行器
- **AuraDamageStatics**: 静态属性捕获定义
- **CharacterClassInfo**: 职业信息数据资产（包含伤害计算系数）

### 系统特点

- ✅ 多因素伤害计算
- ✅ 属性捕获系统
- ✅ 等级系数支持
- ✅ 范围伤害支持
- ✅ Debuff 集成

---

## 伤害计算流程

### 完整计算流程

```
1. 获取基础伤害值（从 SetByCaller）
   ↓
2. 确定 Debuff（在伤害计算前）
   ↓
3. 应用抗性减免
   Damage = Damage * (100 - Resistance) / 100
   ↓
4. 处理范围伤害（如果适用）
   ↓
5. 计算格挡
   if (Block) Damage = Damage / 2
   ↓
6. 计算护甲减免
   EffectiveArmor = Armor * (100 - ArmorPenetration * Coefficient) / 100
   Damage = Damage * (100 - EffectiveArmor * Coefficient) / 100
   ↓
7. 计算暴击
   EffectiveCriticalHitChance = CriticalHitChance - (CriticalHitResistance * Coefficient)
   if (CriticalHit) Damage = Damage * 2 + CriticalHitDamage
   ↓
8. 应用最终伤害
   ↓
9. 应用 Debuff（如果触发）
```

---

## ExecCalc_Damage 实现

### 类定义

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
    
private:
    void DetermineDebuff(...) const;
};
```

### Execute_Implementation

主要的伤害计算函数。

#### 执行步骤

1. **设置属性捕获定义**
2. **获取源和目标 ASC**
3. **获取玩家等级**
4. **确定 Debuff**
5. **计算伤害类型伤害**
6. **计算格挡**
7. **计算护甲减免**
8. **计算暴击**
9. **应用最终伤害**

---

## 属性捕获

### AuraDamageStatics

静态结构体，定义所有需要捕获的属性。

```cpp
struct AuraDamageStatics
{
    // 目标属性
    DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
    DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
    DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance);
    
    // 源属性
    DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
    DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
    DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitDamage);
    
    // 抗性属性
    DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance);
    DECLARE_ATTRIBUTE_CAPTUREDEF(LightningResistance);
    DECLARE_ATTRIBUTE_CAPTUREDEF(ArcaneResistance);
    DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance);
    
    AuraDamageStatics()
    {
        // 定义捕获
        DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, Armor, Target, false);
        DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, BlockChance, Target, false);
        // ... 其他属性
    }
};
```

### 属性捕获参数

- **Source**: 从源（攻击者）捕获
- **Target**: 从目标（受击者）捕获
- **Snapshot**: 是否快照（false = 实时值）

### 捕获属性值

```cpp
float TargetArmor = 0.f;
ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
    DamageStatics().ArmorDef,
    EvaluationParameters,
    TargetArmor
);
TargetArmor = FMath::Max<float>(TargetArmor, 0.f);
```

---

## 伤害类型系统

### 伤害类型映射

```cpp
// 伤害类型 → 抗性类型
TMap<FGameplayTag, FGameplayTag> DamageTypesToResistances = {
    { Damage_Fire, Attributes_Resistance_Fire },
    { Damage_Lightning, Attributes_Resistance_Lightning },
    { Damage_Arcane, Attributes_Resistance_Arcane },
    { Damage_Physical, Attributes_Resistance_Physical }
};
```

### 伤害类型计算

```cpp
// 遍历所有伤害类型
for (const TTuple<FGameplayTag, FGameplayTag>& Pair : DamageTypesToResistances)
{
    const FGameplayTag DamageTypeTag = Pair.Key;
    const FGameplayTag ResistanceTag = Pair.Value;
    
    // 获取伤害值（从 SetByCaller）
    float DamageTypeValue = Spec.GetSetByCallerMagnitude(DamageTypeTag, false);
    if (DamageTypeValue <= 0.f) continue;
    
    // 获取抗性值
    float Resistance = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
        TagsToCaptureDefs[ResistanceTag],
        EvaluationParameters,
        Resistance
    );
    Resistance = FMath::Clamp(Resistance, 0.f, 100.f);
    
    // 应用抗性减免
    DamageTypeValue *= (100.f - Resistance) / 100.f;
    
    // 累加伤害
    Damage += DamageTypeValue;
}
```

---

## 护甲计算

### 护甲穿透

```cpp
// 获取源护甲穿透
float SourceArmorPenetration = 0.f;
ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
    DamageStatics().ArmorPenetrationDef,
    EvaluationParameters,
    SourceArmorPenetration
);
SourceArmorPenetration = FMath::Max<float>(SourceArmorPenetration, 0.f);

// 获取等级系数
const UCharacterClassInfo* CharacterClassInfo = 
    UAuraAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);
const FRealCurve* ArmorPenetrationCurve = 
    CharacterClassInfo->DamageCalculationCoefficients->FindCurve(
        FName("ArmorPenetration"), 
        FString()
    );
const float ArmorPenetrationCoefficient = 
    ArmorPenetrationCurve->Eval(SourcePlayerLevel);

// 计算有效护甲
const float EffectiveArmor = TargetArmor * 
    (100 - SourceArmorPenetration * ArmorPenetrationCoefficient) / 100.f;
```

### 护甲减免

```cpp
// 获取有效护甲系数
const FRealCurve* EffectiveArmorCurve = 
    CharacterClassInfo->DamageCalculationCoefficients->FindCurve(
        FName("EffectiveArmor"), 
        FString()
    );
const float EffectiveArmorCoefficient = 
    EffectiveArmorCurve->Eval(TargetPlayerLevel);

// 应用护甲减免
Damage *= (100 - EffectiveArmor * EffectiveArmorCoefficient) / 100.f;
```

### 计算公式

```
EffectiveArmor = TargetArmor * (100 - ArmorPenetration * Coefficient) / 100
DamageReduction = EffectiveArmor * Coefficient / 100
FinalDamage = Damage * (100 - DamageReduction) / 100
```

---

## 暴击计算

### 暴击几率计算

```cpp
// 获取源暴击几率
float SourceCriticalHitChance = 0.f;
ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
    DamageStatics().CriticalHitChanceDef,
    EvaluationParameters,
    SourceCriticalHitChance
);
SourceCriticalHitChance = FMath::Max<float>(SourceCriticalHitChance, 0.f);

// 获取目标暴击抗性
float TargetCriticalHitResistance = 0.f;
ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
    DamageStatics().CriticalHitResistanceDef,
    EvaluationParameters,
    TargetCriticalHitResistance
);
TargetCriticalHitResistance = FMath::Max<float>(TargetCriticalHitResistance, 0.f);

// 获取暴击抗性系数
const FRealCurve* CriticalHitResistanceCurve = 
    CharacterClassInfo->DamageCalculationCoefficients->FindCurve(
        FName("CriticalHitResistance"), 
        FString()
    );
const float CriticalHitResistanceCoefficient = 
    CriticalHitResistanceCurve->Eval(TargetPlayerLevel);

// 计算有效暴击几率
const float EffectiveCriticalHitChance = SourceCriticalHitChance - 
    (TargetCriticalHitResistance * CriticalHitResistanceCoefficient);

// 判断是否暴击
const bool bCriticalHit = FMath::RandRange(1, 100) < EffectiveCriticalHitChance;
```

### 暴击伤害

```cpp
// 获取暴击伤害加成
float SourceCriticalHitDamage = 0.f;
ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
    DamageStatics().CriticalHitDamageDef,
    EvaluationParameters,
    SourceCriticalHitDamage
);
SourceCriticalHitDamage = FMath::Max<float>(SourceCriticalHitDamage, 0.f);

// 应用暴击伤害
if (bCriticalHit)
{
    // 双倍伤害 + 暴击伤害加成
    Damage = 2.f * Damage + SourceCriticalHitDamage;
}
```

### 计算公式

```
EffectiveCriticalHitChance = CriticalHitChance - 
    (CriticalHitResistance * Coefficient)
bCriticalHit = Random < EffectiveCriticalHitChance
if (bCriticalHit)
    Damage = Damage * 2 + CriticalHitDamage
```

---

## 抗性计算

### 抗性应用

抗性在伤害类型计算阶段应用：

```cpp
// 获取抗性值
float Resistance = 0.f;
ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
    TagsToCaptureDefs[ResistanceTag],
    EvaluationParameters,
    Resistance
);
Resistance = FMath::Clamp(Resistance, 0.f, 100.f);

// 应用抗性减免
DamageTypeValue *= (100.f - Resistance) / 100.f;
```

### 抗性范围

- **最小值**: 0（无抗性）
- **最大值**: 100（完全免疫）

### 计算公式

```
DamageAfterResistance = Damage * (100 - Resistance) / 100
```

---

## 格挡计算

### 格挡判断

```cpp
// 获取目标格挡几率
float TargetBlockChance = 0.f;
ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
    DamageStatics().BlockChanceDef,
    EvaluationParameters,
    TargetBlockChance
);
TargetBlockChance = FMath::Max<float>(TargetBlockChance, 0.f);

// 判断是否格挡
const bool bBlocked = FMath::RandRange(1, 100) < TargetBlockChance;

// 记录格挡状态
UAuraAbilitySystemLibrary::SetIsBlockedHit(EffectContextHandle, bBlocked);

// 如果格挡，伤害减半
Damage = bBlocked ? Damage / 2.f : Damage;
```

### 格挡效果

- **伤害减半**: 格挡成功时伤害减半
- **状态记录**: 记录格挡状态用于 UI 显示

---

## 范围伤害

### 范围伤害处理

```cpp
if (UAuraAbilitySystemLibrary::IsRadialDamage(EffectContextHandle))
{
    // 绑定伤害委托
    if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(TargetAvatar))
    {
        CombatInterface->GetOnDamageSignature().AddLambda(
            [&](float DamageAmount)
            {
                DamageTypeValue = DamageAmount;
            }
        );
    }
    
    // 应用范围伤害
    UGameplayStatics::ApplyRadialDamageWithFalloff(
        TargetAvatar,
        DamageTypeValue,              // 基础伤害
        0.f,                          // 最小伤害
        UAuraAbilitySystemLibrary::GetRadialDamageOrigin(EffectContextHandle),
        UAuraAbilitySystemLibrary::GetRadialDamageInnerRadius(EffectContextHandle),
        UAuraAbilitySystemLibrary::GetRadialDamageOuterRadius(EffectContextHandle),
        1.f,                          // 伤害衰减指数
        UDamageType::StaticClass(),
        TArray<AActor*>(),
        SourceAvatar,
        nullptr
    );
}
```

### 范围伤害参数

- **Origin**: 伤害中心点
- **InnerRadius**: 内半径（满伤害）
- **OuterRadius**: 外半径（最小伤害）
- **Falloff**: 伤害衰减指数

---

## 等级系数

### 系数来源

等级系数存储在 `CharacterClassInfo` 数据资产的 `DamageCalculationCoefficients` 曲线表中。

### 支持的系数

- **ArmorPenetration**: 护甲穿透系数
- **EffectiveArmor**: 有效护甲系数
- **CriticalHitResistance**: 暴击抗性系数

### 使用系数

```cpp
// 获取曲线
const FRealCurve* Curve = CharacterClassInfo->DamageCalculationCoefficients
    ->FindCurve(FName("ArmorPenetration"), FString());

// 根据等级获取系数值
const float Coefficient = Curve->Eval(PlayerLevel);
```

### 系数作用

等级系数用于平衡不同等级之间的伤害计算，确保：

- 低等级时系数较小（避免过度影响）
- 高等级时系数较大（增加策略性）

---

## 使用示例

### 创建伤害 GameplayEffect

#### 步骤 1: 创建 GameplayEffect

在编辑器中创建 GameplayEffect，设置：

- **Execution Calculation**: `ExecCalc_Damage`
- **Duration Policy**: `Instant`

#### 步骤 2: 设置伤害值

在 GameplayEffect 的 Modifiers 中，使用 SetByCaller 设置伤害值：

- **Attribute**: `IncomingDamage`
- **Magnitude Calculation Type**: `SetByCaller`
- **SetByCaller Magnitude**: `Damage.Fire`（或其他伤害类型标签）

#### 步骤 3: 设置伤害类型

在技能中设置伤害类型：

```cpp
// 在技能中
FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
    DamageEffectClass, 
    Level, 
    ContextHandle
);

// 设置伤害值
SpecHandle.Data->SetSetByCallerMagnitude(
    FAuraGameplayTags::Get().Damage_Fire, 
    100.f
);

// 应用效果
ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
```

### 自定义伤害计算

#### 添加新的伤害类型

1. **添加 GameplayTag**:
```cpp
// AuraGameplayTags.h
FGameplayTag Damage_Ice;

// AuraGameplayTags.cpp
GameplayTags.Damage_Ice = UGameplayTagsManager::Get()
    .AddNativeGameplayTag(FName("Damage.Ice"));
```

2. **添加抗性属性**:
```cpp
// AuraAttributeSet.h
UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_IceResistance)
FGameplayAttributeData IceResistance;
ATTRIBUTE_ACCESSORS(UAuraAttributeSet, IceResistance);
```

3. **添加到映射**:
```cpp
// AuraGameplayTags.cpp
GameplayTags.DamageTypesToResistances.Add(
    GameplayTags.Damage_Ice,
    GameplayTags.Attributes_Resistance_Ice
);
```

4. **在 ExecCalc_Damage 中添加捕获**:
```cpp
// ExecCalc_Damage.cpp
DECLARE_ATTRIBUTE_CAPTUREDEF(IceResistance);

DEFINE_ATTRIBUTE_CAPTUREDEF(UAuraAttributeSet, IceResistance, Target, false);
```

---

## 最佳实践

### 1. 伤害设计

- **平衡性**: 确保各种伤害因素平衡
- **可配置**: 使用数据资产配置伤害值
- **可扩展**: 易于添加新的伤害类型

### 2. 性能优化

- **属性捕获**: 只捕获必要的属性
- **缓存系数**: 缓存频繁使用的系数值
- **避免重复计算**: 避免在循环中重复计算

### 3. 调试

- **日志记录**: 记录关键计算步骤
- **可视化**: 在 UI 中显示伤害计算过程
- **测试工具**: 创建测试工具验证计算

---

## 总结

伤害计算系统提供了完整的伤害计算功能：

- ✅ **多因素计算**: 护甲、暴击、抗性、格挡
- ✅ **等级系数**: 支持等级相关的系数
- ✅ **范围伤害**: 支持范围伤害计算
- ✅ **Debuff 集成**: 与 Debuff 系统集成
- ✅ **可扩展**: 易于添加新的伤害类型和计算因素

通过这个系统，开发者可以创建复杂而平衡的战斗系统。

