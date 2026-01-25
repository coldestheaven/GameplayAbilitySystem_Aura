# 敌人头顶 HUD 系统使用指南

## 📋 目录

1. [系统概述](#系统概述)
2. [架构设计](#架构设计)
3. [核心组件](#核心组件)
4. [使用步骤](#使用步骤)
5. [蓝图设置](#蓝图设置)
6. [自定义扩展](#自定义扩展)
7. [最佳实践](#最佳实践)

---

## 系统概述

敌人头顶 HUD 系统提供了一个完整的解决方案，用于在敌人角色头顶显示生命值条、名称、等级等信息。系统采用 MVVM 架构，实现了数据与视图的完全分离。

### 主要特性

- ✅ **MVVM 架构**：清晰的职责分离
- ✅ **自动数据绑定**：属性变化自动更新 UI
- ✅ **蓝图友好**：支持蓝图扩展和自定义
- ✅ **性能优化**：使用委托系统，按需更新
- ✅ **易于扩展**：可轻松添加新的显示元素

---

## 架构设计

```
┌─────────────────────────────────────┐
│      Model (数据层)                  │
│  AAuraEnemy + AttributeSet          │
│  - Health, MaxHealth                │
│  - Level, Name                      │
└──────────────┬──────────────────────┘
               ↓
┌─────────────────────────────────────┐
│   ViewModel (逻辑层)                 │
│  UEnemyHealthBarWidgetController    │
│  - 监听属性变化                      │
│  - 数据转换和计算                    │
│  - 委托广播                          │
└──────────────┬──────────────────────┘
               ↓
┌─────────────────────────────────────┐
│      View (视图层)                   │
│  UEnemyHealthBarWidget              │
│  - ProgressBar (生命值条)            │
│  - TextBlock (名称、等级、数值)      │
└─────────────────────────────────────┘
```

---

## 核心组件

### 1. UEnemyHealthBarWidgetController

**位置**: `Source/Aura/Public/UI/WidgetController/EnemyHealthBarWidgetController.h`

**职责**:
- 监听 GAS 属性变化
- 计算生命值百分比
- 通过委托广播数据更新

**主要委托**:
```cpp
// 生命值变化
FOnHealthChangedSignature OnHealthChanged;
FOnMaxHealthChangedSignature OnMaxHealthChanged;
FOnHealthPercentChangedSignature OnHealthPercentChanged;

// 敌人信息
FOnEnemyNameChangedSignature OnEnemyNameChanged;
FOnEnemyLevelChangedSignature OnEnemyLevelChanged;
```

### 2. UEnemyHealthBarWidget

**位置**: `Source/Aura/Public/UI/Widget/EnemyHealthBarWidget.h`

**职责**:
- 显示 UI 元素
- 响应数据变化
- 提供蓝图事件接口

**可绑定组件**:
```cpp
UPROPERTY(meta = (BindWidget))
TObjectPtr<UProgressBar> HealthBar;  // 必须绑定

UPROPERTY(meta = (BindWidgetOptional))
TObjectPtr<UTextBlock> HealthText;      // 可选
TObjectPtr<UTextBlock> EnemyNameText;   // 可选
TObjectPtr<UTextBlock> LevelText;       // 可选
```

### 3. AAuraEnemy

**位置**: `Source/Aura/Public/Character/AuraEnemy.h`

**新增属性**:
```cpp
// Widget Component
UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
TObjectPtr<UWidgetComponent> HealthBar;

// Widget Controller
UPROPERTY()
TObjectPtr<UEnemyHealthBarWidgetController> HealthBarWidgetController;

UPROPERTY(EditAnywhere, Category = "UI")
TSubclassOf<UEnemyHealthBarWidgetController> HealthBarWidgetControllerClass;

// 敌人信息
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Info")
FText EnemyName;
```

---

## 使用步骤

### 步骤 1: 创建 Widget 蓝图

1. 在内容浏览器中，右键 → **用户界面** → **Widget 蓝图**
2. 命名为 `WBP_EnemyHealthBar`
3. 打开蓝图，设置父类为 `EnemyHealthBarWidget`

### 步骤 2: 设计 UI 布局

在 Widget 设计器中添加以下组件：

```
Canvas Panel
└── Vertical Box
    ├── Text Block (名称) - 命名为 "EnemyNameText"
    ├── Horizontal Box
    │   ├── Text Block (等级) - 命名为 "LevelText"
    │   └── Spacer
    ├── Progress Bar (生命值条) - 命名为 "HealthBar" ⚠️ 必须
    └── Text Block (生命值数值) - 命名为 "HealthText"
```

**重要**: `HealthBar` 必须命名为 "HealthBar"，其他组件可选。

### 步骤 3: 绑定委托（蓝图）

在 `WBP_EnemyHealthBar` 的事件图表中：

```blueprint
Event WidgetControllerSet
    ↓
Get Enemy Health Bar Controller
    ↓
Bind Event to OnHealthPercentChanged
    ↓
    [Event] OnHealthPercentChanged
        ↓
        Set Percent (HealthBar) ← HealthPercent
```

**完整绑定示例**:

```blueprint
// 1. 生命值百分比变化
Event WidgetControllerSet
    → Get Enemy Health Bar Controller
    → Bind Event to OnHealthPercentChanged
        → [Custom Event] Update Health Bar
            → HealthBar → Set Percent

// 2. 生命值数值变化
Event WidgetControllerSet
    → Get Enemy Health Bar Controller
    → Bind Event to OnHealthChanged
        → [Custom Event] Update Health Text
            → Format Text: "{0} / {1}"
            → HealthText → Set Text

// 3. 等级变化
Event WidgetControllerSet
    → Get Enemy Health Bar Controller
    → Bind Event to OnEnemyLevelChanged
        → [Custom Event] Update Level
            → Format Text: "Lv.{0}"
            → LevelText → Set Text
```

### 步骤 4: 配置敌人蓝图

1. 打开敌人蓝图（如 `BP_Goblin`）
2. 选择 **HealthBar** 组件
3. 在细节面板中设置：
   - **Widget Class**: `WBP_EnemyHealthBar`
   - **Draw Size**: X=200, Y=50（根据需要调整）
   - **Pivot**: X=0.5, Y=0.5（居中）
   - **Space**: Screen（始终面向相机）

4. 设置敌人信息：
   - **Enemy Name**: "哥布林战士"
   - **Level**: 5

### 步骤 5: 测试

1. 将敌人放入关卡
2. 运行游戏
3. 观察头顶 HUD 是否正确显示
4. 攻击敌人，观察生命值条是否实时更新

---

## 蓝图设置

### Widget 蓝图完整示例

#### 事件图表

```blueprint
┌─────────────────────────────────────────┐
│ Event WidgetControllerSet               │
└────────────┬────────────────────────────┘
             ↓
┌─────────────────────────────────────────┐
│ Get Enemy Health Bar Controller         │
└────────────┬────────────────────────────┘
             ↓
┌─────────────────────────────────────────┐
│ Bind Event to OnHealthPercentChanged    │
│   → Update Health Bar                   │
│       → HealthBar.SetPercent            │
└─────────────────────────────────────────┘
             ↓
┌─────────────────────────────────────────┐
│ Bind Event to OnHealthChanged           │
│   → Update Health Text                  │
│       → Format: "{0} / {1}"             │
│       → HealthText.SetText              │
└─────────────────────────────────────────┘
             ↓
┌─────────────────────────────────────────┐
│ Bind Event to OnMaxHealthChanged        │
│   → Update Health Text                  │
└─────────────────────────────────────────┘
             ↓
┌─────────────────────────────────────────┐
│ Bind Event to OnEnemyLevelChanged       │
│   → Update Level Text                   │
│       → Format: "Lv.{0}"                │
│       → LevelText.SetText               │
└─────────────────────────────────────────┘
```

### C++ 代码示例（可选）

如果你想在 C++ 中处理 UI 更新：

```cpp
// 在 Widget 类中重写事件
void UMyEnemyHealthBarWidget::WidgetControllerSet()
{
    Super::WidgetControllerSet();
    
    UEnemyHealthBarWidgetController* Controller = GetEnemyHealthBarController();
    if (!Controller) return;
    
    // 绑定生命值百分比变化
    Controller->OnHealthPercentChanged.AddDynamic(
        this, 
        &UMyEnemyHealthBarWidget::OnHealthPercentChanged
    );
    
    // 绑定等级变化
    Controller->OnEnemyLevelChanged.AddDynamic(
        this, 
        &UMyEnemyHealthBarWidget::OnEnemyLevelChanged
    );
}
```

---

## 自定义扩展

### 添加新的显示元素

#### 1. 在 Controller 中添加委托

```cpp
// EnemyHealthBarWidgetController.h
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEnemyTypeChangedSignature, FText, EnemyType);

UPROPERTY(BlueprintAssignable, Category = "Enemy Info")
FOnEnemyTypeChangedSignature OnEnemyTypeChanged;
```

#### 2. 在 Widget 中添加组件

```cpp
// EnemyHealthBarWidget.h
UPROPERTY(meta = (BindWidgetOptional))
TObjectPtr<UTextBlock> EnemyTypeText;

UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Enemy HUD")
void OnEnemyTypeChanged(const FText& Type);
```

#### 3. 在敌人类中添加属性

```cpp
// AuraEnemy.h
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Info")
FText EnemyType;
```

### 添加动画效果

在 Widget 蓝图中：

```blueprint
// 生命值降低时的闪烁效果
Event OnHealthPercentChanged
    ↓
    Branch (HealthPercent < 0.3)
        True → Play Animation (FlashRed)
        False → Stop Animation (FlashRed)
```

### 添加距离淡出

```cpp
// 在 Widget 的 Tick 中
void UEnemyHealthBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    
    // 计算与玩家的距离
    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;
    
    APawn* PlayerPawn = PC->GetPawn();
    AAuraEnemy* Enemy = GetEnemyHealthBarController()->GetEnemy();
    
    if (PlayerPawn && Enemy)
    {
        float Distance = FVector::Dist(PlayerPawn->GetActorLocation(), Enemy->GetActorLocation());
        
        // 根据距离调整透明度
        float Alpha = FMath::Clamp(1.0f - (Distance - 1000.f) / 1000.f, 0.f, 1.f);
        SetRenderOpacity(Alpha);
    }
}
```

---

## 最佳实践

### 1. 性能优化

#### 使用对象池

```cpp
// 在敌人生成时复用 Widget
void AAuraEnemy::InitializeHealthBarWidget()
{
    // 从对象池获取 Widget
    UUserWidget* PooledWidget = WidgetPool->GetWidget(HealthBarWidgetClass);
    HealthBar->SetWidget(PooledWidget);
    
    // ... 初始化逻辑
}
```

#### 限制更新频率

```cpp
// 在 Controller 中
void UEnemyHealthBarWidgetController::HealthChanged(const FOnAttributeChangeData& Data) const
{
    // 只在变化超过阈值时更新
    if (FMath::Abs(Data.NewValue - LastBroadcastHealth) > 1.0f)
    {
        OnHealthChanged.Broadcast(Data.NewValue);
        LastBroadcastHealth = Data.NewValue;
    }
}
```

### 2. UI 设计建议

- **尺寸**: 推荐 200x50 像素
- **字体**: 使用清晰易读的字体，大小 12-16
- **颜色**: 
  - 生命值高 (>70%): 绿色
  - 生命值中 (30-70%): 黄色
  - 生命值低 (<30%): 红色
- **位置**: 在角色头顶上方 50-100 单位

### 3. 可见性控制

```cpp
// 只在玩家靠近时显示
void AAuraEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (APawn* PlayerPawn = PC->GetPawn())
        {
            float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());
            HealthBar->SetVisibility(Distance < 2000.f);
        }
    }
}
```

### 4. 多语言支持

```cpp
// 使用本地化文本
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy Info", meta = (MultiLine = true))
FText EnemyName = NSLOCTEXT("Enemy", "GoblinWarrior", "哥布林战士");
```

---

## 故障排除

### 问题 1: Widget 不显示

**检查清单**:
- ✅ HealthBar 组件是否正确附加到 RootComponent
- ✅ Widget Class 是否设置
- ✅ Draw Size 是否合理
- ✅ Space 设置为 Screen

### 问题 2: 生命值不更新

**检查清单**:
- ✅ 是否调用了 `InitializeHealthBarWidget()`
- ✅ 是否正确绑定了委托
- ✅ AttributeSet 是否正确初始化
- ✅ AbilitySystemComponent 是否有效

### 问题 3: 性能问题

**优化建议**:
- 使用对象池
- 限制可见距离
- 降低更新频率
- 使用 LOD 系统

---

## 总结

敌人头顶 HUD 系统提供了：

- ✅ **完整的 MVVM 架构**
- ✅ **自动数据绑定**
- ✅ **蓝图友好的接口**
- ✅ **高度可扩展**
- ✅ **性能优化**

通过这个系统，你可以轻松为游戏中的敌人添加专业的头顶信息显示。
