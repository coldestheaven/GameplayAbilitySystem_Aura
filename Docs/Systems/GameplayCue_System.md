# GameplayCue 系统详细文档

## 目录

1. [系统概述](#系统概述)
2. [GameplayCue 基础](#gameplaycue-基础)
3. [GameplayCue 类型](#gameplaycue-类型)
4. [GameplayCue 执行方式](#gameplaycue-执行方式)
5. [GameplayCue 参数](#gameplaycue-参数)
6. [项目中的 GameplayCue 实现](#项目中的-gameplaycue-实现)
7. [创建 GameplayCue](#创建-gameplaycue)
8. [执行 GameplayCue](#执行-gameplaycue)
9. [GameplayCue Notify](#gameplaycue-notify)
10. [最佳实践](#最佳实践)
11. [常见问题](#常见问题)

---

## 系统概述

GameplayCue 是 GAS 系统中用于处理视觉效果和音效的组件。与 GameplayEffect 不同，GameplayCue 不修改属性，只负责播放视觉效果、音效等表现层内容。

在 Aura 项目中，GameplayCue 用于：

- **视觉效果**: 播放粒子效果、动画等
- **音效**: 播放音效和音乐
- **UI 反馈**: 显示 UI 提示
- **物理效果**: 触发物理效果（如冲击波）

### 核心组件

- **UGameplayCueManager**: GameplayCue 管理器
- **UGameplayCueNotify_Static**: 静态 GameplayCue Notify
- **UGameplayCueNotify_Actor**: Actor 型 GameplayCue Notify
- **FGameplayCueParameters**: GameplayCue 参数结构
- **FGameplayCueTag**: GameplayCue 标签

### GameplayCue vs GameplayEffect

| 特性 | GameplayCue | GameplayEffect |
|------|-------------|----------------|
| **用途** | 视觉效果、音效 | 属性修改、状态效果 |
| **网络复制** | 可选（可非复制） | 必需（服务器权威） |
| **执行位置** | 客户端 | 服务器和客户端 |
| **性能** | 轻量级 | 较重 |
| **生命周期** | 短暂 | 可持久 |

---

## GameplayCue 基础

### 什么是 GameplayCue

GameplayCue 是一个轻量级的通知系统，用于触发视觉效果和音效。它通过 GameplayTag 标识，可以在客户端执行而不需要服务器同步。

### GameplayCue 标签

GameplayCue 使用特殊的标签命名规范：

```
GameplayCue.Category.Specific
```

**示例**：
- `GameplayCue.FireBlast` - 火焰冲击效果
- `GameplayCue.MeleeImpact` - 近战碰撞效果
- `GameplayCue.ShockBurst` - 电击爆发效果
- `GameplayCue.ShockLoop` - 电击循环效果

### GameplayCue 生命周期

```
执行 GameplayCue
    ↓
查找对应的 Notify
    ↓
OnExecute (一次性)
    ↓
OnActive (激活，持续效果)
    ↓
OnRemove (移除)
```

---

## GameplayCue 类型

### 1. 静态 GameplayCue (Static)

- **特点**: 简单的视觉效果，不需要复杂逻辑
- **用途**: 粒子效果、音效、简单动画
- **实现**: `UGameplayCueNotify_Static`

**示例**: 爆炸效果

```cpp
// 在蓝图中配置
GameplayCue Tag: GameplayCue.FireBlast
OnExecute:
  - Spawn Particle System (爆炸粒子)
  - Play Sound (爆炸音效)
```

### 2. Actor 型 GameplayCue (Actor)

- **特点**: 需要复杂逻辑或持续存在的效果
- **用途**: 持续粒子效果、复杂动画、物理效果
- **实现**: `UGameplayCueNotify_Actor`

**示例**: 持续电击效果

```cpp
// 在蓝图中配置
GameplayCue Tag: GameplayCue.ShockLoop
OnActive:
  - Spawn Actor (电击 Actor)
  - Attach to Target
  - Play Looping Particle
OnRemove:
  - Destroy Actor
```

---

## GameplayCue 执行方式

### 1. 复制执行 (Replicated)

通过 AbilitySystemComponent 执行，会自动复制到客户端：

```cpp
// 在服务器上执行，自动复制到客户端
ASC->ExecuteGameplayCue(
    FAuraGameplayTags::Get().GameplayCue_FireBlast,
    CueParams
);
```

### 2. 非复制执行 (Non-Replicated)

直接在本地执行，不进行网络复制：

```cpp
// 只在本地执行
UGameplayCueManager::ExecuteGameplayCue_NonReplicated(
    GetOwner(),
    FAuraGameplayTags::Get().GameplayCue_FireBlast,
    CueParams
);
```

### 3. 添加/移除 (Add/Remove)

对于持续效果，使用添加/移除方式：

```cpp
// 添加持续效果
ASC->AddGameplayCue(
    FAuraGameplayTags::Get().GameplayCue_ShockLoop,
    CueParams
);

// 移除效果
ASC->RemoveGameplayCue(
    FAuraGameplayTags::Get().GameplayCue_ShockLoop
);
```

---

## GameplayCue 参数

### FGameplayCueParameters

GameplayCue 参数结构，包含执行所需的信息：

```cpp
struct FGameplayCueParameters
{
    FGameplayEffectContextHandle EffectContext;  // Effect 上下文
    FVector_NetQuantize Location;                // 位置
    FVector_NetQuantize Normal;                  // 法线
    AActor* Instigator;                          // 触发者
    AActor* EffectCauser;                       // 效果来源
    UObject* SourceObject;                      // 源对象
    float NormalizedMagnitude;                  // 标准化量级
    float RawMagnitude;                        // 原始量级
    FGameplayTagContainer AggregatedSourceTags; // 源标签
    FGameplayTagContainer AggregatedTargetTags; // 目标标签
    FVector_NetQuantize Origin;                  // 原点
    bool bReplicateLocationWhenUsingMinimalRepProxy; // 是否复制位置
};
```

### 设置参数

```cpp
FGameplayCueParameters CueParams;
CueParams.Location = GetActorLocation();
CueParams.Instigator = GetInstigator();
CueParams.EffectCauser = this;
CueParams.NormalizedMagnitude = 1.0f;
CueParams.RawMagnitude = 100.0f;
```

---

## 项目中的 GameplayCue 实现

### GameplayCue 标签定义

在 `AuraGameplayTags.h` 中定义：

```cpp
struct FAuraGameplayTags
{
    FGameplayTag GameplayCue_FireBlast;
    // ... 更多 GameplayCue Tags
};
```

在 `AuraGameplayTags.cpp` 中注册：

```cpp
GameplayTags.GameplayCue_FireBlast = UGameplayTagsManager::Get().AddNativeGameplayTag(
    FName("GameplayCue.FireBlast"),
    FString("FireBlast GameplayCue Tag")
);
```

### 配置文件中的 Tags

在 `Config/DefaultGameplayTags.ini` 中定义：

```ini
+GameplayTagList=(Tag="GameplayCue.FireBlast",DevComment="")
+GameplayTagList=(Tag="GameplayCue.ArcaneShards",DevComment="")
+GameplayTagList=(Tag="GameplayCue.MeleeImpact",DevComment="")
+GameplayTagList=(Tag="GameplayCue.ShockBurst",DevComment="")
+GameplayTagList=(Tag="GameplayCue.ShockLoop",DevComment="")
```

### GameplayCue 路径配置

在 `Config/DefaultGame.ini` 中配置：

```ini
[/Script/GameplayAbilities.AbilitySystemGlobals]
+GameplayCueNotifyPaths=/Game/Blueprints/AbilitySystem/GameplayCueNotifies
```

### 项目中的使用示例

#### 示例 1: 火焰球爆炸效果

在 `AuraFireBall.cpp` 中：

```cpp
void AAuraFireBall::OnHit()
{
    if (GetOwner())
    {
        FGameplayCueParameters CueParams;
        CueParams.Location = GetActorLocation();
        UGameplayCueManager::ExecuteGameplayCue_NonReplicated(
            GetOwner(),
            FAuraGameplayTags::Get().GameplayCue_FireBlast,
            CueParams
        );
    }
    
    // 停止循环音效
    if (LoopingSoundComponent)
    {
        LoopingSoundComponent->Stop();
        LoopingSoundComponent->DestroyComponent();
    }
    
    bHit = true;
}
```

**说明**:
- 使用 `ExecuteGameplayCue_NonReplicated` 在本地执行
- 设置位置为投射物当前位置
- 触发爆炸视觉效果和音效

---

## 创建 GameplayCue

### 步骤 1: 创建 GameplayCue Tag

#### 1.1 在头文件中添加 Tag

在 `Source/Aura/Public/AuraGameplayTags.h` 中添加：

```cpp
struct FAuraGameplayTags
{
    // ... 现有 Tags
    
    FGameplayTag GameplayCue_IceExplosion;  // 冰霜爆炸效果
};
```

#### 1.2 在实现文件中注册 Tag

在 `Source/Aura/Private/AuraGameplayTags.cpp` 中注册：

```cpp
void FAuraGameplayTags::InitializeNativeGameplayTags()
{
    // ... 现有注册代码
    
    GameplayTags.GameplayCue_IceExplosion = UGameplayTagsManager::Get().AddNativeGameplayTag(
        FName("GameplayCue.IceExplosion"),
        FString("Ice Explosion GameplayCue Tag")
    );
}
```

#### 1.3 在配置文件中添加 Tag（可选）

在 `Config/DefaultGameplayTags.ini` 中添加：

```ini
+GameplayTagList=(Tag="GameplayCue.IceExplosion",DevComment="Ice explosion effect")
```

### 步骤 2: 创建 GameplayCue Notify 蓝图

#### 2.1 创建静态 Notify（简单效果）

1. 在内容浏览器中，导航到 `Content/Blueprints/AbilitySystem/GameplayCueNotifies/`
2. 右键 → `Gameplay` → `Gameplay Cue Notify Static`
3. 命名为 `GC_IceExplosion`
4. 配置 GameplayCue Tag: `GameplayCue.IceExplosion`

#### 2.2 配置效果

在 Notify 蓝图中：

**OnExecute 事件**:
```
Event OnExecute
├── Spawn Emitter at Location
│   └── Particle System: IceExplosion_Particle
├── Play Sound at Location
│   └── Sound: IceExplosion_Sound
└── Spawn Actor (可选)
    └── Actor Class: IceExplosionActor
```

#### 2.3 创建 Actor 型 Notify（复杂效果）

1. 右键 → `Gameplay` → `Gameplay Cue Notify Actor`
2. 命名为 `GC_ShockLoop_Actor`
3. 配置 GameplayCue Tag: `GameplayCue.ShockLoop`

**OnActive 事件**:
```
Event OnActive
├── Spawn Actor
│   └── Actor Class: ShockLoopActor
├── Attach to Target
└── Play Looping Particle
```

**OnRemove 事件**:
```
Event OnRemove
└── Destroy Actor
```

---

## 执行 GameplayCue

### 在 C++ 中执行

#### 非复制执行（推荐用于本地效果）

```cpp
void AMyActor::TriggerExplosion()
{
    FGameplayCueParameters CueParams;
    CueParams.Location = GetActorLocation();
    CueParams.Instigator = GetInstigator();
    CueParams.EffectCauser = this;
    CueParams.NormalizedMagnitude = 1.0f;
    
    UGameplayCueManager::ExecuteGameplayCue_NonReplicated(
        GetOwner(),
        FAuraGameplayTags::Get().GameplayCue_IceExplosion,
        CueParams
    );
}
```

#### 复制执行（需要网络同步）

```cpp
void AMyActor::TriggerExplosion()
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        FGameplayCueParameters CueParams;
        CueParams.Location = GetActorLocation();
        CueParams.Instigator = GetInstigator();
        CueParams.EffectCauser = this;
        
        ASC->ExecuteGameplayCue(
            FAuraGameplayTags::Get().GameplayCue_IceExplosion,
            CueParams
        );
    }
}
```

#### 添加/移除持续效果

```cpp
// 添加持续效果
void AMyActor::StartShockEffect()
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        FGameplayCueParameters CueParams;
        CueParams.Location = GetActorLocation();
        
        ASC->AddGameplayCue(
            FAuraGameplayTags::Get().GameplayCue_ShockLoop,
            CueParams
        );
    }
}

// 移除效果
void AMyActor::StopShockEffect()
{
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
    {
        ASC->RemoveGameplayCue(
            FAuraGameplayTags::Get().GameplayCue_ShockLoop
        );
    }
}
```

### 在蓝图中执行

#### 使用 Execute Gameplay Cue 节点

```
Execute Gameplay Cue
├── Gameplay Cue Tag: GameplayCue.IceExplosion
├── Parameters:
│   ├── Location: Actor Location
│   ├── Instigator: Self
│   └── Effect Causer: Self
└── (Non-Replicated)
```

---

## GameplayCue Notify

### 静态 Notify 事件

#### OnExecute

一次性效果，在 GameplayCue 执行时调用：

```cpp
// 在蓝图中实现
Event OnExecute
├── Parameters:
│   ├── Target: Actor
│   ├── GameplayCueParameters: CueParams
│   └── GameplayCueTag: CueTag
└── Actions:
    ├── Spawn Particle
    ├── Play Sound
    └── Trigger Animation
```

### Actor 型 Notify 事件

#### OnActive

效果激活时调用，用于持续效果：

```cpp
// 在蓝图中实现
Event OnActive
├── Parameters:
│   ├── Target: Actor
│   ├── GameplayCueParameters: CueParams
│   └── GameplayCueTag: CueTag
└── Actions:
    ├── Spawn Actor
    ├── Attach to Target
    └── Start Looping Effect
```

#### OnRemove

效果移除时调用，用于清理：

```cpp
// 在蓝图中实现
Event OnRemove
├── Parameters:
│   ├── Target: Actor
│   ├── GameplayCueParameters: CueParams
│   └── GameplayCueTag: CueTag
└── Actions:
    ├── Destroy Actor
    ├── Stop Particle
    └── Stop Sound
```

#### WhileActive

效果激活期间每帧调用（可选）：

```cpp
// 在蓝图中实现
Event WhileActive
├── Parameters:
│   ├── Target: Actor
│   ├── GameplayCueParameters: CueParams
│   └── GameplayCueTag: CueTag
└── Actions:
    └── Update Effect (如更新粒子位置)
```

---

## 最佳实践

### 1. GameplayCue 设计

- **轻量级**: GameplayCue 应该只处理视觉效果，不处理游戏逻辑
- **可复用**: 设计可复用的 GameplayCue，避免重复创建
- **清晰命名**: 使用清晰的命名（如 `GameplayCue.FireBlast`）

### 2. 执行方式选择

- **非复制执行**: 用于本地效果（如投射物碰撞）
- **复制执行**: 用于需要所有客户端看到的效果（如技能释放）
- **添加/移除**: 用于持续效果（如 Buff 光环）

### 3. 性能考虑

- **对象池**: 对于频繁创建的效果，使用对象池
- **LOD 系统**: 根据距离使用不同细节级别的效果
- **批量执行**: 避免在同一帧执行大量 GameplayCue

### 4. 网络优化

- **最小化复制**: 只在必要时使用复制执行
- **参数优化**: 只传递必要的参数
- **位置压缩**: 使用 `FVector_NetQuantize` 压缩位置

### 5. 错误处理

- **空指针检查**: 检查所有指针
- **参数验证**: 验证参数有效性
- **优雅降级**: 处理资源缺失的情况

---

## 常见问题

### 问题 1: GameplayCue 未执行

**原因**: Tag 未正确配置或 Notify 未找到

**解决方案**:
1. 检查 GameplayCue Tag 是否正确注册
2. 检查 Notify 路径配置是否正确
3. 检查 Notify 蓝图的 Tag 是否匹配
4. 检查 Notify 是否在正确的目录下

### 问题 2: 效果未显示

**原因**: 资源未正确配置或参数错误

**解决方案**:
1. 检查粒子系统、音效等资源是否正确配置
2. 检查位置参数是否正确设置
3. 检查效果是否被其他系统覆盖
4. 检查 LOD 设置是否导致效果被剔除

### 问题 3: 网络同步问题

**原因**: 使用了非复制执行但需要同步

**解决方案**:
1. 对于需要同步的效果，使用 `ExecuteGameplayCue` 而非 `ExecuteGameplayCue_NonReplicated`
2. 检查网络复制设置
3. 确保在服务器上执行

### 问题 4: 持续效果未移除

**原因**: 未正确调用移除函数

**解决方案**:
1. 确保在适当时机调用 `RemoveGameplayCue`
2. 检查效果是否被其他系统持有
3. 检查 OnRemove 事件是否正确实现

### 问题 5: 性能问题

**原因**: 执行频率过高或资源过重

**解决方案**:
1. 降低执行频率
2. 使用对象池管理效果 Actor
3. 优化粒子系统和音效资源
4. 使用 LOD 系统

---

## 完整示例

### 示例: 创建冰霜爆炸效果

#### 1. 添加 Tag

```cpp
// AuraGameplayTags.h
FGameplayTag GameplayCue_IceExplosion;

// AuraGameplayTags.cpp
GameplayTags.GameplayCue_IceExplosion = UGameplayTagsManager::Get().AddNativeGameplayTag(
    FName("GameplayCue.IceExplosion"),
    FString("Ice Explosion GameplayCue Tag")
);
```

#### 2. 创建 Notify 蓝图

1. 创建 `GC_IceExplosion` (Static Notify)
2. 配置 Tag: `GameplayCue.IceExplosion`
3. 在 OnExecute 中：
   - Spawn Particle: `IceExplosion_Particle`
   - Play Sound: `IceExplosion_Sound`
   - Spawn Decal: `IceExplosion_Decal`

#### 3. 执行 GameplayCue

```cpp
void AAuraIceProjectile::OnHit()
{
    FGameplayCueParameters CueParams;
    CueParams.Location = GetActorLocation();
    CueParams.Instigator = GetInstigator();
    CueParams.EffectCauser = this;
    
    UGameplayCueManager::ExecuteGameplayCue_NonReplicated(
        GetOwner(),
        FAuraGameplayTags::Get().GameplayCue_IceExplosion,
        CueParams
    );
}
```

---

## 总结

GameplayCue 是 GAS 系统中用于处理视觉效果和音效的组件：

- ✅ **轻量级**: 不修改属性，只处理表现层
- ✅ **灵活**: 支持静态和 Actor 型两种实现方式
- ✅ **高效**: 可非复制执行，减少网络开销
- ✅ **易用**: 通过蓝图配置，无需编写代码

通过合理使用 GameplayCue，可以实现丰富的视觉效果和音效反馈。

---

## 相关文档

- [GameplayTags 系统文档](./GameplayTags_System.md) - GameplayTags 详细文档
- [GameplayEffect 系统文档](./GameplayEffect_System.md) - GameplayEffect 详细文档
- [GameplayAbility 系统文档](./GameplayAbility_System.md) - GameplayAbility 详细文档
- [Actor 系统文档](./Actor_System.md) - Actor 系统实现

