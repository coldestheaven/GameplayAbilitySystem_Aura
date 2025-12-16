# 角色系统文档

## 概述

角色系统是 Aura 项目的核心，定义了玩家角色和敌人角色的基础行为、属性管理和战斗接口。系统基于 Unreal Engine 的 Character 类，集成了 Gameplay Ability System (GAS) 和战斗接口。

## 核心组件

### AAuraCharacterBase

所有角色的基类，提供通用的角色功能。

#### 类层次结构

```
ACharacter (UE5 Base)
    ↓
AAuraCharacterBase (抽象基类)
    ├── AAuraCharacter (玩家角色)
    └── AAuraEnemy (敌人角色)
```

#### 核心功能

1. **GAS 集成**
   - 实现 `IAbilitySystemInterface`
   - 拥有 `UAuraAbilitySystemComponent`
   - 拥有 `UAuraAttributeSet`

2. **战斗接口**
   - 实现 `ICombatInterface`
   - 提供战斗相关方法（攻击、死亡、伤害等）

3. **角色属性**
   - 职业类型（Elementalist, Warrior, Ranger）
   - 武器系统
   - 动画蒙太奇
   - 状态标志（死亡、眩晕、燃烧、电击）

#### 关键属性

```cpp
// GAS 组件
UPROPERTY()
TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

UPROPERTY()
TObjectPtr<UAttributeSet> AttributeSet;

// 战斗相关
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
TObjectPtr<USkeletalMeshComponent> Weapon;

UPROPERTY(EditAnywhere, Category = "Combat")
TArray<FTaggedMontage> AttackMontages;

// 状态标志
UPROPERTY(ReplicatedUsing=OnRep_Stunned)
bool bIsStunned = false;

UPROPERTY(ReplicatedUsing=OnRep_Burned)
bool bIsBurned = false;

UPROPERTY(Replicated)
bool bIsBeingShocked = false;

UPROPERTY(BlueprintReadOnly)
bool bDead = false;
```

#### 关键方法

**GAS 相关**:
```cpp
virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
UAttributeSet* GetAttributeSet() const;
```

**战斗接口**:
```cpp
virtual void Die(const FVector& DeathImpulse) override;
virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag) override;
virtual bool IsDead_Implementation() const override;
virtual TArray<FTaggedMontage> GetAttackMontages_Implementation() override;
virtual USkeletalMeshComponent* GetWeapon_Implementation() override;
```

**伤害处理**:
```cpp
virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, 
    AController* EventInstigator, AActor* DamageCauser) override;
```

#### 委托系统

```cpp
// ASC 注册委托
FOnASCRegistered OnAscRegistered;

// 死亡委托
FOnDeathSignature OnDeathDelegate;

// 伤害委托
FOnDamageSignature OnDamageDelegate;
```

---

### AAuraCharacter

玩家角色类，继承自 `AAuraCharacterBase`。

#### 特殊功能

1. **玩家特定初始化**
   - 设置玩家控制器
   - 初始化玩家状态
   - 设置初始能力

2. **输入处理**
   - 通过 `AuraPlayerController` 处理输入
   - 绑定技能输入

3. **存档集成**
   - 实现 `IPlayerInterface`
   - 支持存档和加载

#### 关键实现

```cpp
void AAuraCharacter::InitAbilityActorInfo()
{
    // 从 PlayerState 获取 ASC
    AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
    check(AuraPlayerState);
    
    AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
    AbilitySystemComponent->InitAbilityActorInfo(AuraPlayerState, this);
    
    AttributeSet = AuraPlayerState->GetAttributeSet();
    
    // 添加角色能力
    if (AAuraPlayerController* AuraPlayerController = 
        Cast<AAuraPlayerController>(GetController()))
    {
        if (AAuraHUD* AuraHUD = AuraPlayerController->GetHUD<AAuraHUD>())
        {
            AuraHUD->InitOverlay(AuraPlayerController, AuraPlayerState, 
                AbilitySystemComponent, AttributeSet);
        }
    }
    
    InitializeDefaultAttributes();
    AddCharacterAbilities();
}
```

---

### AAuraEnemy

敌人角色类，继承自 `AAuraCharacterBase`。

#### 特殊功能

1. **AI 集成**
   - 使用 `AAuraAIController`
   - 行为树控制
   - 自动寻敌和攻击

2. **敌人特定属性**
   - 经验值奖励
   - 高亮系统
   - 敌人标签

3. **战斗行为**
   - 自动攻击玩家
   - 响应伤害
   - 死亡处理

#### 关键实现

```cpp
void AAuraEnemy::InitAbilityActorInfo()
{
    // 从自身获取 ASC（敌人拥有自己的 ASC）
    AbilitySystemComponent = NewObject<UAuraAbilitySystemComponent>(
        this, UAuraAbilitySystemComponent::StaticClass()
    );
    AbilitySystemComponent->SetIsReplicated(true);
    AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
    
    AttributeSet = NewObject<UAuraAttributeSet>(this);
    
    AbilitySystemComponent->InitAbilityActorInfo(this, this);
    
    InitializeDefaultAttributes();
    AddCharacterAbilities();
}
```

---

## 角色初始化流程

### 1. 构造函数

