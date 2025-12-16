# UI 系统详细文档

## 目录

1. [系统概述](#系统概述)
2. [MVVM 架构](#mvvm-架构)
3. [Widget Controller 系统](#widget-controller-系统)
4. [Overlay Widget Controller](#overlay-widget-controller)
5. [Attribute Menu Widget Controller](#attribute-menu-widget-controller)
6. [Spell Menu Widget Controller](#spell-menu-widget-controller)
7. [数据流](#数据流)
8. [委托系统](#委托系统)
9. [配置指南](#配置指南)
10. [使用示例](#使用示例)

---

## 系统概述

UI 系统采用 MVVM (Model-View-ViewModel) 架构模式，将游戏逻辑（GAS）与 UI 显示分离，提供了清晰的职责划分和易于维护的代码结构。

### 核心组件

- **UAuraWidgetController**: Widget Controller 基类
- **UOverlayWidgetController**: 覆盖层 UI 控制器
- **UAttributeMenuWidgetController**: 属性菜单控制器
- **USpellMenuWidgetController**: 法术菜单控制器
- **AAuraHUD**: HUD 类，管理 Widget Controller

### 系统特点

- ✅ MVVM 架构模式
- ✅ 数据驱动 UI 更新
- ✅ 委托系统实现解耦
- ✅ 自动初始化和绑定
- ✅ 支持蓝图扩展

---

## MVVM 架构

### 架构层次

```
┌─────────────────────────────────┐
│      Model (数据层)              │
│  AttributeSet, ASC, PlayerState │
└──────────────┬──────────────────┘
               ↓
┌─────────────────────────────────┐
│   ViewModel (逻辑层)              │
│   WidgetController               │
│   - 数据转换                      │
│   - 业务逻辑                      │
│   - 委托广播                      │
└──────────────┬──────────────────┘
               ↓
┌─────────────────────────────────┐
│      View (视图层)               │
│   UMG Widgets                   │
│   - UI 显示                      │
│   - 用户交互                      │
└─────────────────────────────────┘
```

### 职责划分

#### Model (数据层)

- **UAuraAttributeSet**: 属性数据
- **UAuraAbilitySystemComponent**: 能力数据
- **AAuraPlayerState**: 玩家状态数据

#### ViewModel (逻辑层)

- **UAuraWidgetController**: 基础控制器
- **UOverlayWidgetController**: 覆盖层控制器
- **UAttributeMenuWidgetController**: 属性菜单控制器
- **USpellMenuWidgetController**: 法术菜单控制器

#### View (视图层)

- **UMG Widgets**: 各种 UI 组件
- **蓝图 Widget**: 在蓝图中创建的 UI

---

## Widget Controller 系统

### UAuraWidgetController

所有 Widget Controller 的基类。

#### 核心数据

```cpp
struct FWidgetControllerParams
{
    TObjectPtr<APlayerController> PlayerController;
    TObjectPtr<APlayerState> PlayerState;
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
    TObjectPtr<UAttributeSet> AttributeSet;
};
```

#### 初始化方法

```cpp
// 设置参数
void SetWidgetControllerParams(const FWidgetControllerParams& WCParams);

// 广播初始值
virtual void BroadcastInitialValues();

// 绑定回调
virtual void BindCallbacksToDependencies();
```

#### 辅助方法

```cpp
// 获取 Aura 特定对象
AAuraPlayerController* GetAuraPC();
AAuraPlayerState* GetAuraPS();
UAuraAbilitySystemComponent* GetAuraASC();
UAuraAttributeSet* GetAuraAS();
```

### 初始化流程

```
1. Widget 请求 WidgetController
   ↓
2. HUD 创建或返回 WidgetController
   ↓
3. 设置 WidgetControllerParams
   ↓
4. 调用 BroadcastInitialValues()
   ↓
5. 调用 BindCallbacksToDependencies()
   ↓
6. Widget 绑定到委托
```

---

## Overlay Widget Controller

### UOverlayWidgetController

覆盖层 UI 的控制器，管理 HUD 上的主要 UI 元素。

#### 委托

```cpp
// 属性变化委托
FOnAttributeChangedSignature OnHealthChanged;
FOnAttributeChangedSignature OnMaxHealthChanged;
FOnAttributeChangedSignature OnManaChanged;
FOnAttributeChangedSignature OnMaxManaChanged;

// 经验值委托
FOnAttributeChangedSignature OnXPPercentChangedDelegate;

// 等级变化委托
FOnLevelChangedSignature OnPlayerLevelChangedDelegate;

// 消息委托
FMessageWidgetRowSignature MessageWidgetRowDelegate;
```

#### 初始化

```cpp
void UOverlayWidgetController::BroadcastInitialValues()
{
    // 广播初始属性值
    OnHealthChanged.Broadcast(GetAuraAS()->GetHealth());
    OnMaxHealthChanged.Broadcast(GetAuraAS()->GetMaxHealth());
    OnManaChanged.Broadcast(GetAuraAS()->GetMana());
    OnMaxManaChanged.Broadcast(GetAuraAS()->GetMaxMana());
}
```

#### 绑定回调

```cpp
void UOverlayWidgetController::BindCallbacksToDependencies()
{
    // 绑定经验值变化
    GetAuraPS()->OnXPChangedDelegate.AddUObject(
        this, 
        &UOverlayWidgetController::OnXPChanged
    );
    
    // 绑定等级变化
    GetAuraPS()->OnLevelChangedDelegate.AddLambda(
        [this](int32 NewLevel, bool bLevelUp)
        {
            OnPlayerLevelChangedDelegate.Broadcast(NewLevel, bLevelUp);
        }
    );
    
    // 绑定属性变化
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
        GetAuraAS()->GetHealthAttribute()
    ).AddLambda(
        [this](const FOnAttributeChangeData& Data)
        {
            OnHealthChanged.Broadcast(Data.NewValue);
        }
    );
    
    // ... 其他属性绑定
    
    // 绑定能力装备
    GetAuraASC()->AbilityEquipped.AddUObject(
        this, 
        &UOverlayWidgetController::OnAbilityEquipped
    );
    
    // 绑定效果资产标签
    GetAuraASC()->EffectAssetTags.AddLambda(
        [this](const FGameplayTagContainer& AssetTags)
        {
            for (const FGameplayTag& Tag : AssetTags)
            {
                if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Message"))))
                {
                    const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(
                        MessageWidgetDataTable, 
                        Tag
                    );
                    MessageWidgetRowDelegate.Broadcast(*Row);
                }
            }
        }
    );
}
```

#### 经验值计算

```cpp
void UOverlayWidgetController::OnXPChanged(int32 NewXP)
{
    const ULevelUpInfo* LevelUpInfo = GetAuraPS()->LevelUpInfo;
    const int32 Level = LevelUpInfo->FindLevelForXP(NewXP);
    const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num();
    
    if (Level <= MaxLevel && Level > 0)
    {
        const int32 LevelUpRequirement = 
            LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
        const int32 PreviousLevelUpRequirement = 
            LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;
        
        const int32 DeltaLevelRequirement = 
            LevelUpRequirement - PreviousLevelUpRequirement;
        const int32 XPForThisLevel = NewXP - PreviousLevelUpRequirement;
        
        // 计算经验条百分比
        const float XPBarPercent = 
            static_cast<float>(XPForThisLevel) / 
            static_cast<float>(DeltaLevelRequirement);
        
        OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
    }
}
```

---

## Attribute Menu Widget Controller

### UAttributeMenuWidgetController

属性菜单的控制器，管理属性显示和升级。

#### 委托

```cpp
// 属性信息委托
FAttributeInfoSignature AttributeInfoDelegate;

// 属性点变化委托
FOnPlayerStatChangedSignature AttributePointsChangedDelegate;
```

#### 初始化

```cpp
void UAttributeMenuWidgetController::BroadcastInitialValues()
{
    UAuraAttributeSet* AS = CastChecked<UAuraAttributeSet>(AttributeSet);
    
    // 广播所有属性信息
    for (auto& Pair : AS->TagsToAttributes)
    {
        BroadcastAttributeInfo(Pair.Key, Pair.Value());
    }
    
    // 广播属性点
    AttributePointsChangedDelegate.Broadcast(
        GetAuraPS()->GetAttributePoints()
    );
}
```

#### 绑定回调

```cpp
void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
    // 绑定所有属性变化
    for (auto& Pair : GetAuraAS()->TagsToAttributes)
    {
        AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
            Pair.Value()
        ).AddLambda(
            [this, Pair](const FOnAttributeChangeData& Data)
            {
                BroadcastAttributeInfo(Pair.Key, Pair.Value());
            }
        );
    }
    
    // 绑定属性点变化
    GetAuraPS()->OnAttributePointsChangedDelegate.AddLambda(
        [this](int32 Points)
        {
            AttributePointsChangedDelegate.Broadcast(Points);
        }
    );
}
```

#### 属性升级

```cpp
void UAttributeMenuWidgetController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
    UAuraAbilitySystemComponent* AuraASC = 
        CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
    AuraASC->UpgradeAttribute(AttributeTag);
}
```

#### 广播属性信息

```cpp
void UAttributeMenuWidgetController::BroadcastAttributeInfo(
    const FGameplayTag& AttributeTag, 
    const FGameplayAttribute& Attribute
) const
{
    // 从 AttributeInfo 数据资产获取信息
    FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
    
    // 设置当前值
    Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
    
    // 广播
    AttributeInfoDelegate.Broadcast(Info);
}
```

---

## Spell Menu Widget Controller

### USpellMenuWidgetController

法术菜单的控制器，管理技能显示、升级和装备。

#### 委托

```cpp
// 法术点变化
FOnPlayerStatChangedSignature SpellPointsChanged;

// 技能球选中
FSpellGlobeSelectedSignature SpellGlobeSelectedDelegate;

// 等待装备选择
FWaitForEquipSelectionSignature WaitForEquipDelegate;
FWaitForEquipSelectionSignature StopWaitingForEquipDelegate;

// 技能球重新分配
FSpellGlobeReassignedSignature SpellGlobeReassignedDelegate;
```

#### 核心方法

##### 技能球选中

```cpp
void USpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
    // 获取技能状态
    FGameplayTag AbilityStatus;
    const FGameplayAbilitySpec* AbilitySpec = 
        GetAuraASC()->GetSpecFromAbilityTag(AbilityTag);
    
    if (AbilitySpec)
    {
        AbilityStatus = GetAuraASC()->GetStatusFromSpec(*AbilitySpec);
    }
    else
    {
        AbilityStatus = FAuraGameplayTags::Get().Abilities_Status_Locked;
    }
    
    // 更新选中技能
    SelectedAbility.Ability = AbilityTag;
    SelectedAbility.Status = AbilityStatus;
    
    // 判断按钮状态
    bool bEnableSpendPoints = false;
    bool bEnableEquip = false;
    ShouldEnableButtons(
        AbilityStatus, 
        GetAuraPS()->GetSpellPoints(), 
        bEnableSpendPoints, 
        bEnableEquip
    );
    
    // 获取技能描述
    FString Description;
    FString NextLevelDescription;
    GetAuraASC()->GetDescriptionsByAbilityTag(
        AbilityTag, 
        Description, 
        NextLevelDescription
    );
    
    // 广播
    SpellGlobeSelectedDelegate.Broadcast(
        bEnableSpendPoints, 
        bEnableEquip, 
        Description, 
        NextLevelDescription
    );
}
```

##### 消耗法术点

```cpp
void USpellMenuWidgetController::SpendPointButtonPressed()
{
    if (GetAuraASC())
    {
        GetAuraASC()->ServerSpendSpellPoint(SelectedAbility.Ability);
    }
}
```

##### 装备技能

```cpp
void USpellMenuWidgetController::EquipButtonPressed()
{
    const FGameplayTag AbilityType = 
        AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
    
    // 等待选择槽位
    WaitForEquipDelegate.Broadcast(AbilityType);
    bWaitingForEquipSelection = true;
    
    // 如果已装备，记录当前槽位
    const FGameplayTag SelectedStatus = 
        GetAuraASC()->GetStatusFromAbilityTag(SelectedAbility.Ability);
    if (SelectedStatus.MatchesTagExact(
        FAuraGameplayTags::Get().Abilities_Status_Equipped))
    {
        SelectedSlot = GetAuraASC()->GetSlotFromAbilityTag(SelectedAbility.Ability);
    }
}
```

##### 槽位选择

```cpp
void USpellMenuWidgetController::SpellRowGlobePressed(
    const FGameplayTag& SlotTag, 
    const FGameplayTag& AbilityType
)
{
    if (!bWaitingForEquipSelection) return;
    
    // 检查技能类型是否匹配
    const FGameplayTag& SelectedAbilityType = 
        AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
    if (!SelectedAbilityType.MatchesTagExact(AbilityType)) return;
    
    // 装备技能
    GetAuraASC()->ServerEquipAbility(SelectedAbility.Ability, SlotTag);
}
```

##### 按钮状态判断

```cpp
void USpellMenuWidgetController::ShouldEnableButtons(
    const FGameplayTag& AbilityStatus, 
    int32 SpellPoints, 
    bool& bShouldEnableSpellPointsButton, 
    bool& bShouldEnableEquipButton
)
{
    const FAuraGameplayTags GameplayTags = FAuraGameplayTags::Get();
    
    bShouldEnableSpellPointsButton = false;
    bShouldEnableEquipButton = false;
    
    // 已装备状态
    if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Equipped))
    {
        bShouldEnableEquipButton = true;
        if (SpellPoints > 0)
        {
            bShouldEnableSpellPointsButton = true;
        }
    }
    // 符合条件状态
    else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
    {
        if (SpellPoints > 0)
        {
            bShouldEnableSpellPointsButton = true;
        }
    }
    // 已解锁状态
    else if (AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
    {
        bShouldEnableEquipButton = true;
        if (SpellPoints > 0)
        {
            bShouldEnableSpellPointsButton = true;
        }
    }
}
```

---

## 数据流

### 初始化数据流

```
1. Widget 创建
   ↓
2. 请求 WidgetController (从 HUD)
   ↓
3. HUD 创建或返回 WidgetController
   ↓
4. 设置 WidgetControllerParams
   ↓
5. BroadcastInitialValues()
   ↓
6. Widget 接收初始值并显示
```

### 更新数据流

```
1. Model 数据变化 (AttributeSet, ASC, PlayerState)
   ↓
2. 触发回调 (通过委托或 Lambda)
   ↓
3. WidgetController 接收回调
   ↓
4. 处理数据（转换、计算等）
   ↓
5. 广播委托给 Widget
   ↓
6. Widget 更新显示
```

### 用户交互数据流

```
1. 用户在 Widget 中操作
   ↓
2. Widget 调用 WidgetController 方法
   ↓
3. WidgetController 处理逻辑
   ↓
4. 修改 Model 数据 (通过 ASC, PlayerState 等)
   ↓
5. Model 数据变化触发回调
   ↓
6. WidgetController 广播更新
   ↓
7. Widget 更新显示
```

---

## 委托系统

### 委托类型

#### 属性变化委托

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnAttributeChangedSignature, 
    float, 
    NewValue
);
```

**用途**: 通知属性值变化

#### 玩家状态变化委托

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FOnPlayerStatChangedSignature, 
    int32, 
    NewValue
);
```

**用途**: 通知玩家状态变化（经验、等级、点数等）

#### 能力信息委托

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FAbilityInfoSignature, 
    const FAuraAbilityInfo&, 
    Info
);
```

**用途**: 广播能力信息

#### 消息委托

```cpp
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FMessageWidgetRowSignature, 
    FUIWidgetRow, 
    Row
);
```

**用途**: 显示消息（例如：获得物品、升级等）

### 委托绑定

#### Lambda 绑定

```cpp
AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
    AttributeSet->GetHealthAttribute()
).AddLambda(
    [this](const FOnAttributeChangeData& Data)
    {
        OnHealthChanged.Broadcast(Data.NewValue);
    }
);
```

#### 对象方法绑定

```cpp
GetAuraPS()->OnXPChangedDelegate.AddUObject(
    this, 
    &UOverlayWidgetController::OnXPChanged
);
```

---

## 配置指南

### 创建 Widget Controller

#### 步骤 1: 创建 Widget Controller 类

```cpp
// MyWidgetController.h
UCLASS()
class AURA_API UMyWidgetController : public UAuraWidgetController
{
    GENERATED_BODY()
    
public:
    virtual void BroadcastInitialValues() override;
    virtual void BindCallbacksToDependencies() override;
    
    // 委托
    UPROPERTY(BlueprintAssignable)
    FOnAttributeChangedSignature OnMyAttributeChanged;
};
```

#### 步骤 2: 实现初始化和绑定

```cpp
// MyWidgetController.cpp
void UMyWidgetController::BroadcastInitialValues()
{
    // 广播初始值
    OnMyAttributeChanged.Broadcast(CurrentValue);
}

void UMyWidgetController::BindCallbacksToDependencies()
{
    // 绑定属性变化
    AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
        AttributeSet->GetMyAttributeAttribute()
    ).AddLambda(
        [this](const FOnAttributeChangeData& Data)
        {
            OnMyAttributeChanged.Broadcast(Data.NewValue);
        }
    );
}
```

#### 步骤 3: 在 HUD 中注册

```cpp
// AuraHUD.h
UPROPERTY()
TObjectPtr<UMyWidgetController> MyWidgetController;

UMyWidgetController* GetMyWidgetController(const FWidgetControllerParams& WCParams);
```

```cpp
// AuraHUD.cpp
UMyWidgetController* AAuraHUD::GetMyWidgetController(const FWidgetControllerParams& WCParams)
{
    if (MyWidgetController == nullptr)
    {
        MyWidgetController = NewObject<UMyWidgetController>(this, MyWidgetControllerClass);
        MyWidgetController->SetWidgetControllerParams(WCParams);
        MyWidgetController->BindCallbacksToDependencies();
    }
    return MyWidgetController;
}
```

---

## 使用示例

### 在 Widget 中使用

```cpp
// 在 Widget 蓝图中
void UMyWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    // 获取 WidgetController
    FWidgetControllerParams WCParams;
    AAuraHUD* AuraHUD = Cast<AAuraHUD>(GetOwningPlayer()->GetHUD());
    if (AuraHUD)
    {
        UOverlayWidgetController* Controller = 
            AuraHUD->GetOverlayWidgetController(WCParams);
        
        // 绑定委托
        Controller->OnHealthChanged.AddDynamic(
            this, 
            &UMyWidget::OnHealthChanged
        );
    }
}

void UMyWidget::OnHealthChanged(float NewHealth)
{
    // 更新 UI
    HealthBar->SetPercent(NewHealth / MaxHealth);
}
```

### 自定义数据转换

```cpp
// 在 WidgetController 中
void UMyWidgetController::BroadcastHealthPercent()
{
    float Health = GetAuraAS()->GetHealth();
    float MaxHealth = GetAuraAS()->GetMaxHealth();
    float HealthPercent = MaxHealth > 0 ? Health / MaxHealth : 0.f;
    
    OnHealthPercentChanged.Broadcast(HealthPercent);
}
```

---

## 最佳实践

### 1. MVVM 设计

- **职责分离**: Model、ViewModel、View 各司其职
- **数据单向流**: 数据从 Model → ViewModel → View
- **避免直接访问**: View 不直接访问 Model

### 2. 委托使用

- **使用委托**: 通过委托实现解耦
- **及时清理**: 在适当时机清理委托绑定
- **避免循环引用**: 使用 `AddUObject` 或 `AddWeakLambda`

### 3. 性能优化

- **按需更新**: 只更新变化的 UI 元素
- **批量更新**: 批量处理多个属性变化
- **缓存数据**: 缓存频繁访问的数据

---

## 总结

UI 系统提供了清晰的 MVVM 架构：

- ✅ **MVVM 模式**: 清晰的职责划分
- ✅ **数据驱动**: 自动响应数据变化
- ✅ **委托系统**: 实现解耦和通信
- ✅ **易于扩展**: 易于添加新的 Widget Controller
- ✅ **蓝图支持**: 支持蓝图扩展

通过这个系统，开发者可以轻松创建和维护复杂的游戏 UI。

