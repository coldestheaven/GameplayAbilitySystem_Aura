# 敌人头顶 HUD 系统

## 📖 概述

为 Aura 项目添加了完整的敌人头顶 HUD 系统，用于显示敌人的生命值、名称、等级等信息。系统采用 MVVM 架构，提供了清晰的职责分离和易于维护的代码结构。

![Enemy HUD System](https://via.placeholder.com/800x400?text=Enemy+HUD+System)

---

## ✨ 特性

- ✅ **MVVM 架构** - 清晰的数据与视图分离
- ✅ **自动数据绑定** - 属性变化自动更新 UI
- ✅ **蓝图友好** - 完全支持蓝图扩展和自定义
- ✅ **高性能** - 使用委托系统，按需更新
- ✅ **易于扩展** - 可轻松添加新的显示元素
- ✅ **多语言支持** - 支持本地化文本
- ✅ **动画系统** - 支持淡入、闪烁、脉动等动画效果

---

## 🏗️ 架构

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

## 📦 新增文件

### C++ 类

| 文件 | 描述 |
|------|------|
| `EnemyHealthBarWidgetController.h/cpp` | Widget Controller，管理数据和委托 |
| `EnemyHealthBarWidget.h/cpp` | Widget 基类，提供蓝图接口 |
| `AuraEnemy.h/cpp` (修改) | 添加了头顶 HUD 支持 |

### 文档

| 文件 | 描述 |
|------|------|
| `Enemy_HUD_System.md` | 完整的系统文档 |
| `Enemy_HUD_QuickRef.md` | 快速参考指南 |
| `Enemy_HUD_Blueprint_Setup.md` | 蓝图配置详解 |
| `README_Enemy_HUD.md` | 本文件 |

---

## 🚀 快速开始

### 1. 创建 Widget 蓝图

```
内容浏览器 → 右键 → 用户界面 → Widget 蓝图
命名: WBP_EnemyHealthBar
父类: EnemyHealthBarWidget
```

### 2. 添加 UI 组件

必须添加：
- **Progress Bar** (命名为 `HealthBar`)

可选添加：
- **Text Block** (命名为 `EnemyNameText`)
- **Text Block** (命名为 `LevelText`)
- **Text Block** (命名为 `HealthText`)

### 3. 绑定事件

在 Widget 蓝图的事件图表中：

```blueprint
Event WidgetControllerSet
  → Get Enemy Health Bar Controller
  → Bind Event to OnHealthPercentChanged
      → HealthBar → Set Percent
```

### 4. 配置敌人

在敌人蓝图中：
- 选择 `HealthBar` 组件
- 设置 `Widget Class` 为 `WBP_EnemyHealthBar`
- 设置 `Draw Size` 为 `200 x 50`
- 设置 `Space` 为 `Screen`

### 5. 测试

运行游戏，攻击敌人，观察生命值条是否实时更新。

---

## 📚 文档

### 完整文档
- [系统详细文档](./Enemy_HUD_System.md) - 完整的架构说明、使用指南和最佳实践
- [快速参考](./Enemy_HUD_QuickRef.md) - 快速查阅常用配置和节点
- [蓝图配置](./Enemy_HUD_Blueprint_Setup.md) - 详细的蓝图设置步骤

### 核心概念

#### Widget Controller
负责监听 GAS 属性变化，计算数据，并通过委托广播给 Widget。

```cpp
// 主要委托
OnHealthPercentChanged(float HealthPercent)
OnHealthChanged(float NewHealth)
OnMaxHealthChanged(float NewMaxHealth)
OnEnemyLevelChanged(int32 Level)
```

#### Widget
负责显示 UI 元素，响应 Controller 的委托事件。

```cpp
// 可绑定组件
HealthBar (Progress Bar) - 必须
EnemyNameText (Text Block) - 可选
LevelText (Text Block) - 可选
HealthText (Text Block) - 可选
```

---

## 🎨 自定义

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
```

#### 3. 在蓝图中绑定

```blueprint
Bind Event to OnEnemyTypeChanged
  → EnemyTypeText → Set Text
```

### 自定义样式

在 Widget 蓝图中修改：
- **颜色**: 修改 Progress Bar 的 Fill Color
- **字体**: 修改 Text Block 的 Font 属性
- **大小**: 修改 Size Box 的 Width/Height Override
- **动画**: 添加 Animation 轨道

---

## ⚡ 性能优化

### 限制可见距离

```cpp
// 在敌人 Tick 中
void AAuraEnemy::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    float Distance = GetDistanceToPlayer();
    HealthBar->SetVisibility(Distance < 2000.f);
}
```

### 使用对象池

```cpp
// 复用 Widget 实例
UUserWidget* PooledWidget = WidgetPool->GetWidget(HealthBarWidgetClass);
HealthBar->SetWidget(PooledWidget);
```

### 降低更新频率

```cpp
// 只在变化超过阈值时更新
if (FMath::Abs(NewHealth - LastHealth) > 1.0f)
{
    OnHealthChanged.Broadcast(NewHealth);
}
```

---

## 🐛 故障排除

### Widget 不显示

**检查清单**:
- ✅ Widget Class 是否设置
- ✅ Draw Size 是否合理（推荐 200x50）
- ✅ Space 设置为 Screen
- ✅ HealthBar 组件是否正确命名

### 生命值不更新

**检查清单**:
- ✅ 是否绑定了委托
- ✅ HealthBar 组件名称是否为 "HealthBar"
- ✅ AttributeSet 是否正确初始化
- ✅ AbilitySystemComponent 是否有效

### 性能问题

**优化建议**:
- 限制可见距离（推荐 2000 单位）
- 使用对象池复用 Widget
- 降低更新频率
- 禁用不必要的 Tick

---

## 📊 API 参考

### UEnemyHealthBarWidgetController

#### 委托

```cpp
// 生命值相关
FOnHealthChangedSignature OnHealthChanged;
FOnMaxHealthChangedSignature OnMaxHealthChanged;
FOnHealthPercentChangedSignature OnHealthPercentChanged;