```cpp
AAuraCharacterBase::AAuraCharacterBase()
{
    // 创建武器组件
    Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
    Weapon->SetupAttachment(GetMesh(), "WeaponHandSocket");
    Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    
    // 设置 Socket 名称
    WeaponTipSocketName = "TipSocket";
    LeftHandSocketName = "LeftHandSocket";
    RightHandSocketName = "RightHandSocket";
    TailSocketName = "TailSocket";
}
```

### 2. BeginPlay

```cpp
void AAuraCharacterBase::BeginPlay()
{
    Super::BeginPlay();
    
    // 初始化 GAS
    InitAbilityActorInfo();
    
    // 应用初始属性
    InitializeDefaultAttributes();
    
    // 添加初始能力
    AddCharacterAbilities();
}
```

### 3. InitAbilityActorInfo

```cpp
void AAuraCharacterBase::InitAbilityActorInfo()
{
    // 子类实现
    // 玩家：从 PlayerState 获取 ASC
    // 敌人：创建自己的 ASC
}
```

### 4. InitializeDefaultAttributes

```cpp
void AAuraCharacterBase::InitializeDefaultAttributes() const
{
    // 从 CharacterClassInfo 获取职业默认属性
    // 应用主属性、次属性、生命值属性
}
```

### 5. AddCharacterAbilities

```cpp
void AAuraCharacterBase::AddCharacterAbilities()
{
    // 从 CharacterClassInfo 获取初始能力
    // 添加通用能力和职业特定能力
}
```

---

## 战斗系统

### 伤害处理

```cpp
float AAuraCharacterBase::TakeDamage(
    float DamageAmount,
    FDamageEvent const& DamageEvent,
    AController* EventInstigator,
    AActor* DamageCauser
)
{
    // 广播伤害委托（用于范围伤害）
    OnDamageDelegate.Broadcast(DamageAmount);
    
    return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}
```

### 死亡处理

```cpp
void AAuraCharacterBase::Die(const FVector& DeathImpulse)
{
    bDead = true;
    
    // 广播死亡委托
    OnDeathDelegate.Broadcast();
    
    // 多播死亡处理（网络同步）
    MulticastHandleDeath(DeathImpulse);
}

void AAuraCharacterBase::MulticastHandleDeath_Implementation(const FVector& DeathImpulse)
{
    // 禁用碰撞
    // 播放死亡动画
    // 应用死亡冲量
    // 启动溶解效果
}
```

### 战斗插槽

```cpp
FVector AAuraCharacterBase::GetCombatSocketLocation_Implementation(
    const FGameplayTag& MontageTag
) const
{
    // 根据标签返回对应的 Socket 位置
    // CombatSocket.RightHand -> RightHandSocketName
    // CombatSocket.LeftHand -> LeftHandSocketName
    // CombatSocket.Tail -> TailSocketName
}
```

---

## 状态管理

### 眩晕状态

```cpp
UPROPERTY(ReplicatedUsing=OnRep_Stunned)
bool bIsStunned = false;

void AAuraCharacterBase::OnRep_Stunned()
{
    // 更新 UI
    // 应用视觉效果
}
```

### 燃烧状态

```cpp
UPROPERTY(ReplicatedUsing=OnRep_Burned)
bool bIsBurned = false;

void AAuraCharacterBase::OnRep_Burned()
{
    // 更新视觉效果
    // 播放燃烧特效
}
```

### 电击状态

```cpp
UPROPERTY(Replicated)
bool bIsBeingShocked = false;

void AAuraCharacterBase::SetIsBeingShocked_Implementation(bool bInShock)
{
    bIsBeingShocked = bInShock;
}
```

---

## 职业系统

### 职业类型

```cpp
enum class ECharacterClass : uint8
{
    Elementalist,  // 元素师
    Warrior,       // 战士
    Ranger         // 游侠
};
```

### 职业配置

每个职业在 `CharacterClassInfo` 数据资产中配置：
- 主属性值
- 初始能力
- 经验值奖励

---

## 网络复制

### 复制属性

```cpp
void AAuraCharacterBase::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // 复制状态标志
    DOREPLIFETIME(AAuraCharacterBase, bIsStunned);
    DOREPLIFETIME(AAuraCharacterBase, bIsBurned);
    DOREPLIFETIME(AAuraCharacterBase, bIsBeingShocked);
    DOREPLIFETIME(AAuraCharacterBase, bDead);
}
```

---

## 相关文档

- [属性系统](./Attribute_System.md) - 属性管理
- [技能系统](../Core/Ability_System.md) - 能力系统
- [AI 系统](./AI_System.md) - 敌人 AI
- [交互系统](./Interaction_System.md) - 战斗接口

---

## 总结

角色系统是项目的核心，提供了：

1. ✅ **统一的角色基类** - 所有角色共享基础功能
2. ✅ **GAS 集成** - 完整的技能和属性系统
3. ✅ **战斗接口** - 标准化的战斗方法
4. ✅ **状态管理** - 眩晕、燃烧、电击等状态
5. ✅ **网络支持** - 完整的复制系统
6. ✅ **职业系统** - 多职业支持

通过这个系统，可以轻松创建新的角色类型，只需继承 `AAuraCharacterBase` 并实现特定功能即可。

