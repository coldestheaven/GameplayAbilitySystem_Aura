# 玩家系统文档

## 概述

玩家系统管理玩家相关的所有功能，包括玩家状态、玩家控制器、经验值、等级、属性点和法术点等。系统基于 Unreal Engine 的 PlayerState 和 PlayerController，集成了 GAS 和 UI 系统。

## 核心组件

### AAuraPlayerState

玩家状态类，存储玩家的持久化数据。

#### 类层次结构

```
APlayerState (UE5 Base)
    ↓
AAuraPlayerState
```

#### 核心功能

1. **GAS 集成**
   - 拥有 `UAuraAbilitySystemComponent`
   - 拥有 `UAuraAttributeSet`
   - 作为 GAS 的 Avatar Actor

2. **玩家进度**
   - 等级（Level）
   - 经验值（XP）
   - 属性点（Attribute Points）
   - 法术点（Spell Points）

3. **委托系统**
   - 等级变化委托
   - 经验值变化委托
   - 属性点变化委托
   - 法术点变化委托

#### 关键属性

```cpp
// GAS 组件
UPROPERTY(VisibleAnywhere)
TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

UPROPERTY()
TObjectPtr<UAttributeSet> AttributeSet;

// 等级信息
UPROPERTY(EditDefaultsOnly)
TObjectPtr<ULevelUpInfo> LevelUpInfo;

// 玩家进度（网络复制）
UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_Level)
int32 Level = 1;

UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_XP)
int32 XP = 0;

UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_AttributePoints)
int32 AttributePoints = 0;

UPROPERTY(VisibleAnywhere, ReplicatedUsing=OnRep_SpellPoints)
int32 SpellPoints = 0;
```

#### 关键方法

**获取方法**:
```cpp
FORCEINLINE int32 GetPlayerLevel() const { return Level; }
FORCEINLINE int32 GetXP() const { return XP; }
FORCEINLINE int32 GetAttributePoints() const { return AttributePoints; }
FORCEINLINE int32 GetSpellPoints() const { return SpellPoints; }
```

**设置方法**:
```cpp
void AddToXP(int32 InXP);
void AddToLevel(int32 InLevel);
void AddToAttributePoints(int32 InPoints);
void AddToSpellPoints(int32 InPoints);

void SetXP(int32 InXP);
void SetLevel(int32 InLevel);
void SetAttributePoints(int32 InPoints);
void SetSpellPoints(int32 InPoints);
```

#### 委托系统

```cpp
// 经验值变化
FOnPlayerStatChanged OnXPChangedDelegate;

// 等级变化（包含是否升级标志）
FOnLevelChanged OnLevelChangedDelegate;

// 属性点变化
FOnPlayerStatChanged OnAttributePointsChangedDelegate;

// 法术点变化
FOnPlayerStatChanged OnSpellPointsChangedDelegate;
```

#### 经验值系统

```cpp
void AAuraPlayerState::AddToXP(int32 InXP)
{
    XP = FMath::Clamp(XP + InXP, 0, MAX_int32);
    OnXPChangedDelegate.Broadcast(XP);
    
    // 检查是否升级
    const int32 NewLevel = LevelUpInfo->FindLevelForXP(XP);
    const int32 NumLevelUps = NewLevel - Level;
    
    if (NumLevelUps > 0)
    {
        for (int32 i = 0; i < NumLevelUps; ++i)
        {
            Level++;
            OnLevelChangedDelegate.Broadcast(Level, true);
            
            // 升级奖励
            AddToAttributePoints(LevelUpInfo->GetAttributePointReward(Level));
            AddToSpellPoints(LevelUpInfo->GetSpellPointReward(Level));
        }
    }
}
```

---

### AAuraPlayerController

玩家控制器类，处理玩家输入和 UI 管理。

#### 类层次结构

```
APlayerController (UE5 Base)
    ↓
AAuraPlayerController
```

