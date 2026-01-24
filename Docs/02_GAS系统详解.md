# GAS 系统详解

> **文档**: Gameplay Ability System 深度解析  
> **关联**: [01_架构总览.md](./01_架构总览.md)  
> **更新日期**: 2026-01-24

---

## 📋 目录

1. [GAS 概述](#gas-概述)
2. [核心组件](#核心组件)
3. [属性系统](#属性系统)
4. [技能系统](#技能系统)
5. [效果系统](#效果系统)
6. [标签系统](#标签系统)
7. [网络复制](#网络复制)
8. [扩展机制](#扩展机制)

---

## 🎯 GAS 概述

### 什么是 GAS？
**Gameplay Ability System (GAS)** 是 Unreal Engine 提供的一个强大的游戏能力框架，用于实现：
- ✅ 技能系统（释放技能、冷却、消耗）
- ✅ 属性系统（生命、法力、攻击力等）
- ✅ 效果系统（Buff、Debuff、伤害计算）
- ✅ 标签系统（状态标记、条件判断）
- ✅ 网络复制（多人游戏支持）

### Aura 项目中的 GAS 架构
```
AuraCharacterBase (拥有 ASC)
  ├── AuraAbilitySystemComponent (核心组件)
  │     ├── 管理技能 (GameplayAbilities)
  │     ├── 管理效果 (GameplayEffects)
  │     └── 管理标签 (GameplayTags)
  │
  └── AuraAttributeSet (属性集)
        ├── 主属性 (力量、智力等)
        ├── 次级属性 (护甲、暴击等)
        ├── 抗性属性 (火焰抗性等)
        └── 生命属性 (生命值、法力值)
```

---

## 🧩 核心组件

### 1. AbilitySystemComponent (ASC)

#### 类定义
```cpp
// AuraAbilitySystemComponent.h
UCLASS()
class AURA_API UAuraAbilitySystemComponent : public UAbilitySystemComponent
{
    GENERATED_BODY()
public:
    // 初始化
    void AbilityActorInfoSet();
    
    // 技能管理
    void AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities);
    void AddCharacterPassiveAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities);
    
    // 输入处理
    void AbilityInputTagPressed(const FGameplayTag& InputTag);
    void AbilityInputTagHeld(const FGameplayTag& InputTag);
    void AbilityInputTagReleased(const FGameplayTag& InputTag);
    
    // 技能查询
    FGameplayAbilitySpec* GetSpecFromAbilityTag(const FGameplayTag& AbilityTag);
    FGameplayTag GetStatusFromAbilityTag(const FGameplayTag& AbilityTag);
    
    // 技能升级
    void UpdateAbilityStatuses(int32 Level);
    
    // 委托
    FEffectAssetTags EffectAssetTags;
    FAbilitiesGiven AbilitiesGivenDelegate;
    FAbilityStatusChanged AbilityStatusChanged;
    FAbilityEquipped AbilityEquipped;
};
```

#### 职责
- **技能生命周期管理** - 授予、激活、取消、结束
- **输入绑定** - 将输入映射到技能
- **效果应用** - 管理 GameplayEffects
- **标签管理** - 添加/移除 GameplayTags
- **网络复制** - 同步技能状态

#### 初始化流程
```cpp
// 在 PlayerState 中创建 ASC
AAuraPlayerState::AAuraPlayerState()
{
    AbilitySystemComponent = CreateDefaultSubobject<UAuraAbilitySystemComponent>("AbilitySystemComponent");
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

// 在 Character 中初始化
void AAuraCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    InitAbilityActorInfo();  // 服务器端
}

void AAuraCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    InitAbilityActorInfo();  // 客户端
}

void AAuraCharacter::InitAbilityActorInfo()
{
    AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
    AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
    AbilitySystemComponent->InitAbilityActorInfo(AuraPlayerState, this);
    Cast<UAuraAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
}
```

---

### 2. AttributeSet

#### 类定义
```cpp
// AuraAttributeSet.h
UCLASS()
class AURA_API UAuraAttributeSet : public UAttributeSet
{
    GENERATED_BODY()
public:
    // 属性变化钩子
    virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
    virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
    virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
    
    // 主属性
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "Primary Attributes")
    FGameplayAttributeData Strength;
    ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Strength);
    
    // 次级属性
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Armor, Category = "Secondary Attributes")
    FGameplayAttributeData Armor;
    ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Armor);
    
    // 生命属性
    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Vital Attributes")
    FGameplayAttributeData Health;
    ATTRIBUTE_ACCESSORS(UAuraAttributeSet, Health);
    
    // Meta 属性（不复制）
    UPROPERTY(BlueprintReadOnly, Category = "Meta Attributes")
    FGameplayAttributeData IncomingDamage;
    ATTRIBUTE_ACCESSORS(UAuraAttributeSet, IncomingDamage);
};
```

#### 属性分类

| 类别 | 属性 | 说明 |
|------|------|------|
| **主属性** | Strength, Intelligence, Resilience, Vigor | 基础属性，影响次级属性 |
| **次级属性** | Armor, ArmorPenetration, BlockChance, CriticalHitChance, etc. | 由主属性计算得出 |
| **抗性属性** | FireResistance, LightningResistance, ArcaneResistance, PhysicalResistance | 元素抗性 |
| **生命属性** | Health, Mana, MaxHealth, MaxMana | 当前值和最大值 |
| **Meta 属性** | IncomingDamage, IncomingXP | 临时计算属性 |

#### 属性计算流程
```cpp
// 1. PreAttributeChange - 限制新值范围
void UAuraAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
    if (Attribute == GetHealthAttribute())
    {
        NewValue = FMath::Clamp(NewValue, 0.f, GetMaxHealth());
    }
}

// 2. PostGameplayEffectExecute - 处理效果应用后的逻辑
void UAuraAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    // 处理伤害
    if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
    {
        const float LocalIncomingDamage = GetIncomingDamage();
        SetIncomingDamage(0.f);
        
        if (LocalIncomingDamage > 0.f)
        {
            const float NewHealth = GetHealth() - LocalIncomingDamage;
            SetHealth(FMath::Clamp(NewHealth, 0.f, GetMaxHealth()));
        }
    }
}

// 3. PostAttributeChange - 属性变化后的回调
void UAuraAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    // 可以在这里触发 UI 更新等
}
```

---

## 🎮 技能系统

### 技能类层次结构
```
UGameplayAbility (引擎基类)
  └── UAuraGameplayAbility (项目基类)
        ├── UAuraDamageGameplayAbility (伤害技能基类)
        │     ├── UAuraProjectileSpell (投射物技能)
        │     │     ├── UAuraFireBolt (火球术)
        │     │     └── UArcaneShards (奥术碎片)
        │     ├── UAuraBeamSpell (光束技能)
        │     │     └── UElectrocute (电击)
        │     ├── UAuraMeleeAttack (近战攻击)
        │     └── UAuraFireBlast (火焰爆炸)
        ├── UAuraSummonAbility (召唤技能)
        └── UAuraPassiveAbility (被动技能)
```

### 技能基类设计
```cpp
// AuraGameplayAbility.h
UCLASS()
class AURA_API UAuraGameplayAbility : public UGameplayAbility
{
    GENERATED_BODY()
public:
    // 技能标签
    UPROPERTY(EditDefaultsOnly, Category = "Ability")
    FGameplayTag StartupInputTag;
    
    // 获取技能描述
    virtual FString GetDescription(int32 Level);
    virtual FString GetNextLevelDescription(int32 Level);
    
    // 获取锁定描述
    static FString GetLockedDescription(int32 Level);
};
```

### 伤害技能基类
```cpp
// AuraDamageGameplayAbility.h
UCLASS()
class AURA_API UAuraDamageGameplayAbility : public UAuraGameplayAbility
{
    GENERATED_BODY()
public:
    // 伤害参数
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<UGameplayEffect> DamageEffectClass;
    
    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    TMap<FGameplayTag, FScalableFloat> DamageTypes;
    
    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    float DebuffChance = 20.f;
    
    UPROPERTY(EditDefaultsOnly, Category = "Damage")
    float DebuffDamage = 5.f;
    
    // 创建伤害参数
    UFUNCTION(BlueprintCallable)
    FDamageEffectParams MakeDamageEffectParamsFromClassDefaults(
        AActor* TargetActor = nullptr,
        FVector InRadialDamageOrigin = FVector::ZeroVector,
        bool bOverrideKnockbackDirection = false,
        FVector KnockbackDirectionOverride = FVector::ZeroVector,
        bool bOverrideDeathImpulse = false,
        FVector DeathImpulseDirectionOverride = FVector::ZeroVector,
        bool bOverridePitch = false,
        float PitchOverride = 0.f
    ) const;
    
    // 应用伤害效果
    UFUNCTION(BlueprintCallable)
    float GetDamageAtLevel() const;
};
```

### 投射物技能示例
```cpp
// AuraProjectileSpell.h
UCLASS()
class AURA_API UAuraProjectileSpell : public UAuraDamageGameplayAbility
{
    GENERATED_BODY()
public:
    // 投射物类
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<AAuraProjectile> ProjectileClass;
    
    // 投射物数量
    UPROPERTY(EditDefaultsOnly)
    int32 NumProjectiles = 5;
    
protected:
    virtual void ActivateAbility(...) override;
    
    // 生成投射物
    UFUNCTION(BlueprintCallable, Category = "Projectile")
    void SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch = false, float PitchOverride = 0.f);
};

// AuraProjectileSpell.cpp
void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, bool bOverridePitch, float PitchOverride)
{
    // 获取发射位置
    const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), SocketTag);
    
    // 计算旋转
    FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
    if (bOverridePitch) Rotation.Pitch = PitchOverride;
    
    // 生成投射物
    FTransform SpawnTransform;
    SpawnTransform.SetLocation(SocketLocation);
    SpawnTransform.SetRotation(Rotation.Quaternion());
    
    AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
        ProjectileClass,
        SpawnTransform,
        GetOwningActorFromActorInfo(),
        Cast<APawn>(GetOwningActorFromActorInfo()),
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );
    
    // 设置伤害参数
    Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
    
    Projectile->FinishSpawning(SpawnTransform);
}
```

### 技能激活流程
```
1. 玩家输入
   ↓
2. AuraPlayerController::AbilityInputTagPressed
   ↓
3. AuraAbilitySystemComponent::AbilityInputTagPressed
   ↓
4. 查找对应的 AbilitySpec
   ↓
5. TryActivateAbility
   ↓
6. UAuraGameplayAbility::ActivateAbility
   ↓
7. 执行技能逻辑（生成投射物、应用效果等）
   ↓
8. EndAbility
```

---

## ⚡ 效果系统

### GameplayEffect 类型

| 类型 | 说明 | 示例 |
|------|------|------|
| **Instant** | 瞬时效果 | 伤害、治疗 |
| **Duration** | 持续效果 | Buff、Debuff |
| **Infinite** | 永久效果 | 被动技能 |

### 自定义 EffectContext
```cpp
// AuraAbilityTypes.h
USTRUCT(BlueprintType)
struct FAuraGameplayEffectContext : public FGameplayEffectContext
{
    GENERATED_BODY()
public:
    // 自定义数据
    bool IsCriticalHit() const { return bIsCriticalHit; }
    bool IsBlockedHit() const { return bIsBlockedHit; }
    bool IsSuccessfulDebuff() const { return bIsSuccessfulDebuff; }
    float GetDebuffDamage() const { return DebuffDamage; }
    TSharedPtr<FGameplayTag> GetDamageType() const { return DamageType; }
    FVector GetDeathImpulse() const { return DeathImpulse; }
    FVector GetKnockbackForce() const { return KnockbackForce; }
    bool IsRadialDamage() const { return bIsRadialDamage; }
    
    // 网络序列化
    virtual bool NetSerialize(FArchive& Ar, class UPackageMap* Map, bool& bOutSuccess) override;
    
protected:
    UPROPERTY()
    bool bIsCriticalHit = false;
    
    UPROPERTY()
    bool bIsBlockedHit = false;
    
    UPROPERTY()
    bool bIsSuccessfulDebuff = false;
    
    UPROPERTY()
    float DebuffDamage = 0.f;
    
    TSharedPtr<FGameplayTag> DamageType;
    
    UPROPERTY()
    FVector DeathImpulse = FVector::ZeroVector;
    
    UPROPERTY()
    FVector KnockbackForce = FVector::ZeroVector;
    
    UPROPERTY()
    bool bIsRadialDamage = false;
};
```

### 伤害计算 (ExecCalc)
```cpp
// ExecCalc_Damage.h
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

// ExecCalc_Damage.cpp - 简化版
void UExecCalc_Damage::Execute_Implementation(...)
{
    // 1. 获取源和目标属性
    float SourceArmorPenetration = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenDef, EvaluationParameters, SourceArmorPenetration);
    
    float TargetArmor = 0.f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, TargetArmor);
    
    // 2. 计算有效护甲
    const float EffectiveArmor = TargetArmor * (100 - SourceArmorPenetration) / 100.f;
    
    // 3. 计算伤害减免
    const float DamageReduction = EffectiveArmor * 0.04f;  // 每点护甲减免 4% 伤害
    
    // 4. 计算最终伤害
    float Damage = BaseDamage * (100 - DamageReduction) / 100.f;
    
    // 5. 检查格挡
    if (FMath::RandRange(1, 100) < TargetBlockChance)
    {
        Damage *= 0.5f;  // 格挡减免 50% 伤害
        AuraEffectContext->SetIsBlockedHit(true);
    }
    
    // 6. 检查暴击
    if (FMath::RandRange(1, 100) < SourceCriticalHitChance)
    {
        Damage *= 2.f + SourceCriticalHitDamage / 100.f;
        AuraEffectContext->SetIsCriticalHit(true);
    }
    
    // 7. 应用伤害
    OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(
        DamageStatics().IncomingDamageDef.AttributeToCapture,
        EGameplayModOp::Additive,
        Damage
    ));
}
```

### ModMagnitudeCalculation (MMC)
```cpp
// MMC_MaxHealth.h - 最大生命值计算
UCLASS()
class AURA_API UMMC_MaxHealth : public UGameplayModMagnitudeCalculation
{
    GENERATED_BODY()
public:
    UMMC_MaxHealth();
    
    virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
    
private:
    FGameplayEffectAttributeCaptureDefinition VigorDef;
};

// MMC_MaxHealth.cpp
float UMMC_MaxHealth::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
    // 获取 Vigor 属性
    float Vigor = 0.f;
    GetCapturedAttributeMagnitude(VigorDef, Spec, FAggregatorEvaluateParameters(), Vigor);
    Vigor = FMath::Max<float>(Vigor, 0.f);
    
    // 获取角色等级
    int32 PlayerLevel = 1;
    if (Spec.GetContext().GetSourceObject()->Implements<UCombatInterface>())
    {
        PlayerLevel = ICombatInterface::Execute_GetPlayerLevel(Spec.GetContext().GetSourceObject());
    }
    
    // 计算最大生命值: 80 + (Vigor * 2.5) + (Level * 10)
    return 80.f + (Vigor * 2.5f) + (PlayerLevel * 10.f);
}
```

---

## 🏷️ 标签系统

### GameplayTags 架构
```cpp
// AuraGameplayTags.h
struct FAuraGameplayTags
{
public:
    static const FAuraGameplayTags& Get() { return GameplayTags; }
    static void InitializeNativeGameplayTags();
    
    // 属性标签
    FGameplayTag Attributes_Primary_Strength;
    FGameplayTag Attributes_Primary_Intelligence;
    FGameplayTag Attributes_Secondary_Armor;
    FGameplayTag Attributes_Secondary_CriticalHitChance;
    
    // 输入标签
    FGameplayTag InputTag_LMB;
    FGameplayTag InputTag_RMB;
    FGameplayTag InputTag_1;
    FGameplayTag InputTag_2;
    
    // 伤害类型标签
    FGameplayTag Damage_Fire;
    FGameplayTag Damage_Lightning;
    FGameplayTag Damage_Arcane;
    FGameplayTag Damage_Physical;
    
    // 技能标签
    FGameplayTag Abilities_Fire_FireBolt;
    FGameplayTag Abilities_Lightning_Electrocute;
    
    // 技能状态标签
    FGameplayTag Abilities_Status_Locked;
    FGameplayTag Abilities_Status_Eligible;
    FGameplayTag Abilities_Status_Unlocked;
    FGameplayTag Abilities_Status_Equipped;
    
    // Debuff 标签
    FGameplayTag Debuff_Burn;
    FGameplayTag Debuff_Stun;
    
    // 映射表
    TMap<FGameplayTag, FGameplayTag> DamageTypesToResistances;
    TMap<FGameplayTag, FGameplayTag> DamageTypesToDebuffs;
    
private:
    static FAuraGameplayTags GameplayTags;
};
```

### 标签初始化
```cpp
// AuraGameplayTags.cpp
void FAuraGameplayTags::InitializeNativeGameplayTags()
{
    // 主属性
    GameplayTags.Attributes_Primary_Strength = UGameplayTagsManager::Get().AddNativeGameplayTag(
        FName("Attributes.Primary.Strength"),
        FString("Increases physical damage")
    );
    
    // 伤害类型
    GameplayTags.Damage_Fire = UGameplayTagsManager::Get().AddNativeGameplayTag(
        FName("Damage.Fire"),
        FString("Fire Damage Type")
    );
    
    // 建立映射关系
    GameplayTags.DamageTypesToResistances.Add(GameplayTags.Damage_Fire, GameplayTags.Attributes_Resistance_Fire);
    GameplayTags.DamageTypesToDebuffs.Add(GameplayTags.Damage_Fire, GameplayTags.Debuff_Burn);
}
```

### 标签使用场景

| 场景 | 说明 | 示例 |
|------|------|------|
| **技能激活条件** | 检查是否满足激活条件 | 需要特定 Buff 才能释放 |
| **技能阻止** | 阻止特定技能激活 | 眩晕状态无法释放技能 |
| **效果应用条件** | 条件性应用效果 | 只对带有特定标签的目标生效 |
| **属性映射** | 标签到属性的映射 | 伤害类型 → 抗性属性 |
| **输入绑定** | 输入到技能的映射 | InputTag_1 → FireBolt |

---

## 🌐 网络复制

### ASC 复制模式
```cpp
// 在 PlayerState 构造函数中设置
AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
```

| 模式 | 说明 | 适用场景 |
|------|------|----------|
| **Full** | 完全复制所有数据 | 单人游戏或小规模多人 |
| **Mixed** | 只复制 GameplayEffects 给所有者 | 多人游戏（推荐） |
| **Minimal** | 最小复制 | 大规模多人游戏 |

### 属性复制
```cpp
// 属性自动复制
UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Vital Attributes")
FGameplayAttributeData Health;

UFUNCTION()
void OnRep_Health(const FGameplayAttributeData& OldHealth) const
{
    // 通知 ASC 属性已复制
    GAMEPLAYATTRIBUTE_REPNOTIFY(UAuraAttributeSet, Health, OldHealth);
}

// 注册复制属性
void UAuraAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_Always);
    DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Mana, COND_None, REPNOTIFY_Always);
}
```

### RPC 调用
```cpp
// 服务器 RPC - 客户端请求升级属性
UFUNCTION(Server, Reliable)
void ServerUpgradeAttribute(const FGameplayTag& AttributeTag);

void UAuraAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
    // 在服务器上执行升级逻辑
    FGameplayEventData Payload;
    Payload.EventTag = AttributeTag;
    Payload.EventMagnitude = 1.f;
    
    UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);
}

// 客户端 RPC - 服务器通知客户端
UFUNCTION(Client, Reliable)
void ClientUpdateAbilityStatus(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel);

void UAuraAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, int32 AbilityLevel)
{
    // 在客户端更新 UI
    AbilityStatusChanged.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}
```

---

## 🔧 扩展机制

### 1. 自定义 AbilitySystemGlobals
```cpp
// AuraAbilitySystemGlobals.h
UCLASS()
class AURA_API UAuraAbilitySystemGlobals : public UAbilitySystemGlobals
{
    GENERATED_BODY()
public:
    virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};

// AuraAbilitySystemGlobals.cpp
FGameplayEffectContext* UAuraAbilitySystemGlobals::AllocGameplayEffectContext() const
{
    return new FAuraGameplayEffectContext();
}
```

### 2. AbilityTasks
```cpp
// TargetDataUnderMouse.h - 获取鼠标下的目标
UCLASS()
class AURA_API UTargetDataUnderMouse : public UAbilityTask
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "TargetDataUnderMouse", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
    static UTargetDataUnderMouse* CreateTargetDataUnderMouse(UGameplayAbility* OwningAbility);
    
    UPROPERTY(BlueprintAssignable)
    FMouseTargetDataSignature ValidData;
    
private:
    virtual void Activate() override;
    void SendMouseCursorData();
    void OnTargetDataReplicatedCallback(const FGameplayAbilityTargetDataHandle& DataHandle, FGameplayTag ActivationTag);
};
```

### 3. AsyncTasks
```cpp
// WaitCooldownChange.h - 监听冷却变化
UCLASS(BlueprintType, meta = (ExposedAsyncProxy = "AsyncTask"))
class AURA_API UWaitCooldownChange : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintAssignable)
    FCooldownChangeSignature CooldownStart;
    
    UPROPERTY(BlueprintAssignable)
    FCooldownChangeSignature CooldownEnd;
    
    UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
    static UWaitCooldownChange* WaitForCooldownChange(UAbilitySystemComponent* AbilitySystemComponent, const FGameplayTag& InCooldownTag);
    
    UFUNCTION(BlueprintCallable)
    void EndTask();
    
protected:
    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> ASC;
    
    FGameplayTag CooldownTag;
    
    void CooldownTagChanged(const FGameplayTag InCooldownTag, int32 NewCount);
    void OnActiveEffectAdded(UAbilitySystemComponent* TargetASC, const FGameplayEffectSpec& SpecApplied, FActiveGameplayEffectHandle ActiveEffectHandle);
};
```

---

## 📊 性能优化建议

### 1. 属性复制优化
```cpp
// 只复制给所有者
DOREPLIFETIME_CONDITION(UAuraAttributeSet, Health, COND_OwnerOnly);

// 只在变化时复制
DOREPLIFETIME_CONDITION_NOTIFY(UAuraAttributeSet, Health, COND_None, REPNOTIFY_OnChanged);
```

### 2. 效果应用优化
```cpp
// 使用对象池管理 GameplayEffectSpec
// 批量应用效果
TArray<FActiveGameplayEffectHandle> AppliedEffects;
for (const auto& EffectClass : EffectsToApply)
{
    FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(EffectClass, Level, EffectContext);
    AppliedEffects.Add(ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get()));
}
```

### 3. 标签查询优化
```cpp
// 缓存常用标签查询
FGameplayTagContainer OwnedTags;
ASC->GetOwnedGameplayTags(OwnedTags);

// 使用 HasMatchingGameplayTag 而不是 HasTag
if (ASC->HasMatchingGameplayTag(FAuraGameplayTags::Get().Abilities_Status_Locked))
{
    // ...
}
```

---

## 📖 相关文档

- [01_架构总览.md](./01_架构总览.md)
- [03_角色系统架构.md](./03_角色系统架构.md)
- [04_UI架构设计.md](./04_UI架构设计.md)

---

**文档维护**: GAS 系统是项目核心，任何修改请及时更新文档