# 属性系统详细文档

## 目录

1. [系统概述](#系统概述)
2. [属性分类](#属性分类)
3. [属性访问器](#属性访问器)
4. [属性初始化](#属性初始化)
5. [属性计算](#属性计算)
6. [属性变化回调](#属性变化回调)
7. [网络复制](#网络复制)
8. [Gameplay Tags 映射](#gameplay-tags-映射)
9. [属性升级](#属性升级)
10. [使用示例](#使用示例)

---

## 系统概述

属性系统是 Aura 项目的核心，管理所有游戏角色的数值属性。系统基于 Unreal Engine 的 Gameplay Attribute System，提供了完整的属性管理、计算和同步功能。

### 核心组件

- **UAuraAttributeSet**: 属性集类，管理所有属性
- **ATTRIBUTE_ACCESSORS 宏**: 自动生成属性访问器
- **TagsToAttributes 映射**: GameplayTag 到属性的映射

### 系统特点

- ✅ 完整的属性分类体系
- ✅ 自动属性访问器生成
- ✅ 属性变化回调系统
- ✅ 网络复制支持
- ✅ GameplayTag 映射
- ✅ 属性计算系统（MMC）

---

## 属性分类

### 主属性 (Primary Attributes)

主属性是角色的基础属性，影响次属性的计算。

#### Strength (力量)

- **GameplayTag**: `Attributes.Primary.Strength`
- **描述**: "Increases physical damage"
- **影响**:
  - 物理伤害
  - 最大生命值（通过 MMC）

#### Intelligence (智力)

- **GameplayTag**: `Attributes.Primary.Intelligence`
- **描述**: "Increases magical damage"
- **影响**:
  - 法术伤害
  - 最大法力值（通过 MMC）

#### Resilience (韧性)

- **GameplayTag**: `Attributes.Primary.Resilience`
- **描述**: "Increases Armor and Armor Penetration"
- **影响**:
  - 护甲值
  - 护甲穿透

#### Vigor (活力)

- **GameplayTag**: `Attributes.Primary.Vigor`
- **描述**: "Increases Health"
- **影响**:
  - 最大生命值（通过 MMC）
  - 生命恢复

### 次属性 (Secondary Attributes)

次属性由主属性计算得出，直接影响战斗表现。

#### Armor (护甲)

- **GameplayTag**: `Attributes.Secondary.Armor`
- **描述**: "Reduces damage taken, improves Block Chance"
- **作用**: 减少受到的伤害
- **计算**: 由 Resilience 等主属性计算

#### ArmorPenetration (护甲穿透)

- **GameplayTag**: `Attributes.Secondary.ArmorPenetration`
- **描述**: "Ignores Percentage of enemy Armor, increases Critical Hit Chance"
- **作用**: 忽略目标一定百分比的护甲

#### BlockChance (格挡几率)

- **GameplayTag**: `Attributes.Secondary.BlockChance`
- **描述**: "Chance to cut incoming damage in half"
- **作用**: 有几率将受到的伤害减半

#### CriticalHitChance (暴击几率)

- **GameplayTag**: `Attributes.Secondary.CriticalHitChance`
- **描述**: "Chance to double damage plus critical hit bonus"
- **作用**: 有几率造成暴击

#### CriticalHitDamage (暴击伤害)

- **GameplayTag**: `Attributes.Secondary.CriticalHitDamage`
- **描述**: 暴击时的额外伤害加成
- **作用**: 增加暴击伤害

#### CriticalHitResistance (暴击抗性)

- **GameplayTag**: `Attributes.Secondary.CriticalHitResistance`
- **描述**: 减少被暴击的几率
- **作用**: 降低敌人暴击几率

#### HealthRegeneration (生命恢复)

- **GameplayTag**: `Attributes.Secondary.HealthRegeneration`
- **描述**: 每秒恢复的生命值
- **作用**: 持续恢复生命值

#### ManaRegeneration (法力恢复)

- **GameplayTag**: `Attributes.Secondary.ManaRegeneration`
- **描述**: 每秒恢复的法力值
- **作用**: 持续恢复法力值

#### MaxHealth (最大生命值)

- **GameplayTag**: `Attributes.Secondary.MaxHealth`
- **描述**: 最大生命值
- **计算**: 通过 `MMC_MaxHealth` 计算
- **公式**: `BaseValue + (Vigor * Coefficient) + (Strength * Coefficient)`

#### MaxMana (最大法力值)

- **GameplayTag**: `Attributes.Secondary.MaxMana`
- **描述**: 最大法力值
- **计算**: 通过 `MMC_MaxMana` 计算
- **公式**: `BaseValue + (Intelligence * Coefficient)`

### 抗性属性 (Resistance Attributes)

抗性属性减少对应类型伤害的百分比。

#### FireResistance (火焰抗性)

- **GameplayTag**: `Attributes.Resistance.Fire`
- **作用**: 减少火焰伤害
- **范围**: 0-100（百分比）

#### LightningResistance (闪电抗性)

- **GameplayTag**: `Attributes.Resistance.Lightning`
- **作用**: 减少闪电伤害

#### ArcaneResistance (奥术抗性)

- **GameplayTag**: `Attributes.Resistance.Arcane`
- **作用**: 减少奥术伤害

#### PhysicalResistance (物理抗性)

- **GameplayTag**: `Attributes.Resistance.Physical`
- **作用**: 减少物理伤害

### 生命值属性 (Vital Attributes)

生命值属性是角色的当前状态值。

#### Health (生命值)

- **GameplayTag**: `Attributes.Vital.Health`
- **范围**: 0 到 MaxHealth
- **限制**: 通过 `PreAttributeChange` 限制

#### Mana (法力值)

- **GameplayTag**: `Attributes.Vital.Mana`
- **范围**: 0 到 MaxMana
- **限制**: 通过 `PreAttributeChange` 限制

### 元属性 (Meta Attributes)

元属性用于临时计算，不直接显示。

#### IncomingDamage (受到的伤害)

- **GameplayTag**: `Attributes.Meta.IncomingDamage`
- **用途**: 在伤害计算中存储临时伤害值
- **处理**: 在 `PostGameplayEffectExecute` 中处理

#### IncomingXP (获得的经验)

- **GameplayTag**: `Attributes.Meta.IncomingXP`
- **用途**: 存储获得的经验值
- **处理**: 在 `PostGameplayEffectExecute` 中处理

---

## 属性访问器

### ATTRIBUTE_ACCESSORS 宏

每个属性都使用 `ATTRIBUTE_ACCESSORS` 宏自动生成访问器：

```cpp
UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "Primary Attributes")
FGameplayAttributeData Strength;
ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Strength);
```

### 生成的访问器

宏会生成以下方法：

#### Getter

```cpp
// 获取当前值
float GetStrength() const;

// 获取属性
FGameplayAttribute GetStrengthAttribute() const;
```

#### Setter

```cpp
// 设置值
void SetStrength(float NewStrength);
```

#### Initializer

```cpp
// 初始化值
void InitStrength(float InitialStrength);
```

### 使用示例

```cpp
// 获取属性值
float CurrentStrength = AttributeSet->GetStrength();

// 获取属性对象
FGameplayAttribute StrengthAttr = AttributeSet->GetStrengthAttribute();

// 设置属性值
AttributeSet->SetStrength(100.f);

// 初始化属性值
AttributeSet->InitStrength(50.f);
```

---

## 属性初始化

### 初始化流程

属性初始化通过 GameplayEffect 完成：

```
1. 应用主属性 GameplayEffect
   ↓
2. 应用次属性 GameplayEffect（使用 MMC 计算）
   ↓
3. 应用生命值 GameplayEffect
   ↓
4. 属性值设置完成
```

### 初始化方法

```cpp
void AAuraCharacterBase::InitializeDefaultAttributes() const
{
    // 应用主属性
    ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);
    
    // 应用次属性
    ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
    
    // 应用生命值
    ApplyEffectToSelf(DefaultVitalAttributes, 1.f);
}
```

### 职业差异

不同职业有不同的初始属性值：

- **Warrior**: 高 Strength, Resilience
- **Ranger**: 平衡的属性分布
- **Elementalist**: 高 Intelligence

通过 `CharacterClassInfo` 数据资产配置。

---

## 属性计算

### Modifier Magnitude Calculation (MMC)

次属性通过 MMC 计算得出。

#### MMC_MaxHealth

```cpp
// 计算最大生命值
float MaxHealth = BaseValue 
    + (Vigor * VigorCoefficient) 
    + (Strength * StrengthCoefficient);
```

**系数来源**: `CharacterClassInfo` 数据资产中的曲线表

#### MMC_MaxMana

```cpp
// 计算最大法力值
float MaxMana = BaseValue 
    + (Intelligence * IntelligenceCoefficient);
```

### 属性计算公式

#### 护甲计算

```cpp
// 在伤害计算中
EffectiveArmor = Armor * (100 - ArmorPenetration * Coefficient) / 100;
DamageReduction = EffectiveArmor * Coefficient / 100;
FinalDamage = Damage * (100 - DamageReduction) / 100;
```

#### 暴击计算

```cpp
EffectiveCriticalHitChance = CriticalHitChance 
    - (CriticalHitResistance * Coefficient);
bCriticalHit = Random < EffectiveCriticalHitChance;
if (bCriticalHit)
    Damage = Damage * 2 + CriticalHitDamage;
```

---

## 属性变化回调

### PreAttributeChange

在属性值改变前调用，用于限制值范围。

```cpp
void UAuraAttributeSet::PreAttributeChange(
    const FGameplayAttribute& Attribute, 
    float& NewValue
)
{
    Super::PreAttributeChange(Attribute, NewValue);
    
    // 限制生命值范围
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
    }
    
    // 限制法力值范围
    if (Attribute == GetManaAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.f, GetMaxMana());
    }
}
```

### PostAttributeChange

在属性值改变后调用，用于响应变化。

```cpp
void UAuraAttributeSet::PostAttributeChange(
    const FGameplayAttribute& Attribute, 
    float OldValue, 
    float NewValue
)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);
    
    // 可以在这里处理属性变化逻辑
    // 例如：更新 UI、触发事件等
}
```

### PostGameplayEffectExecute

在 GameplayEffect 执行后调用，用于处理效果结果。

```cpp
void UAuraAttributeSet::PostGameplayEffectExecute(
    const FGameplayEffectModCallbackData& Data
)
{
    Super::PostGameplayEffectExecute(Data);
    
    FEffectProperties Props;
    SetEffectProperties(Data, Props);
    
    // 处理受到的伤害
    if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
    {
        HandleIncomingDamage(Props);
    }
    
    // 处理获得的经验
    if (Data.EvaluatedData.Attribute == GetIncomingXPAttribute())
    {
        HandleIncomingXP(Props);
    }
}
```

---

## 网络复制

### 复制配置

所有属性都配置为网络复制：

```cpp
void UAuraAttributeSet::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // 主属性
    DOREPLIFETIME_CONDITION_NOTIFY(
        UAuraAttributeSet, 
        Strength, 
        COND_None, 
        REPNOTIFY_Always
    );
    
    // ... 其他属性
}
```

### 复制参数

- **COND_None**: 无条件复制
- **REPNOTIFY_Always**: 总是通知变化

### OnRep 函数

每个属性都有对应的 OnRep 函数：

```cpp
UFUNCTION()
void OnRep_Strength(const FGameplayAttributeData& OldStrength) const
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Strength, OldStrength);
}
```

**GAMEPLAYATTRIBUTE_REPNOTIFY 宏**: 自动处理属性复制通知

---

## Gameplay Tags 映射

### TagsToAttributes 映射

属性集维护一个 GameplayTag 到属性的映射：

```cpp
TMap<FGameplayTag, TStaticFuncPtr<FGameplayAttribute()>> TagsToAttributes;
```

### 初始化映射

```cpp
UAuraAttributeSet::UAuraAttributeSet()
{
    const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
    
    // 主属性映射
    TagsToAttributes.Add(
        GameplayTags.Attributes_Primary_Strength, 
        GetStrengthAttribute
    );
    
    // ... 其他属性映射
}
```

### 使用映射

```cpp
// 通过 Tag 获取属性
FGameplayAttribute* AttributePtr = TagsToAttributes.Find(Tag);
if (AttributePtr)
{
    FGameplayAttribute Attribute = (*AttributePtr)();
    float Value = GetNumericAttributeValue(Attribute);
}
```

---

## 属性升级

### 升级机制

属性通过 GameplayEvent 升级：

```cpp
void UAuraAbilitySystemComponent::UpgradeAttribute(
    const FGameplayTag& AttributeTag
)
{
    if (GetAvatarActor()->Implements<UPlayerInterface>())
    {
        if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
        {
            ServerUpgradeAttribute(AttributeTag);
        }
    }
}
```

### 服务器端升级

```cpp
void UAuraAbilitySystemComponent::ServerUpgradeAttribute_Implementation(
    const FGameplayTag& AttributeTag
)
{
    // 发送 GameplayEvent
    FGameplayEventData Payload;
    Payload.EventTag = AttributeTag;
    Payload.EventMagnitude = 1.f;
    
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
        GetAvatarActor(), 
        AttributeTag, 
        Payload
    );
    
    // 消耗属性点
    if (GetAvatarActor()->Implements<UPlayerInterface>())
    {
        IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
    }
}
```

### GameplayEvent 处理

属性升级通过监听 GameplayEvent 实现：

1. 发送 GameplayEvent（Tag = 属性 Tag）
2. GameplayEffect 监听该 Event
3. 应用属性修改
4. 属性值增加

---

## 使用示例

### 获取属性值

```cpp
// 方式 1: 直接获取
float Health = AttributeSet->GetHealth();
float MaxHealth = AttributeSet->GetMaxHealth();

// 方式 2: 通过属性对象获取
FGameplayAttribute HealthAttr = AttributeSet->GetHealthAttribute();
float HealthValue = AttributeSet->GetNumericAttributeValue(HealthAttr);

// 方式 3: 通过 Tag 获取
FGameplayTag HealthTag = FAuraGameplayTags::Get().Attributes_Vital_Health;
FGameplayAttribute* AttrPtr = AttributeSet->TagsToAttributes.Find(HealthTag);
if (AttrPtr)
{
    FGameplayAttribute Attr = (*AttrPtr)();
    float Value = AttributeSet->GetNumericAttributeValue(Attr);
}
```

### 设置属性值

```cpp
// 直接设置
AttributeSet->SetHealth(100.f);

// 通过 GameplayEffect 设置（推荐）
FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(HealEffect, 1.f, ContextHandle);
SpecHandle.Data->SetSetByCallerMagnitude(HealTag, 50.f);
ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
```

### 监听属性变化

```cpp
// 在 WidgetController 中
AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
    AttributeSet->GetHealthAttribute()
).AddLambda(
    [this](const FOnAttributeChangeData& Data)
    {
        OnHealthChanged.Broadcast(Data.NewValue);
    }
);
```

### 应用属性修改

```cpp
// 创建 GameplayEffect Spec
FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(
    AttributeEffect, 
    Level, 
    ContextHandle
);

// 设置修改器值
SpecHandle.Data->SetSetByCallerMagnitude(AttributeTag, NewValue);

// 应用效果
ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), ASC);
```

---

## 配置指南

### 添加新属性

#### 步骤 1: 在 AttributeSet 中添加属性

```cpp
// AuraAttributeSet.h
UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_NewAttribute, Category = "New Attributes")
FGameplayAttributeData NewAttribute;
ATTRIBUTE_ACCESSORS(UAuraAttributeSet, NewAttribute);

UFUNCTION()
void OnRep_NewAttribute(const FGameplayAttributeData& OldNewAttribute) const;
```

#### 步骤 2: 实现复制

```cpp
// AuraAttributeSet.cpp
void UAuraAttributeSet::GetLifetimeReplicatedProps(...)
{
    DOREPLIFETIME_CONDITION_NOTIFY(
        UAuraAttributeSet, 
        NewAttribute, 
        COND_None, 
        REPNOTIFY_Always
    );
}

void UAuraAttributeSet::OnRep_NewAttribute(...)
{
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, NewAttribute, OldNewAttribute);
}
```

#### 步骤 3: 添加 GameplayTag

```cpp
// AuraGameplayTags.h
FGameplayTag Attributes_New_NewAttribute;

// AuraGameplayTags.cpp
GameplayTags.Attributes_New_NewAttribute = UGameplayTagsManager::Get()
    .AddNativeGameplayTag(FName("Attributes.New.NewAttribute"), FString("Description"));
```

#### 步骤 4: 添加到映射

```cpp
// AuraAttributeSet.cpp
UAuraAttributeSet::UAuraAttributeSet()
{
    TagsToAttributes.Add(
        GameplayTags.Attributes_New_NewAttribute, 
        GetNewAttributeAttribute
    );
}
```

#### 步骤 5: 创建初始化 GameplayEffect

在编辑器中创建 GameplayEffect 来初始化新属性。

---

## 最佳实践

### 1. 属性设计

- **分类清晰**: 明确区分主属性、次属性、抗性等
- **命名规范**: 使用清晰的命名
- **文档完善**: 为每个属性添加描述

### 2. 性能优化

- **最小化复制**: 只复制必要的属性
- **批量更新**: 使用 GameplayEffect 批量修改属性
- **缓存计算**: 缓存频繁计算的属性值

### 3. 网络同步

- **服务器权威**: 属性修改在服务器执行
- **客户端预测**: 使用 GAS 的预测系统
- **合理复制**: 避免不必要的网络复制

---

## 总结

属性系统提供了完整的属性管理功能：

- ✅ **完整的属性体系**: 主属性、次属性、抗性、生命值
- ✅ **自动访问器**: 通过宏自动生成
- ✅ **属性计算**: 通过 MMC 计算次属性
- ✅ **变化回调**: 完整的回调系统
- ✅ **网络复制**: 支持多人游戏
- ✅ **Tag 映射**: 通过 Tag 访问属性

通过这个系统，开发者可以轻松管理和使用各种游戏属性。