// 敌人信息
FOnEnemyNameChangedSignature OnEnemyNameChanged;
FOnEnemyLevelChangedSignature OnEnemyLevelChanged;
```

#### 方法

```cpp
// 设置敌人引用
void SetEnemy(AAuraEnemy* InEnemy);

// 广播初始值
virtual void BroadcastInitialValues() override;

// 绑定回调
virtual void BindCallbacksToDependencies() override;
```

### UEnemyHealthBarWidget

#### 组件

```cpp
// 必须绑定
TObjectPtr<UProgressBar> HealthBar;

// 可选绑定
TObjectPtr<UTextBlock> HealthText;
TObjectPtr<UTextBlock> EnemyNameText;
TObjectPtr<UTextBlock> LevelText;
```

#### 蓝图事件

```cpp
// 可在蓝图中重写
void OnHealthPercentChanged(float HealthPercent);
void OnHealthValueChanged(float Health, float MaxHealth);
void OnEnemyNameChanged(const FText& Name);
void OnEnemyLevelChanged(int32 Level);
```

### AAuraEnemy

#### 新增属性

```cpp
// Widget Controller
TObjectPtr<UEnemyHealthBarWidgetController> HealthBarWidgetController;
TSubclassOf<UEnemyHealthBarWidgetController> HealthBarWidgetControllerClass;

// 敌人信息
FText EnemyName;
```

#### 新增方法

```cpp
// 初始化头顶 HUD
void InitializeHealthBarWidget();
```

---

## 🎯 使用示例

### 基础用法

```cpp
// 在敌人类中
void AAuraEnemy::BeginPlay()
{
    Super::BeginPlay();
    
    // 自动初始化头顶 HUD
    InitializeHealthBarWidget();
}
```

### 蓝图用法

```blueprint
// 在 Widget 蓝图中
Event WidgetControllerSet
    ↓
Get Enemy Health Bar Controller
    ↓
Bind Event to OnHealthPercentChanged
    ↓
    [Event] OnHealthPercentChanged
        ↓
        Branch (HealthPercent < 0.3)
            True → Set Fill Color (Red)
            False → Set Fill Color (Green)
        ↓
        HealthBar → Set Percent
```

---

## 🔄 版本历史

### v1.0.0 (2026-01-25)
- ✅ 初始版本发布
- ✅ 实现 MVVM 架构
- ✅ 支持生命值、名称、等级显示
- ✅ 完整的文档和示例

---

## 📝 待办事项

- [ ] 添加伤害数字飘字效果
- [ ] 支持 Buff/Debuff 图标显示
- [ ] 添加更多动画效果
- [ ] 优化性能（对象池）
- [ ] 添加单元测试

---

## 🤝 贡献

欢迎提交 Issue 和 Pull Request！

---

## 📄 许可证

Copyright Druid Mechanics

---

## 📞 联系方式

如有问题，请查阅文档或提交 Issue。

---

## 🙏 致谢

感谢 Aura 项目团队提供的优秀架构基础！
