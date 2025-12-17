# 如何添加新资源

本指南将详细介绍如何在 Aura 项目中添加一个新的资源（如 Energy、Stamina 等）。资源是 Vital Attributes，包括当前值、最大值和恢复值。

## 目录

1. [概述](#概述)
2. [步骤 1: 添加 GameplayTag](#步骤-1-添加-gameplaytag)
3. [步骤 2: 在 AttributeSet 中添加属性](#步骤-2-在-attributeset-中添加属性)
4. [步骤 3: 实现网络复制](#步骤-3-实现网络复制)
5. [步骤 4: 实现属性限制](#步骤-4-实现属性限制)
6. [步骤 5: 添加到 TagsToAttributes 映射](#步骤-5-添加到-tagstoattributes-映射)
7. [步骤 6: 创建 ModMagCalc（可选）](#步骤-6-创建-modmagcalc可选)
8. [步骤 7: 创建初始化 GameplayEffect](#步骤-7-创建初始化-gameplayeffect)
9. [步骤 8: 在 UI 中显示资源](#步骤-8-在-ui-中显示资源)
10. [步骤 9: 在技能中使用资源](#步骤-9-在技能中使用资源)
11. [完整示例](#完整示例)
12. [常见问题](#常见问题)

---

## 概述

在 Aura 项目中，资源（Resource）是 Vital Attributes，包括：

- **当前值** (Current Value): 如 `Health`、`Mana`
- **最大值** (Max Value): 如 `MaxHealth`、`MaxMana`
- **恢复值** (Regeneration): 如 `HealthRegeneration`、`ManaRegeneration`

### 资源类型示例

- **Health** (生命值): 角色的生命值
- **Mana** (法力值): 施放技能消耗的资源
- **Energy** (能量值): 可用于特殊技能
- **Stamina** (耐力值): 用于冲刺、闪避等动作

### 添加资源需要完成的步骤

1. ✅ 添加 GameplayTag
2. ✅ 在 AttributeSet 中添加属性
3. ✅ 实现网络复制
4. ✅ 实现属性限制（PreAttributeChange）
5. ✅ 添加到 TagsToAttributes 映射
6. ✅ 创建 ModMagCalc（如果需要计算最大值）
7. ✅ 创建初始化 GameplayEffect
8. ✅ 在 UI 中显示资源
9. ✅ 在技能中使用资源（可选）

---

## 步骤 1: 添加 GameplayTag

### 1.1 在头文件中添加 Tag

在 `Source/Aura/Public/AuraGameplayTags.h` 中添加：

```cpp
struct FAuraGameplayTags
{
    // ... 现有 Tags
    
    // Vital Attributes - Energy
    FGameplayTag Attributes_Vital_Energy;
    FGameplayTag Attributes_Secondary_MaxEnergy;
    FGameplayTag Attributes_Secondary_EnergyRegeneration;
};
```

### 1.2 在实现文件中注册 Tag

在 `Source/Aura/Private/AuraGameplayTags.cpp` 的 `InitializeNativeGameplayTags()` 函数中添加：

```cpp
void FAuraGameplayTags::InitializeNativeGameplayTags()
{
    // ... 现有注册代码
    
    /*
     * Vital Attributes - Energy
     */
    GameplayTags.Attributes_Vital_Energy = UGameplayTagsManager::Get().AddNativeGameplayTag(
        FName("Attributes.Vital.Energy"),
        FString("Current Energy value")
    );
    
    GameplayTags.Attributes_Secondary_MaxEnergy = UGameplayTagsManager::Get().AddNativeGameplayTag(
        FName("Attributes.Secondary.MaxEnergy"),
        FString("Maximum amount of Energy obtainable")
    );
    
    GameplayTags.Attributes_Secondary_EnergyRegeneration = UGameplayTagsManager::Get().AddNativeGameplayTag(
        FName("Attributes.Secondary.EnergyRegeneration"),
        FString("Amount of Energy regenerated every 1 second")
    );
}
```

### 1.3 在配置文件中添加 Tag（可选）

在 `Config/DefaultGameplayTags.ini` 中添加：

```ini
+GameplayTagList=(Tag="Attributes.Vital.Energy",DevComment="Current Energy value")
+GameplayTagList=(Tag="Attributes.Secondary.MaxEnergy",DevComment="Maximum Energy")
+GameplayTagList=(Tag="Attributes.Secondary.EnergyRegeneration",DevComment="Energy regeneration per second")
```

---

## 步骤 2: 在 AttributeSet 中添加属性

### 2.1 在头文件中添加属性

在 `Source/Aura/Public/AbilitySystem/AuraAttributeSet.h` 中添加：

```cpp
class UAuraAttributeSet : public UAttributeSet
{
    // ... 现有代码
    
    /*
     * Vital Attributes
     */
    
    // 现有: Health, Mana
    
    // 新增: Energy
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Energy, Category = "Vital Attributes")
    FGameplayAttributeData Energy;
    ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Energy);
    
    /*
     * Secondary Attributes (Max Values)
     */
    
    // 现有: MaxHealth, MaxMana
    
    // 新增: MaxEnergy
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxEnergy, Category = "Vital Attributes")
    FGameplayAttributeData MaxEnergy;
    ATTRIBUTE_ACCESSORS(UAuraAttributeSet, MaxEnergy);
    
    /*
     * Secondary Attributes (Regeneration)
     */
    
    // 现有: HealthRegeneration, ManaRegeneration
    
    // 新增: EnergyRegeneration
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_EnergyRegeneration, Category = "Secondary Attributes")
    FGameplayAttributeData EnergyRegeneration;
    ATTRIBUTE_ACCESSORS(UAuraAttributeSet, EnergyRegeneration);
    
    // ... 现有 OnRep 函数声明
    
    // 新增 OnRep 函数
    UFUNCTION()
    void OnRep_Energy(const FGameplayAttributeData& OldEnergy) const;
    
    UFUNCTION()
    void OnRep_MaxEnergy(const FGameplayAttributeData& OldMaxEnergy) const;
    
    UFUNCTION()
    void OnRep_EnergyRegeneration(const FGameplayAttributeData& OldEnergyRegeneration) const;
};
```

### 2.2 在实现文件中实现 OnRep 函数

在 `Source/Aura/Private/AbilitySystem/AuraAttributeSet.cpp` 中添加：

```cpp
void UAuraAttributeSet::OnRep_Energy(const FGameplayAttributeData& OldEnergy) const
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Energy, OldEnergy);
}

void UAuraAttributeSet::OnRep_MaxEnergy(const FGameplayAttributeData& OldMaxEnergy) const
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, MaxEnergy, OldMaxEnergy);
}

void UAuraAttributeSet::OnRep_EnergyRegeneration(const FGameplayAttributeData& OldEnergyRegeneration) const
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, EnergyRegeneration, OldEnergyRegeneration);
}
```

---

## 步骤 3: 实现网络复制

在 `Source/Aura/Private/AbilitySystem/AuraAttributeSet.cpp` 的 `GetLifetimeReplicatedProps()` 函数中添加：

```cpp
void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // ... 现有复制代码
    
    // Vital Attributes
    DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Energy, COND_None, REPNOTIFY_Always);  // 新增
    
    // Secondary Attributes (Max Values)
    DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, MaxEnergy, COND_None, REPNOTIFY_Always);  // 新增
    
    // Secondary Attributes (Regeneration)
    DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, HealthRegeneration, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, ManaRegeneration, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, EnergyRegeneration, COND_None, REPNOTIFY_Always);  // 新增
}
```

---

## 步骤 4: 实现属性限制

在 `Source/Aura/Private/AbilitySystem/AuraAttributeSet.cpp` 的 `PreAttributeChange()` 函数中添加：

```cpp
void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    Super::PreAttributeChange(Attribute, NewValue);
    
    // 现有限制
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
    }
    if (Attribute == GetManaAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
    }
    
    // 新增: Energy 限制
    if (Attribute == GetEnergyAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.f, GetMaxEnergy());
    }
}
```

在 `PostGameplayEffectExecute()` 函数中添加：

```cpp
void UAuraAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);
    
    FEffectProperties Props;
    SetEffectProperties(Data, Props);
    
    // ... 现有处理代码
    
    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        SetHealth(FMath::Clamp(GetHealth(), 0.f, GetMaxHealth()));
    }
    if (Data.EvaluatedData.Attribute == GetManaAttribute())
    {
        SetMana(FMath::Clamp(GetMana(), 0.f, GetMaxMana()));
    }
    
    // 新增: Energy 处理
    if (Data.EvaluatedData.Attribute == GetEnergyAttribute())
    {
        SetEnergy(FMath::Clamp(GetEnergy(), 0.f, GetMaxEnergy()));
    }
}
```

---

## 步骤 5: 添加到 TagsToAttributes 映射

在 `Source/Aura/Private/AbilitySystem/AuraAttributeSet.cpp` 的构造函数中添加：

```cpp
UAuraAttributeSet::UAuraAttributeSet()
{
    const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
    
    // ... 现有映射
    
    // Vital Attributes
    TagsToAttributes.Add(GameplayTags.Attributes_Vital_Health, GetHealthAttribute);
    TagsToAttributes.Add(GameplayTags.Attributes_Vital_Mana, GetManaAttribute);
    TagsToAttributes.Add(GameplayTags.Attributes_Vital_Energy, GetEnergyAttribute);  // 新增
    
    // Secondary Attributes (Max Values)
    TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxHealth, GetMaxHealthAttribute);
    TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxMana, GetMaxManaAttribute);
    TagsToAttributes.Add(GameplayTags.Attributes_Secondary_MaxEnergy, GetMaxEnergyAttribute);  // 新增
    
    // Secondary Attributes (Regeneration)
    TagsToAttributes.Add(GameplayTags.Attributes_Secondary_HealthRegeneration, GetHealthRegenerationAttribute);
    TagsToAttributes.Add(GameplayTags.Attributes_Secondary_ManaRegeneration, GetManaRegenerationAttribute);
    TagsToAttributes.Add(GameplayTags.Attributes_Secondary_EnergyRegeneration, GetEnergyRegenerationAttribute);  // 新增
}
```

---

## 步骤 6: 创建 ModMagCalc（可选）

如果需要根据主属性计算最大资源值，可以创建 ModMagCalc。

### 6.1 创建 MMC_MaxEnergy 类

在 `Source/Aura/Public/AbilitySystem/ModMagCalc/MMC_MaxEnergy.h` 中：

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_MaxEnergy.generated.h"

/**
 * 计算最大能量值
 * 公式: BaseValue + (Intelligence * Coefficient)
 */
UCLASS()
class AURA_API UMMC_MaxEnergy : public UGameplayModMagnitudeCalculation
{
    GENERATED_BODY()
    
public:
    UMMC_MaxEnergy();
    
    virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
    
private:
    FGameplayEffectAttributeCaptureDefinition IntelligenceDef;
    FGameplayEffectAttributeCaptureDefinition VigorDef;  // 可选：也可以使用 Vigor
};
```

在 `Source/Aura/Private/AbilitySystem/ModMagCalc/MMC_MaxEnergy.cpp` 中：

```cpp
#include "AbilitySystem/ModMagCalc/MMC_MaxEnergy.h"
#include "AbilitySystem/AuraAttributeSet.h"
#include "AuraGameplayTags.h"

UMMC_MaxEnergy::UMMC_MaxEnergy()
{
    // 捕获 Intelligence
    IntelligenceDef.AttributeToCapture = UAuraAttributeSet::GetIntelligenceAttribute();
    IntelligenceDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
    IntelligenceDef.bSnapshot = false;
    
    // 捕获 Vigor（可选）
    VigorDef.AttributeToCapture = UAuraAttributeSet::GetVigorAttribute();
    VigorDef.AttributeSource = EGameplayEffectAttributeCaptureSource::Target;
    VigorDef.bSnapshot = false;
    
    // 添加到捕获列表
    RelevantAttributesToCapture.Add(IntelligenceDef);
    RelevantAttributesToCapture.Add(VigorDef);
}

float UMMC_MaxEnergy::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    // 获取捕获的属性值
    const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
    
    FAggregatorEvaluateParameters EvaluationParameters;
    EvaluationParameters.SourceTags = SourceTags;
    EvaluationParameters.TargetTags = TargetTags;
    
    float Intelligence = 0.f;
    GetCapturedAttributeMagnitude(IntelligenceDef, Spec, EvaluationParameters, Intelligence);
    
    float Vigor = 0.f;
    GetCapturedAttributeMagnitude(VigorDef, Spec, EvaluationParameters, Vigor);
    
    // 计算最大值
    // 公式: BaseValue + (Intelligence * 2.5f) + (Vigor * 1.0f)
    float BaseValue = 50.f;  // 基础值
    float IntelligenceCoefficient = 2.5f;
    float VigorCoefficient = 1.0f;
    
    return BaseValue + (Intelligence * IntelligenceCoefficient) + (Vigor * VigorCoefficient);
}
```

### 6.2 在 GameplayEffect 中使用

在创建 `MaxEnergy` 的 GameplayEffect 时：
- 设置 `Modifier Magnitude Calculation Type` 为 `Attribute Based`
- 选择 `MMC_MaxEnergy` 类

---

## 步骤 7: 创建初始化 GameplayEffect

### 7.1 创建 MaxEnergy GameplayEffect

1. 在内容浏览器中，右键 → `Gameplay` → `Gameplay Effect`
2. 命名为 `GE_DefaultMaxEnergy`
3. 配置：
   - **Duration Policy**: `Infinite`
   - **Modifiers**:
     - Attribute: `Attributes.Secondary.MaxEnergy`
     - Modifier Op: `Override`
     - Magnitude Calculation Type: `Scalable Float` 或 `Attribute Based`（如果使用 MMC）
     - Magnitude: `100.0`（或使用 MMC）

### 7.2 创建 EnergyRegeneration GameplayEffect

1. 创建 `GE_DefaultEnergyRegeneration`
2. 配置：
   - **Duration Policy**: `Infinite`
   - **Modifiers**:
     - Attribute: `Attributes.Secondary.EnergyRegeneration`
     - Modifier Op: `Override`
     - Magnitude: `5.0`（每秒恢复 5 点）

### 7.3 创建初始 Energy GameplayEffect

1. 创建 `GE_DefaultEnergy`
2. 配置：
   - **Duration Policy**: `Instant`
   - **Modifiers**:
     - Attribute: `Attributes.Vital.Energy`
     - Modifier Op: `Override`
     - Magnitude: `100.0`（初始值，通常等于 MaxEnergy）

### 7.4 在角色初始化中应用

在 `AuraCharacterBase` 的 `InitializeDefaultAttributes()` 中：

```cpp
void AAuraCharacterBase::InitializeDefaultAttributes() const
{
    ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);
    ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
    ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
    ApplyEffectToSelf(DefaultMaxEnergy, 1.f);  // 新增
    ApplyEffectToSelf(DefaultEnergyRegeneration, 1.f);  // 新增
    ApplyEffectToSelf(DefaultEnergy, 1.f);  // 新增
}
```

在角色蓝图中：
1. 找到 `Default Max Energy` 属性
2. 设置为 `GE_DefaultMaxEnergy`
3. 找到 `Default Energy Regeneration` 属性
4. 设置为 `GE_DefaultEnergyRegeneration`
5. 找到 `Default Energy` 属性
6. 设置为 `GE_DefaultEnergy`

---

## 步骤 8: 在 UI 中显示资源

### 8.1 在 OverlayWidgetController 中添加委托

在 `Source/Aura/Public/UI/WidgetController/OverlayWidgetController.h` 中添加：

```cpp
class UOverlayWidgetController : public UAuraWidgetController
{
    // ... 现有委托
    
    UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
    FOnAttributeChangedSignature OnEnergyChanged;
    
    UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
    FOnAttributeChangedSignature OnMaxEnergyChanged;
};
```

### 8.2 在 OverlayWidgetController 中实现

在 `Source/Aura/Private/UI/WidgetController/OverlayWidgetController.cpp` 中：

```cpp
void UOverlayWidgetController::BroadcastInitialValues()
{
    // ... 现有广播
    
    OnEnergyChanged.Broadcast(GetAuraAS()->GetEnergy());
    OnMaxEnergyChanged.Broadcast(GetAuraAS()->GetMaxEnergy());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
    // ... 现有绑定
    
    // Energy
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
        GetAuraAS()->GetEnergyAttribute()
    ).AddLambda(
        [this](const FOnAttributeChangeData& Data)
        {
            OnEnergyChanged.Broadcast(Data.NewValue);
        }
    );
    
    // MaxEnergy
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
        GetAuraAS()->GetMaxEnergyAttribute()
    ).AddLambda(
        [this](const FOnAttributeChangeData& Data)
        {
            OnMaxEnergyChanged.Broadcast(Data.NewValue);
        }
    );
}
```

### 8.3 在 UI Widget 中绑定

在 UI Widget 蓝图中：

1. 在 `Construct` 或 `NativeConstruct` 中：
   - 获取 `OverlayWidgetController`
   - 绑定 `OnEnergyChanged` 委托
   - 绑定 `OnMaxEnergyChanged` 委托

2. 创建更新函数：
   ```
   Event OnEnergyChanged (float NewValue)
   ├── Update Energy Text
   └── Update Energy Bar (NewValue / MaxEnergy)
   
   Event OnMaxEnergyChanged (float NewValue)
   ├── Update Max Energy Text
   └── Update Energy Bar (Energy / NewValue)
   ```

---

## 步骤 9: 在技能中使用资源

### 9.1 创建资源消耗 GameplayEffect

1. 创建 `GE_EnergyCost` GameplayEffect
2. 配置：
   - **Duration Policy**: `Instant`
   - **Modifiers**:
     - Attribute: `Attributes.Vital.Energy`
     - Modifier Op: `Subtract`
     - Magnitude Calculation Type: `SetByCaller`
     - SetByCaller Magnitude Data Name: `Data.EnergyCost`

### 9.2 在技能中设置消耗

在技能蓝图中：

```cpp
// 在 ActivateAbility 中
void UMyAbility::ActivateAbility(...)
{
    // ... 其他逻辑
    
    // 应用能量消耗
    FGameplayEffectSpecHandle CostSpecHandle = MakeOutgoingGameplayEffectSpec(CostGameplayEffectClass);
    CostSpecHandle.Data->SetSetByCallerMagnitude(
        FGameplayTag::RequestGameplayTag("Data.EnergyCost"),
        EnergyCost
    );
    ApplyGameplayEffectSpecToOwner(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, CostSpecHandle);
}
```

### 9.3 检查资源是否足够

```cpp
bool UMyAbility::CheckCost(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, OUT FGameplayTagContainer* OptionalRelevantTags) const
{
    if (!Super::CheckCost(Handle, ActorInfo, OptionalRelevantTags))
    {
        return false;
    }
    
    // 检查能量是否足够
    if (UAuraAttributeSet* AS = Cast<UAuraAttributeSet>(ActorInfo->AbilitySystemComponent->GetAttributeSet(UAuraAttributeSet::StaticClass())))
    {
        if (AS->GetEnergy() < EnergyCost)
        {
            return false;
        }
    }
    
    return true;
}
```

---

## 完整示例

### 示例: 添加 Energy 资源

#### 1. GameplayTags

```cpp
// AuraGameplayTags.h
FGameplayTag Attributes_Vital_Energy;
FGameplayTag Attributes_Secondary_MaxEnergy;
FGameplayTag Attributes_Secondary_EnergyRegeneration;

// AuraGameplayTags.cpp
GameplayTags.Attributes_Vital_Energy = UGameplayTagsManager::Get().AddNativeGameplayTag(
    FName("Attributes.Vital.Energy"),
    FString("Current Energy value")
);
```

#### 2. AttributeSet

```cpp
// AuraAttributeSet.h
UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Energy, Category = "Vital Attributes")
FGameplayAttributeData Energy;
ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Energy);

// AuraAttributeSet.cpp
DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Energy, COND_None, REPNOTIFY_Always);
```

#### 3. UI 显示

```cpp
// OverlayWidgetController.h
UPROPERTY(BlueprintAssignable, Category="GAS|Attributes")
FOnAttributeChangedSignature OnEnergyChanged;

// OverlayWidgetController.cpp
OnEnergyChanged.Broadcast(GetAuraAS()->GetEnergy());
```

---

## 常见问题

### 问题 1: 资源值未正确限制

**原因**: `PreAttributeChange` 或 `PostGameplayEffectExecute` 未正确实现

**解决方案**: 确保在两个函数中都添加了限制逻辑

### 问题 2: UI 未更新

**原因**: 委托未正确绑定或广播

**解决方案**: 
1. 检查 `BroadcastInitialValues()` 是否调用
2. 检查 `BindCallbacksToDependencies()` 是否调用
3. 检查 Widget 是否绑定到委托

### 问题 3: 资源未初始化

**原因**: GameplayEffect 未正确应用

**解决方案**: 
1. 检查 `InitializeDefaultAttributes()` 是否调用
2. 检查 GameplayEffect 配置是否正确
3. 检查角色蓝图中的默认值设置

### 问题 4: 网络同步问题

**原因**: 网络复制未正确配置

**解决方案**: 
1. 检查 `GetLifetimeReplicatedProps()` 是否添加
2. 检查 `OnRep` 函数是否正确实现
3. 确保在服务器上修改属性

---

## 总结

添加新资源需要完成以下步骤：

1. ✅ 添加 GameplayTag
2. ✅ 在 AttributeSet 中添加属性
3. ✅ 实现网络复制
4. ✅ 实现属性限制
5. ✅ 添加到 TagsToAttributes 映射
6. ✅ 创建 ModMagCalc（可选）
7. ✅ 创建初始化 GameplayEffect
8. ✅ 在 UI 中显示资源
9. ✅ 在技能中使用资源（可选）

通过遵循这些步骤，可以成功添加新的资源类型。

---

## 相关文档

- [属性系统文档](../Systems/Attribute_System.md) - 属性系统详细文档
- [GameplayEffect 系统文档](../Systems/GameplayEffect_System.md) - GameplayEffect 详细文档
- [UI 系统文档](../Systems/UI_System.md) - UI 系统详细文档
- [如何添加新技能指南](./How_To_Add_New_Ability.md) - 添加新技能的完整指南

