# 敌人头顶 HUD 蓝图配置示例

## 📋 目录

1. [Widget 蓝图配置](#widget-蓝图配置)
2. [敌人蓝图配置](#敌人蓝图配置)
3. [完整事件图表](#完整事件图表)
4. [样式配置](#样式配置)

---

## Widget 蓝图配置

### 文件信息
- **名称**: `WBP_EnemyHealthBar`
- **父类**: `EnemyHealthBarWidget`
- **路径**: `Content/UI/Enemy/`

### UI 层级结构

```
Canvas Panel
└── Size Box (固定大小)
    └── Overlay
        ├── Image (背景)
        │   └── Border (边框)
        └── Vertical Box (主容器)
            ├── Horizontal Box (顶部信息栏)
            │   ├── Text Block "EnemyNameText" (敌人名称)
            │   ├── Spacer
            │   └── Text Block "LevelText" (等级)
            ├── Spacer (间距)
            ├── Progress Bar "HealthBar" (生命值条) ⚠️ 必须
            └── Text Block "HealthText" (生命值数值)
```

### 组件详细配置

#### 1. Canvas Panel
```
属性:
  - Is Variable: False
```

#### 2. Size Box
```
属性:
  - Width Override: 200
  - Height Override: 60
  - Is Variable: False
```

#### 3. Image (背景)
```
属性:
  - Color: (R=0, G=0, B=0, A=0.7) 半透明黑色
  - Brush:
      - Draw As: Rounded Box
      - Rounding: (4, 4, 4, 4)
```

#### 4. Border (边框)
```
属性:
  - Brush Color: (R=0.2, G=0.2, B=0.2, A=1.0)
  - Padding: (2, 2, 2, 2)
```

#### 5. Vertical Box (主容器)
```
属性:
  - Padding: (8, 8, 8, 8)
```

#### 6. Text Block "EnemyNameText" ⭐
```
属性:
  - Is Variable: True ⚠️ 必须
  - Text: "Enemy Name"
  - Font:
      - Size: 14
      - Typeface: Bold
  - Color: (R=1, G=1, B=1, A=1) 白色
  - Justification: Left
  - Shadow:
      - Offset: (1, 1)
      - Color: (R=0, G=0, B=0, A=0.8)
```

#### 7. Text Block "LevelText" ⭐
```
属性:
  - Is Variable: True ⚠️ 必须
  - Text: "Lv.1"
  - Font:
      - Size: 12
      - Typeface: Regular
  - Color: (R=1, G=0.8, B=0, A=1) 金色
  - Justification: Right
  - Shadow:
      - Offset: (1, 1)
      - Color: (R=0, G=0, B=0, A=0.8)
```

#### 8. Progress Bar "HealthBar" ⭐⚠️
```
属性:
  - Is Variable: True ⚠️ 必须
  - Percent: 1.0
  - Fill Color and Opacity: (R=0, G=1, B=0, A=1) 绿色
  - Background Color: (R=0.2, G=0.2, B=0.2, A=1) 深灰色
  - Bar Fill Type: Left to Right
  - Size:
      - Fill: Horizontal
      - Fixed: 20 (Height)
```

#### 9. Text Block "HealthText" ⭐
```
属性:
  - Is Variable: True ⚠️ 必须
  - Text: "100 / 100"
  - Font:
      - Size: 12
      - Typeface: Regular
  - Color: (R=1, G=1, B=1, A=1) 白色
  - Justification: Center
  - Shadow:
      - Offset: (1, 1)
      - Color: (R=0, G=0, B=0, A=0.8)
```

---

## 敌人蓝图配置

### 文件信息
- **名称**: `BP_Goblin` (示例)
- **父类**: `AuraEnemy`
- **路径**: `Content/Blueprints/Character/Enemy/`

### HealthBar 组件配置

选择 **HealthBar** 组件，在细节面板中设置：

#### User Interface
```
Widget Class: WBP_EnemyHealthBar
Draw Size: (X=200, Y=60)
Pivot: (X=0.5, Y=0.5)
```

#### Rendering
```
Space: Screen
Blend Mode: Transparent
```

#### Transform
```
Location: (X=0, Y=0, Z=100) // 头顶上方 100 单位
Rotation: (X=0, Y=0, Z=0)
Scale: (X=1, Y=1, Z=1)
```

#### Visibility
```
Visible: True
Hidden in Game: False
```

### 敌人信息配置

在 **Details** 面板中：

#### Enemy Info
```
Enemy Name: "哥布林战士"
Level: 5
Character Class: Warrior
```

#### UI
```
Health Bar Widget Controller Class: EnemyHealthBarWidgetController
```

---

## 完整事件图表

### Event WidgetControllerSet

这是最重要的事件，在这里绑定所有委托。

```blueprint
┌─────────────────────────────────────────────────────────┐
│ Event WidgetControllerSet                               │
└────────────┬────────────────────────────────────────────┘
             ↓
┌─────────────────────────────────────────────────────────┐
│ Get Enemy Health Bar Controller                         │
│   Return Value → [Local Variable] Controller            │
└────────────┬────────────────────────────────────────────┘
             ↓
┌─────────────────────────────────────────────────────────┐
│ Branch (Is Valid: Controller)                           │
└────────────┬────────────────────────────────────────────┘
             ↓ True
┌─────────────────────────────────────────────────────────┐
│ Bind Event to OnHealthPercentChanged                    │
│   Event: OnHealthPercentChanged                         │
│   Object: Controller                                    │
└────────────┬────────────────────────────────────────────┘
             ↓
┌─────────────────────────────────────────────────────────┐
│ Bind Event to OnHealthChanged                           │
│   Event: OnHealthChanged                                │
│   Object: Controller                                    │
└────────────┬────────────────────────────────────────────┘
             ↓
┌─────────────────────────────────────────────────────────┐
│ Bind Event to OnMaxHealthChanged                        │
│   Event: OnMaxHealthChanged                             │
│   Object: Controller                                    │
└────────────┬────────────────────────────────────────────┘
             ↓
┌─────────────────────────────────────────────────────────┐
│ Bind Event to OnEnemyLevelChanged                       │
│   Event: OnEnemyLevelChanged                            │
│   Object: Controller                                    │
└─────────────────────────────────────────────────────────┘
```

### Custom Event: OnHealthPercentChanged

```blueprint
┌─────────────────────────────────────────────────────────┐
│ [Event] OnHealthPercentChanged                          │
│   Health Percent (Float)                                │
└────────────┬────────────────────────────────────────────┘
             ↓
┌─────────────────────────────────────────────────────────┐
│ HealthBar → Set Percent                                 │
│   In Percent: Health Percent                            │
└────────────┬────────────────────────────────────────────┘
             ↓
┌─────────────────────────────────────────────────────────┐
│ Branch (Health Percent < 0.3)                           │
└────────────┬────────────────────────────────────────────┘
             ↓ True
┌─────────────────────────────────────────────────────────┐
│ HealthBar → Set Fill Color and Opacity                  │
│   Color: (R=1, G=0, B=0, A=1) 红色                      │
└─────────────────────────────────────────────────────────┘
             ↓ False
┌─────────────────────────────────────────────────────────┐
│ Branch (Health Percent < 0.7)                           │
└────────────┬────────────────────────────────────────────┘
             ↓ True
┌─────────────────────────────────────────────────────────┐
│ HealthBar → Set Fill Color and Opacity                  │
│   Color: (R=1, G=1, B=0, A=1) 黄色                      │
└─────────────────────────────────────────────────────────┘
             ↓ False
┌─────────────────────────────────────────────────────────┐
│ HealthBar → Set Fill Color and Opacity                  │
│   Color: (R=0, G=1, B=0, A=1) 绿色                      │
└─────────────────────────────────────────────────────────┘
```

### Custom Event: OnHealthChanged

```blueprint
┌─────────────────────────────────────────────────────────┐
│ [Event] OnHealthChanged                                 │
│   New Health (Float)                                    │
└────────────┬────────────────────────────────────────────┘
             ↓
┌─────────────────────────────────────────────────────────┐
│ Get Enemy Health Bar Controller                         │
└────────────┬────────────────────────────────────────────┘
             ↓
┌─────────────────────────────────────────────────────────┐
│ Get Attribute Set (from Controller)                     │
└────────────┬────────────────────────────────────────────┘
             ↓
┌─────────────────────────────────────────────────────────┐
│ Cast to AuraAttributeSet                                │
└────────────┬────────────────────────────────────────────┘
             ↓
┌─────────────────────────────────────────────────────────┐
│ Get Max Health                                          │
└────────────┬────────────────────────────────────────────┘
             ↓
┌─────────────────────────────────────────────────────────┐
│ Format Text                                             │
│   Format: "{0} / {1}"                                   │
│   0: New Health (Round to Int)                          │
│   1: Max Health (Round to Int)                          │
└────────────┬────────────────────────────────────────────┘
             ↓
┌─────────────────────────────────────────────────────────┐
│ HealthText → Set Text                                   │
└─────────────────────────────────────────────────────────┘
```

### Custom Event: OnEnemyLevelChanged

```blueprint
┌─────────────────────────────────────────────────────────┐
│ [Event] OnEnemyLevelChanged                             │
│   Level (Integer)                                       │
└────────────┬────────────────────────────────────────────┘
             ↓
┌─────────────────────────────────────────────────────────┐
│ Format Text                                             │
│   Format: "Lv.{0}"                                      │
│   0: Level                                              │
└────────────┬────────────────────────────────────────────┘
             ↓
┌─────────────────────────────────────────────────────────┐
│ LevelText → Set Text                                    │
└─────────────────────────────────────────────────────────┘
```

---

## 样式配置

### 主题 1: 经典风格

```
背景: 半透明黑色 (0, 0, 0, 0.7)
边框: 深灰色 (0.2, 0.2, 0.2, 1.0)
生命值条:
  - 高: 绿色 (0, 1, 0, 1)
  - 中: 黄色 (1, 1, 0, 1)
  - 低: 红色 (1, 0, 0, 1)
文字: 白色 (1, 1, 1, 1)
等级: 金色 (1, 0.8, 0, 1)
```

### 主题 2: 现代风格

```
背景: 深蓝色 (0.1, 0.1, 0.3, 0.8)
边框: 亮蓝色 (0.3, 0.5, 1.0, 1.0)
生命值条:
  - 高: 青色 (0, 1, 1, 1)
  - 中: 橙色 (1, 0.5, 0, 1)
  - 低: 品红色 (1, 0, 0.5, 1)
文字: 白色 (1, 1, 1, 1)
等级: 紫色 (0.8, 0.3, 1.0, 1)
```

### 主题 3: 暗黑风格

```
背景: 纯黑色 (0, 0, 0, 0.9)
边框: 血红色 (0.8, 0, 0, 1.0)
生命值条:
  - 高: 暗绿色 (0, 0.6, 0, 1)
  - 中: 暗橙色 (0.8, 0.4, 0, 1)
  - 低: 深红色 (0.8, 0, 0, 1)
文字: 灰白色 (0.9, 0.9, 0.9, 1)
等级: 暗金色 (0.8, 0.6, 0, 1)
```

---

## 动画配置

### 淡入动画

```
名称: FadeIn
时长: 0.3 秒
轨道:
  - Render Opacity: 0 → 1
  - Scale: (0.8, 0.8) → (1, 1)
```

### 受伤闪烁动画

```
名称: DamageFlash
时长: 0.2 秒
循环: 2 次
轨道:
  - Color Tint: White → Red → White
```

### 低生命值脉动动画

```
名称: LowHealthPulse
时长: 1.0 秒
循环: 无限
轨道:
  - HealthBar Scale: (1, 1) → (1.05, 1.05) → (1, 1)
  - HealthBar Opacity: 1.0 → 0.7 → 1.0
```

### 在蓝图中使用动画

```blueprint
// 播放淡入动画
Event Construct
    ↓
Play Animation (FadeIn)

// 受伤时播放闪烁
Event OnHealthPercentChanged
    ↓
Branch (Health Percent < Previous Health Percent)
    ↓ True
    Play Animation (DamageFlash)

// 低生命值时循环播放脉动
Event OnHealthPercentChanged
    ↓
Branch (Health Percent < 0.3)
    ↓ True
    Play Animation (LowHealthPulse, Loop)
    ↓ False
    Stop Animation (LowHealthPulse)
```

---

## 完整配置检查清单

### Widget 蓝图 ✅
- [ ] 父类设置为 `EnemyHealthBarWidget`
- [ ] 添加 `HealthBar` Progress Bar（必须）
- [ ] 添加可选文本组件
- [ ] 所有组件设置为 `Is Variable`
- [ ] 绑定 `Event WidgetControllerSet`
- [ ] 绑定所有委托事件
- [ ] 实现颜色变化逻辑
- [ ] 添加动画（可选）

### 敌人蓝图 ✅
- [ ] 选择 `HealthBar` 组件
- [ ] 设置 `Widget Class`
- [ ] 设置 `Draw Size`
- [ ] 设置 `Space` 为 Screen
- [ ] 设置 `Pivot` 为 (0.5, 0.5)
- [ ] 配置 `Enemy Name`
- [ ] 配置 `Level`
- [ ] 设置 `Health Bar Widget Controller Class`

### 测试 ✅
- [ ] 放置敌人到关卡
- [ ] 运行游戏
- [ ] 检查 HUD 是否显示
- [ ] 攻击敌人测试生命值更新
- [ ] 检查颜色变化
- [ ] 检查动画效果
- [ ] 测试性能

---

## 示例项目文件

完整的示例项目文件可以在以下位置找到：

```
Content/
├── UI/
│   └── Enemy/
│       ├── WBP_EnemyHealthBar.uasset
│       └── WBP_EnemyHealthBar_Advanced.uasset
└── Blueprints/
    └── Character/
        └── Enemy/
            ├── BP_Goblin.uasset
            └── BP_Orc.uasset
```

---

## 总结

通过以上配置，你可以创建一个功能完整、外观专业的敌人头顶 HUD 系统。记得根据你的游戏风格调整颜色、字体和动画效果！