#### 核心功能

1. **输入处理**
   - Enhanced Input 集成
   - 技能输入绑定
   - 鼠标点击处理

2. **UI 管理**
   - HUD 初始化
   - Widget Controller 管理
   - 覆盖层 UI

3. **鼠标交互**
   - 鼠标悬停检测
   - 点击目标选择
   - 高亮系统

#### 关键属性

```cpp
// 输入组件
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Input")
TObjectPtr<UAuraInputComponent> AuraInputComponent;

// 鼠标光标
UPROPERTY(EditDefaultsOnly, Category = "Input")
TObjectPtr<UMouseCursorTrace> MouseCursorTrace;
```

#### 关键方法

**输入处理**:
```cpp
void AAuraPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    
    // 创建自定义输入组件
    AuraInputComponent = NewObject<UAuraInputComponent>(this);
    AuraInputComponent->BindAction(AuraInputConfig, this);
}
```

**鼠标交互**:
```cpp
void AAuraPlayerController::AcknowledgePossession(APawn* P)
{
    Super::AcknowledgePossession(P);
    
    // 初始化鼠标光标追踪
    if (IsLocalController())
    {
        MouseCursorTrace = NewObject<UMouseCursorTrace>(this);
        MouseCursorTrace->Initialize(this);
    }
}
```

---

## 玩家初始化流程

### 1. 玩家生成

```
GameMode::SpawnDefaultPawnFor
    ↓
AAuraCharacter::BeginPlay
    ↓
InitAbilityActorInfo
    ↓
从 PlayerState 获取 ASC
    ↓
初始化 UI
```

### 2. GAS 初始化

```cpp
void AAuraCharacter::InitAbilityActorInfo()
{
    // 1. 获取 PlayerState
    AAuraPlayerState* AuraPlayerState = GetPlayerState<AAuraPlayerState>();
    
    // 2. 获取 ASC
    AbilitySystemComponent = AuraPlayerState->GetAbilitySystemComponent();
    AbilitySystemComponent->InitAbilityActorInfo(AuraPlayerState, this);
    
    // 3. 获取 AttributeSet
    AttributeSet = AuraPlayerState->GetAttributeSet();
    
    // 4. 初始化 UI
    if (AAuraPlayerController* AuraPlayerController = 
        Cast<AAuraPlayerController>(GetController()))
    {
        if (AAuraHUD* AuraHUD = AuraPlayerController->GetHUD<AAuraHUD>())
        {
            AuraHUD->InitOverlay(AuraPlayerController, AuraPlayerState, 
                AbilitySystemComponent, AttributeSet);
        }
    }
    
    // 5. 初始化属性和能力
    InitializeDefaultAttributes();
    AddCharacterAbilities();
}
```

---

## 经验值和升级系统

### LevelUpInfo 数据资产

存储等级信息的数据资产：

```cpp
USTRUCT(BlueprintType)
struct FLevelUpInfo
{
    GENERATED_BODY()
    
    UPROPERTY(EditDefaultsOnly)
    int32 LevelUpRequirement = 0;  // 升级所需经验值
    
    UPROPERTY(EditDefaultsOnly)
    int32 AttributePointReward = 0;  // 属性点奖励
    
    UPROPERTY(EditDefaultsOnly)
    int32 SpellPointReward = 0;  // 法术点奖励
};
```

### 升级流程

1. **获得经验值**
   ```cpp
   PlayerState->AddToXP(XPReward);
   ```

2. **检查升级**
   ```cpp
   const int32 NewLevel = LevelUpInfo->FindLevelForXP(XP);
   if (NewLevel > Level)
   {
       // 升级
   }
   ```

3. **升级奖励**
   - 增加属性点
   - 增加法术点
   - 广播升级委托

---

## 属性点和法术点系统

### 属性点

用于升级角色属性（Strength, Intelligence, Resilience, Vigor）。

