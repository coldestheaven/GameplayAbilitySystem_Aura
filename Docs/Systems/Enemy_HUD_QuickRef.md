# 敌人头顶 HUD 快速参考

## 🚀 快速开始（5 分钟）

### 1. 创建 Widget 蓝图

```
内容浏览器 → 右键 → 用户界面 → Widget 蓝图
命名: WBP_EnemyHealthBar
父类: EnemyHealthBarWidget
```

### 2. 添加 UI 组件

```
必须组件:
  ✅ Progress Bar (命名: HealthBar)

可选组件:
  ⭕ Text Block (命名: EnemyNameText)
  ⭕ Text Block (命名: LevelText)
  ⭕ Text Block (命名: HealthText)
```

### 3. 绑定事件（蓝图）

```blueprint
Event WidgetControllerSet
  → Get Enemy Health Bar Controller
  → Bind Event to OnHealthPercentChanged
      → HealthBar → Set Percent
```

### 4. 配置敌人

```
敌人蓝图 → HealthBar 组件:
  - Widget Class: WBP_EnemyHealthBar
  - Draw Size: 200 x 50
  - Space: Screen
```

---

## 📋 核心类

| 类名 | 职责 | 位置 |
|------|------|------|
| `UEnemyHealthBarWidgetController` | 数据管理和委托 | `UI/WidgetController/` |
| `UEnemyHealthBarWidget` | UI 显示 | `UI/Widget/` |
| `AAuraEnemy` | 敌人角色 | `Character/` |

---

## 🎯 主要委托

```cpp
// 在 Widget 蓝图中可用的事件
OnHealthPercentChanged(float HealthPercent)
OnHealthChanged(float NewHealth)
OnMaxHealthChanged(float NewMaxHealth)
OnEnemyLevelChanged(int32 Level)
OnEnemyNameChanged(FText Name)
```

---

## 🔧 常用蓝图节点

### 获取 Controller
```blueprint
Get Enemy Health Bar Controller
  → Returns: EnemyHealthBarWidgetController
```

### 绑定生命值变化
```blueprint
Bind Event to OnHealthPercentChanged
  → [Event] Update Health Bar
      → HealthBar → Set Percent
```

### 绑定等级变化
```blueprint
Bind Event to OnEnemyLevelChanged
  → [Event] Update Level
      → Format Text: "Lv.{0}"
      → LevelText → Set Text
```

---

## 🎨 推荐设置

### Widget Component
```
Draw Size: 200 x 50
Pivot: (0.5, 0.5)
Space: Screen
Widget Class: WBP_EnemyHealthBar
```

### Progress Bar
```
Fill Color: 
  - Green (>70%)
  - Yellow (30-70%)
  - Red (<30%)
Percent: 1.0 (初始)
```

### Text Block
```
Font Size: 14
Justification: Center
Color: White
Shadow: Enabled
```

---

## ⚡ 性能优化

### 限制可见距离
```cpp
// 在敌人 Tick 中
float Distance = GetDistanceToPlayer();
HealthBar->SetVisibility(Distance < 2000.f);
```

### 降低更新频率
```cpp
// 只在变化超过阈值时更新
if (FMath::Abs(NewHealth - LastHealth) > 1.0f)
{
    UpdateHealthBar(NewHealth);
}
```

---

## 🐛 常见问题

### Widget 不显示？
- ✅ 检查 Widget Class 是否设置
- ✅ 检查 Draw Size 是否合理
- ✅ 检查 Space 设置为 Screen

### 生命值不更新？
- ✅ 检查是否绑定了委托
- ✅ 检查 HealthBar 组件名称是否正确
- ✅ 检查 AttributeSet 是否初始化

### 性能问题？
- ✅ 限制可见距离
- ✅ 使用对象池
- ✅ 降低更新频率

---

## 📚 完整文档

详细文档请参考: [Enemy_HUD_System.md](./Enemy_HUD_System.md)
