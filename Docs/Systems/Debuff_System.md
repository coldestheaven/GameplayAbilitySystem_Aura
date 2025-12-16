# Debuff 系统详细文档

## 目录

1. [系统概述](#系统概述)
2. [Debuff 类型](#debuff-类型)
3. [Debuff 触发机制](#debuff-触发机制)
4. [Debuff 应用流程](#debuff-应用流程)
5. [动态 Debuff 创建](#动态-debuff-创建)
6. [Debuff 参数](#debuff-参数)
7. [Debuff 视觉效果](#debuff-视觉效果)
8. [Debuff 堆叠](#debuff-堆叠)
9. [特殊 Debuff](#特殊-debuff)
10. [使用示例](#使用示例)

---

## 系统概述

Debuff 系统提供持续伤害和状态效果功能，支持多种 Debuff 类型，包括火焰燃烧、闪电眩晕、奥术和物理 Debuff。

### 核心组件

- **UExecCalc_Damage**: 伤害计算中确定 Debuff
- **UAuraAttributeSet::Debuff()**: 创建和应用 Debuff
- **UDebuffNiagaraComponent**: Debuff 视觉效果组件

### 系统特点

- ✅ 动态 Debuff 创建
- ✅ 持续伤害支持
- ✅ 视觉效果集成
- ✅ 堆叠控制
- ✅ 特殊效果支持（如眩晕）

---

## Debuff 类型

### 支持的 Debuff 类型

#### Burn (燃烧)

- **GameplayTag**: `Debuff.Burn`
- **伤害类型**: `Damage.Fire`
- **效果**: 持续火焰伤害
- **视觉效果**: 火焰粒子效果

#### Stun (眩晕)

- **GameplayTag**: `Debuff.Stun`
- **伤害类型**: `Damage.Lightning`
- **效果**: 禁用输入和光标追踪
- **视觉效果**: 闪电粒子效果

#### Arcane (奥术)

- **GameplayTag**: `Debuff.Arcane`
- **伤害类型**: `Damage.Arcane`
- **效果**: 持续奥术伤害
- **视觉效果**: 奥术粒子效果

#### Physical (物理)

- **GameplayTag**: `Debuff.Physical`
- **伤害类型**: `Damage.Physical`
- **效果**: 持续物理伤害
- **视觉效果**: 物理粒子效果

### 伤害类型到 Debuff 映射

```cpp
TMap<FGameplayTag, FGameplayTag> DamageTypesToDebuffs = {
    { Damage_Fire, Debuff_Burn },
    { Damage_Lightning, Debuff_Stun },
    { Damage_Arcane, Debuff_Arcane },
    { Damage_Physical, Debuff_Physical }
};
```

---

## Debuff 触发机制

### 触发判断

在 `UExecCalc_Damage::DetermineDebuff()` 中判断是否触发 Debuff。

#### 触发流程

```
1. 遍历所有伤害类型
   ↓
2. 检查是否有该类型的伤害
   ↓
3. 获取源 Debuff 触发几率
   ↓
4. 获取目标抗性值
   ↓
5. 计算有效 Debuff 几率
   EffectiveDebuffChance = SourceDebuffChance * (100 - Resistance) / 100
   ↓
6. 随机判断是否触发
   bDebuff = Random < EffectiveDebuffChance
   ↓
7. 如果触发，保存 Debuff 信息到 Context
```

#### 代码实现

```cpp
void UExecCalc_Damage::DetermineDebuff(...) const
{
    const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
    
    // 遍历所有伤害类型
    for (TTuple<FGameplayTag, FGameplayTag> Pair : GameplayTags.DamageTypesToDebuffs)
    {
        const FGameplayTag& DamageType = Pair.Key;
        const FGameplayTag& DebuffType = Pair.Value;
        
        // 检查是否有该类型的伤害
        const float TypeDamage = Spec.GetSetByCallerMagnitude(DamageType, false, -1.f);
        if (TypeDamage <= -.5f) continue; // .5 padding for floating point precision
        
        // 获取源 Debuff 触发几率
        const float SourceDebuffChance = Spec.GetSetByCallerMagnitude(
            GameplayTags.Debuff_Chance, 
            false, 
            -1.f
        );
        
        // 获取目标抗性
        float TargetDebuffResistance = 0.f;
        const FGameplayTag& ResistanceTag = GameplayTags.DamageTypesToResistances[DamageType];
        ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
            InTagsToDefs[ResistanceTag], 
            EvaluationParameters, 
            TargetDebuffResistance
        );
        TargetDebuffResistance = FMath::Max<float>(TargetDebuffResistance, 0.f);
        
        // 计算有效 Debuff 几率
        const float EffectiveDebuffChance = SourceDebuffChance * 
            (100 - TargetDebuffResistance) / 100.f;
        
        // 判断是否触发
        const bool bDebuff = FMath::RandRange(1, 100) < EffectiveDebuffChance;
        
        if (bDebuff)
        {
            // 保存 Debuff 信息
            FGameplayEffectContextHandle ContextHandle = Spec.GetContext();
            UAuraAbilitySystemLibrary::SetIsSuccessfulDebuff(ContextHandle, true);
            UAuraAbilitySystemLibrary::SetDamageType(ContextHandle, DamageType);
            
            // 保存 Debuff 参数
            const float DebuffDamage = Spec.GetSetByCallerMagnitude(
                GameplayTags.Debuff_Damage, 
                false, 
                -1.f
            );
            const float DebuffDuration = Spec.GetSetByCallerMagnitude(
                GameplayTags.Debuff_Duration, 
                false, 
                -1.f
            );
            const float DebuffFrequency = Spec.GetSetByCallerMagnitude(
                GameplayTags.Debuff_Frequency, 
                false, 
                -1.f
            );
            
            UAuraAbilitySystemLibrary::SetDebuffDamage(ContextHandle, DebuffDamage);
            UAuraAbilitySystemLibrary::SetDebuffDuration(ContextHandle, DebuffDuration);
            UAuraAbilitySystemLibrary::SetDebuffFrequency(ContextHandle, DebuffFrequency);
        }
    }
}
```

### Debuff 触发几率计算

```
EffectiveDebuffChance = SourceDebuffChance * (100 - TargetResistance) / 100
```

**说明**:
- **SourceDebuffChance**: 源（攻击者）的 Debuff 触发几率
- **TargetResistance**: 目标（受击者）的抗性值
- **EffectiveDebuffChance**: 有效 Debuff 触发几率

---

## Debuff 应用流程

### 应用时机

Debuff 在伤害处理后应用：

```
1. 计算伤害
   ↓
2. 应用伤害到 IncomingDamage
   ↓
3. 在 PostGameplayEffectExecute 中处理伤害
   ↓
4. 检查是否成功触发 Debuff
   ↓
5. 如果触发，调用 Debuff() 函数
```

### 应用代码

```cpp
void UAuraAttributeSet::HandleIncomingDamage(const FEffectProperties& Props)
{
    // ... 处理伤害 ...
    
    // 检查是否成功触发 Debuff
    if (UAuraAbilitySystemLibrary::IsSuccessfulDebuff(Props.EffectContextHandle))
    {
        Debuff(Props);
    }
}
```

---

## 动态 Debuff 创建

### Debuff() 函数

`UAuraAttributeSet::Debuff()` 函数动态创建 Debuff GameplayEffect。

#### 创建流程

```
1. 获取 Debuff 参数（伤害、持续时间、频率）
   ↓
2. 创建动态 GameplayEffect 对象
   ↓
3. 配置 GameplayEffect 属性
   - Duration Policy: HasDuration
   - Period: DebuffFrequency
   - Duration: DebuffDuration
   - Tags: DebuffTag
   ↓
4. 配置堆叠
   - StackingType: AggregateBySource
   - StackLimitCount: 1
   ↓
5. 添加修改器（伤害）
   ↓
6. 创建 Spec 并应用
```

#### 代码实现

```cpp
void UAuraAttributeSet::Debuff(const FEffectProperties& Props)
{
    const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
    
    // 创建效果上下文
    FGameplayEffectContextHandle EffectContext = Props.SourceASC->MakeEffectContext();
    EffectContext.AddSourceObject(Props.SourceAvatarActor);
    
    // 获取 Debuff 参数
    const FGameplayTag DamageType = UAuraAbilitySystemLibrary::GetDamageType(Props.EffectContextHandle);
    const float DebuffDamage = UAuraAbilitySystemLibrary::GetDebuffDamage(Props.EffectContextHandle);
    const float DebuffDuration = UAuraAbilitySystemLibrary::GetDebuffDuration(Props.EffectContextHandle);
    const float DebuffFrequency = UAuraAbilitySystemLibrary::GetDebuffFrequency(Props.EffectContextHandle);
    
    // 创建动态 GameplayEffect
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
    
    // 特殊处理：眩晕效果
    if (DebuffTag.MatchesTagExact(GameplayTags.Debuff_Stun))
    {
        Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_CursorTrace);
        Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_InputHeld);
        Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_InputPressed);
        Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_InputReleased);
    }
    
    // 配置堆叠
    Effect->StackingType = EGameplayEffectStackingType::AggregateBySource;
    Effect->StackLimitCount = 1;
    
    // 添加伤害修改器
    const int32 Index = Effect->Modifiers.Num();
    Effect->Modifiers.Add(FGameplayModifierInfo());
    FGameplayModifierInfo& ModifierInfo = Effect->Modifiers[Index];
    
    ModifierInfo.ModifierMagnitude = FScalableFloat(DebuffDamage);
    ModifierInfo.ModifierOp = EGameplayModOp::Additive;
    ModifierInfo.Attribute = UAuraAttributeSet::GetIncomingDamageAttribute();
    
    // 创建 Spec 并应用
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

## Debuff 参数

### 参数来源

Debuff 参数通过 SetByCaller 从技能中传递：

- **Debuff_Chance**: Debuff 触发几率
- **Debuff_Damage**: Debuff 伤害值
- **Debuff_Duration**: Debuff 持续时间
- **Debuff_Frequency**: Debuff 触发频率（周期）

### 参数设置

在技能中设置 Debuff 参数：

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

// 设置 Debuff 参数
SpecHandle.Data->SetSetByCallerMagnitude(
    FAuraGameplayTags::Get().Debuff_Chance, 
    50.f  // 50% 触发几率
);
SpecHandle.Data->SetSetByCallerMagnitude(
    FAuraGameplayTags::Get().Debuff_Damage, 
    10.f  // 每次触发 10 点伤害
);
SpecHandle.Data->SetSetByCallerMagnitude(
    FAuraGameplayTags::Get().Debuff_Duration, 
    5.f   // 持续 5 秒
);
SpecHandle.Data->SetSetByCallerMagnitude(
    FAuraGameplayTags::Get().Debuff_Frequency, 
    1.f   // 每秒触发一次
);
```

---

## Debuff 视觉效果

### UDebuffNiagaraComponent

Debuff 视觉效果通过 `UDebuffNiagaraComponent` 组件实现。

#### 组件初始化

```cpp
UDebuffNiagaraComponent::UDebuffNiagaraComponent()
{
    bAutoActivate = false;  // 不自动激活
}

void UDebuffNiagaraComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // 获取 AbilitySystemComponent
    if (UAbilitySystemComponent* ASC = 
        UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
    {
        // 注册 Tag 事件
        ASC->RegisterGameplayTagEvent(
            DebuffTag, 
            EGameplayTagEventType::NewOrRemoved
        ).AddUObject(this, &UDebuffNiagaraComponent::DebuffTagChanged);
    }
    else if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetOwner()))
    {
        // 如果 ASC 还未注册，等待注册
        CombatInterface->GetOnASCRegisteredDelegate().AddWeakLambda(
            this, 
            [this](UAbilitySystemComponent* InASC)
            {
                InASC->RegisterGameplayTagEvent(
                    DebuffTag, 
                    EGameplayTagEventType::NewOrRemoved
                ).AddUObject(this, &UDebuffNiagaraComponent::DebuffTagChanged);
            }
        );
    }
}
```

#### Tag 变化处理

```cpp
void UDebuffNiagaraComponent::DebuffTagChanged(
    const FGameplayTag CallbackTag, 
    int32 NewCount
)
{
    const bool bOwnerValid = IsValid(GetOwner());
    const bool bOwnerAlive = GetOwner()->Implements<UCombatInterface>() && 
        !ICombatInterface::Execute_IsDead(GetOwner());
    
    // 如果 Debuff 存在且角色存活，激活效果
    if (NewCount > 0 && bOwnerValid && bOwnerAlive)
    {
        Activate();
    }
    else
    {
        Deactivate();
    }
}
```

### 在角色中添加组件

```cpp
// 在 AuraCharacterBase 中
UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
TObjectPtr<UDebuffNiagaraComponent> BurnDebuffComponent;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
TObjectPtr<UDebuffNiagaraComponent> StunDebuffComponent;

// 在构造函数中
BurnDebuffComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>(
    TEXT("BurnDebuffComponent")
);
BurnDebuffComponent->SetupAttachment(GetRootComponent());
BurnDebuffComponent->DebuffTag = FAuraGameplayTags::Get().Debuff_Burn;
BurnDebuffComponent->SetAsset(BurnNiagaraSystem);  // 设置 Niagara 系统

// ... 其他 Debuff 组件
```

---

## Debuff 堆叠

### 堆叠配置

```cpp
// 在 Debuff() 函数中
Effect->StackingType = EGameplayEffectStackingType::AggregateBySource;
Effect->StackLimitCount = 1;
```

**说明**:
- **AggregateBySource**: 按源聚合（同一源的效果堆叠）
- **StackLimitCount**: 堆叠上限为 1（同一源只能有一个 Debuff）

### 堆叠行为

- **同一源**: 如果同一源再次触发 Debuff，会刷新持续时间
- **不同源**: 不同源的 Debuff 可以同时存在
- **堆叠上限**: 每个源的 Debuff 最多堆叠 1 次

---

## 特殊 Debuff

### Stun (眩晕)

眩晕 Debuff 有特殊效果：禁用玩家输入。

#### 特殊 Tag

```cpp
if (DebuffTag.MatchesTagExact(GameplayTags.Debuff_Stun))
{
    Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_CursorTrace);
    Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_InputHeld);
    Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_InputPressed);
    Effect->InheritableOwnedTagsContainer.AddTag(GameplayTags.Player_Block_InputReleased);
}
```

#### 输入阻止

这些 Tag 会阻止玩家输入：

- **Player_Block_CursorTrace**: 阻止光标追踪
- **Player_Block_InputHeld**: 阻止输入按住
- **Player_Block_InputPressed**: 阻止输入按下
- **Player_Block_InputReleased**: 阻止输入释放

#### 实现方式

在输入处理中检查 Tag：

```cpp
// 在输入处理中
if (AbilitySystemComponent->HasMatchingGameplayTag(
    FAuraGameplayTags::Get().Player_Block_InputPressed))
{
    return;  // 阻止输入
}
```

---

## 使用示例

### 在技能中配置 Debuff

```cpp
// 在技能蓝图中或 C++ 中
void UAuraFireBolt::OnSpellHit()
{
    // 创建伤害效果 Spec
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
    
    // 设置 Debuff 参数
    SpecHandle.Data->SetSetByCallerMagnitude(
        FAuraGameplayTags::Get().Debuff_Chance, 
        75.f  // 75% 触发几率
    );
    SpecHandle.Data->SetSetByCallerMagnitude(
        FAuraGameplayTags::Get().Debuff_Damage, 
        15.f  // 每次 15 点伤害
    );
    SpecHandle.Data->SetSetByCallerMagnitude(
        FAuraGameplayTags::Get().Debuff_Duration, 
        6.f   // 持续 6 秒
    );
    SpecHandle.Data->SetSetByCallerMagnitude(
        FAuraGameplayTags::Get().Debuff_Frequency, 
        0.5f  // 每 0.5 秒触发一次
    );
    
    // 应用效果
    ASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
}
```

### 添加新的 Debuff 类型

#### 步骤 1: 添加 GameplayTag

```cpp
// AuraGameplayTags.h
FGameplayTag Debuff_Poison;

// AuraGameplayTags.cpp
GameplayTags.Debuff_Poison = UGameplayTagsManager::Get()
    .AddNativeGameplayTag(FName("Debuff.Poison"));
```

#### 步骤 2: 添加伤害类型映射

```cpp
// AuraGameplayTags.cpp
GameplayTags.DamageTypesToDebuffs.Add(
    GameplayTags.Damage_Poison,
    GameplayTags.Debuff_Poison
);
```

#### 步骤 3: 创建视觉效果组件

```cpp
// 在角色中
UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
TObjectPtr<UDebuffNiagaraComponent> PoisonDebuffComponent;

// 在构造函数中
PoisonDebuffComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>(
    TEXT("PoisonDebuffComponent")
);
PoisonDebuffComponent->SetupAttachment(GetRootComponent());
PoisonDebuffComponent->DebuffTag = FAuraGameplayTags::Get().Debuff_Poison;
PoisonDebuffComponent->SetAsset(PoisonNiagaraSystem);
```

---

## 最佳实践

### 1. Debuff 设计

- **平衡性**: 确保 Debuff 效果平衡
- **可配置**: 使用数据资产配置 Debuff 参数
- **清晰反馈**: 提供清晰的视觉和音频反馈

### 2. 性能优化

- **按需创建**: 只在需要时创建 Debuff 效果
- **及时清理**: 在适当时机清理 Debuff 效果
- **效果复用**: 复用相同的视觉效果

### 3. 用户体验

- **视觉反馈**: 提供清晰的视觉反馈
- **状态显示**: 在 UI 中显示 Debuff 状态
- **持续时间**: 显示 Debuff 剩余时间

---

## 总结

Debuff 系统提供了完整的持续伤害和状态效果功能：

- ✅ **动态创建**: 动态创建 Debuff GameplayEffect
- ✅ **持续伤害**: 支持周期性持续伤害
- ✅ **视觉效果**: 集成 Niagara 粒子效果
- ✅ **堆叠控制**: 支持堆叠控制
- ✅ **特殊效果**: 支持特殊效果（如眩晕）

通过这个系统，开发者可以创建丰富多样的战斗效果。