**使用流程**:
1. 玩家升级获得属性点
2. 在属性菜单中分配
3. 通过 `UAuraAbilitySystemComponent::UpgradeAttribute` 升级

### 法术点

用于解锁和升级技能。

**使用流程**:
1. 玩家升级获得法术点
2. 在法术菜单中使用
3. 通过 `UAuraAbilitySystemComponent::SpendSpellPoint` 使用

---

## 网络复制

### 复制属性

```cpp
void AAuraPlayerState::GetLifetimeReplicatedProps(
    TArray<FLifetimeProperty>& OutLifetimeProps
) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    // 复制玩家进度
    DOREPLIFETIME(AAuraPlayerState, Level);
    DOREPLIFETIME(AAuraPlayerState, XP);
    DOREPLIFETIME(AAuraPlayerState, AttributePoints);
    DOREPLIFETIME(AAuraPlayerState, SpellPoints);
}
```

### 复制通知

```cpp
void AAuraPlayerState::OnRep_Level(int32 OldLevel)
{
    // 通知 UI 更新
    OnLevelChangedDelegate.Broadcast(Level, Level > OldLevel);
}

void AAuraPlayerState::OnRep_XP(int32 OldXP)
{
    // 通知 UI 更新
    OnXPChangedDelegate.Broadcast(XP);
}
```

---

## UI 集成

### HUD 初始化

```cpp
void AAuraHUD::InitOverlay(
    AAuraPlayerController* PC,
    AAuraPlayerState* PS,
    UAbilitySystemComponent* ASC,
    UAttributeSet* AS
)
{
    // 创建覆盖层 Widget
    // 初始化 Widget Controller
    // 绑定委托
}
```

### Widget Controller

玩家状态通过 Widget Controller 与 UI 通信：

```cpp
// OverlayWidgetController
void UOverlayWidgetController::BindCallbacksToDependencies()
{
    // 绑定玩家状态委托
    AuraPlayerState->OnXPChangedDelegate.AddLambda(
        [this](int32 NewXP) { OnXPChanged.Broadcast(NewXP); }
    );
    
    AuraPlayerState->OnLevelChangedDelegate.AddLambda(
        [this](int32 NewLevel, bool bLevelUp) 
        { OnLevelChanged.Broadcast(NewLevel, bLevelUp); }
    );
}
```

---

## 存档系统集成

### 保存玩家数据

```cpp
void ULoadScreenSaveGame::SavePlayerState(AAuraPlayerState* PS)
{
    PlayerLevel = PS->GetPlayerLevel();
    XP = PS->GetXP();
    AttributePoints = PS->GetAttributePoints();
    SpellPoints = PS->GetSpellPoints();
}
```

### 加载玩家数据

```cpp
void AAuraGameModeBase::LoadPlayerState(AAuraPlayerState* PS, 
    const FPlayerSaveData& PlayerData)
{
    PS->SetLevel(PlayerData.PlayerLevel);
    PS->SetXP(PlayerData.XP);
    PS->SetAttributePoints(PlayerData.AttributePoints);
    PS->SetSpellPoints(PlayerData.SpellPoints);
}
```

---

## 相关文档

- [角色系统](./Character_System.md) - 角色实现
- [UI 系统](./UI_System.md) - UI 集成
- [输入系统](./Input_System.md) - 输入处理
- [Gameplay 框架](../Gameplay/Gameplay_Framework.md) - 存档系统

---

## 总结

玩家系统提供了：

1. ✅ **持久化数据** - 等级、经验值、属性点、法术点
2. ✅ **GAS 集成** - 完整的技能和属性系统
3. ✅ **升级系统** - 经验值和等级管理
4. ✅ **UI 集成** - 通过 Widget Controller 与 UI 通信
5. ✅ **网络支持** - 完整的复制系统
6. ✅ **存档支持** - 保存和加载玩家进度

通过这个系统，可以完整地管理玩家的游戏进度和能力发展。

