# 交互系统详细文档

## 目录

1. [系统概述](#系统概述)
2. [接口系统](#接口系统)
3. [检查点系统](#检查点系统)
4. [高亮系统](#高亮系统)
5. [存档接口](#存档接口)
6. [配置指南](#配置指南)
7. [使用示例](#使用示例)

---

## 系统概述

交互系统提供了游戏中各种交互功能的接口和实现，包括检查点、高亮、存档等。

### 核心组件

- **IHighlightInterface**: 高亮接口
- **IEnemyInterface**: 敌人接口
- **ISaveInterface**: 存档接口
- **ACheckpoint**: 检查点 Actor

### 系统特点

- ✅ 接口驱动设计
- ✅ 高亮系统
- ✅ 检查点系统
- ✅ 存档集成
- ✅ 可扩展性

---

## 接口系统

### IHighlightInterface

高亮接口，用于高亮显示可交互对象。

#### 接口定义

```cpp
class AURA_API IHighlightInterface
{
    GENERATED_BODY()
    
public:
    UFUNCTION(BlueprintNativeEvent)
    void HighlightActor();
    
    UFUNCTION(BlueprintNativeEvent)
    void UnHighlightActor();
    
    UFUNCTION(BlueprintNativeEvent)
    void SetMoveToLocation(FVector& OutDestination);
};
```

#### 实现示例（敌人）

```cpp
void AAuraEnemy::HighlightActor_Implementation()
{
    GetMesh()->SetRenderCustomDepth(true);
    Weapon->SetRenderCustomDepth(true);
}

void AAuraEnemy::UnHighlightActor_Implementation()
{
    GetMesh()->SetRenderCustomDepth(false);
    Weapon->SetRenderCustomDepth(false);
}

void AAuraEnemy::SetMoveToLocation_Implementation(FVector& OutDestination)
{
    // 敌人不响应移动命令
}
```

### IEnemyInterface

敌人接口，用于管理战斗目标。

#### 接口定义

```cpp
class AURA_API IEnemyInterface
{
    GENERATED_BODY()
    
public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void SetCombatTarget(AActor* InCombatTarget);
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    AActor* GetCombatTarget() const;
};
```

#### 实现示例

```cpp
void AAuraEnemy::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
    CombatTarget = InCombatTarget;
}

AActor* AAuraEnemy::GetCombatTarget_Implementation() const
{
    return CombatTarget;
}
```

### ISaveInterface

存档接口，用于支持存档系统。

#### 接口定义

```cpp
class AURA_API ISaveInterface
{
    GENERATED_BODY()
    
public:
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    bool ShouldLoadTransform();
    
    UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
    void LoadActor();
};
```

#### 实现示例

```cpp
bool AMyActor::ShouldLoadTransform_Implementation()
{
    return true;  // 加载变换
}

void AMyActor::LoadActor_Implementation()
{
    // 加载后的后处理
}
```

---

## 检查点系统

### ACheckpoint

检查点 Actor，继承自 `APlayerStart`，实现 `ISaveInterface` 和 `IHighlightInterface`。

#### 核心功能

- **玩家起始点**: 作为玩家起始位置
- **存档点**: 保存游戏进度
- **高亮显示**: 支持高亮显示
- **视觉效果**: 检查点到达效果

#### 核心属性

```cpp
UPROPERTY(BlueprintReadWrite, SaveGame)
bool bReached = false;

UPROPERTY(EditAnywhere)
bool bBindOverlapCallback = true;

UPROPERTY(VisibleAnywhere)
TObjectPtr<USceneComponent> MoveToComponent;

UPROPERTY(EditDefaultsOnly)
int32 CustomDepthStencilOverride = CUSTOM_DEPTH_TAN;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
TObjectPtr<UStaticMeshComponent> CheckpointMesh;

UPROPERTY(VisibleAnywhere)
TObjectPtr<USphereComponent> Sphere;
```

#### 重叠处理

```cpp
void ACheckpoint::OnSphereOverlap(
    UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex, 
    bool bFromSweep, 
    const FHitResult& SweepResult
)
{
    // 检查是否为玩家
    if (!OtherActor->Implements<UPlayerInterface>()) return;
    
    // 检查是否已到达
    if (bReached) return;
    
    // 标记为已到达
    bReached = true;
    
    // 保存游戏
    if (IPlayerInterface* PlayerInterface = Cast<IPlayerInterface>(OtherActor))
    {
        // 获取检查点标签
        FName CheckpointTag = GetFName();
        
        // 保存进度
        IPlayerInterface::Execute_SaveProgress(OtherActor, CheckpointTag);
    }
    
    // 处理视觉效果
    HandleGlowEffects();
    
    // 触发蓝图事件
    CheckpointReached(DynamicMaterialInstance);
}
```

#### 高亮实现

```cpp
void ACheckpoint::HighlightActor_Implementation()
{
    CheckpointMesh->SetRenderCustomDepth(true);
    CheckpointMesh->SetCustomDepthStencilValue(CustomDepthStencilOverride);
}

void ACheckpoint::UnHighlightActor_Implementation()
{
    CheckpointMesh->SetRenderCustomDepth(false);
}

void ACheckpoint::SetMoveToLocation_Implementation(FVector& OutDestination)
{
    OutDestination = MoveToComponent->GetComponentLocation();
}
```

#### 存档集成

```cpp
bool ACheckpoint::ShouldLoadTransform_Implementation()
{
    return false;  // 检查点不加载变换
}

void ACheckpoint::LoadActor_Implementation()
{
    // 如果已到达，应用视觉效果
    if (bReached)
    {
        HandleGlowEffects();
    }
}
```

#### 视觉效果处理

```cpp
void ACheckpoint::HandleGlowEffects()
{
    // 创建动态材质实例
    if (UMaterialInterface* Material = CheckpointMesh->GetMaterial(0))
    {
        UMaterialInstanceDynamic* DynamicMaterial = 
            UMaterialInstanceDynamic::Create(Material, this);
        CheckpointMesh->SetMaterial(0, DynamicMaterial);
        
        // 设置发光参数
        DynamicMaterial->SetScalarParameterValue("GlowIntensity", 1.0f);
        
        // 触发蓝图事件
        CheckpointReached(DynamicMaterial);
    }
}
```

---

## 高亮系统

### 工作原理

高亮系统使用自定义深度（Custom Depth）和模板值（Stencil Value）来实现高亮效果。

#### 自定义深度值

```cpp
// 在 Aura.h 中定义
#define CUSTOM_DEPTH_RED 250
#define CUSTOM_DEPTH_BLUE 251
#define CUSTOM_DEPTH_TAN 252
```

#### 高亮流程

```
1. 玩家光标指向对象
   ↓
2. 检测到可高亮对象
   ↓
3. 调用 HighlightActor()
   ↓
4. 设置 RenderCustomDepth = true
   ↓
5. 设置 CustomDepthStencilValue
   ↓
6. 后处理材质显示高亮
```

#### 取消高亮流程

```
1. 玩家光标移开对象
   ↓
2. 调用 UnHighlightActor()
   ↓
3. 设置 RenderCustomDepth = false
   ↓
4. 高亮效果消失
```

### 在角色中实现

```cpp
// 在构造函数中设置
GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);

// 实现接口
void AAuraEnemy::HighlightActor_Implementation()
{
    GetMesh()->SetRenderCustomDepth(true);
    Weapon->SetRenderCustomDepth(true);
}

void AAuraEnemy::UnHighlightActor_Implementation()
{
    GetMesh()->SetRenderCustomDepth(false);
    Weapon->SetRenderCustomDepth(false);
}
```

---

## 存档接口

### ISaveInterface

存档接口定义了 Actor 需要实现的存档相关方法。

#### 接口方法

##### ShouldLoadTransform

```cpp
UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
bool ShouldLoadTransform();
```

**功能**: 决定是否从存档加载 Actor 的变换

**返回**: 
- `true`: 加载变换
- `false`: 不加载变换

##### LoadActor

```cpp
UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
void LoadActor();
```

**功能**: Actor 加载完成后的后处理

**用途**:
- 恢复 Actor 的特殊状态
- 重新初始化组件
- 更新视觉效果

### 实现要求

要实现存档功能，Actor 需要：

1. **实现接口**: 实现 `ISaveInterface`
2. **标记变量**: 需要保存的变量使用 `UPROPERTY(SaveGame)` 标记
3. **实现方法**: 实现 `ShouldLoadTransform()` 和 `LoadActor()`

### 使用示例

```cpp
// 在 Actor 中
class AURA_API AMyActor : public AActor, public ISaveInterface
{
    GENERATED_BODY()
    
public:
    // 需要保存的变量
    UPROPERTY(SaveGame)
    bool bActivated = false;
    
    // 实现接口方法
    virtual bool ShouldLoadTransform_Implementation() override 
    { 
        return true; 
    }
    
    virtual void LoadActor_Implementation() override
    {
        // 恢复状态
        if (bActivated)
        {
            Activate();
        }
    }
};
```

---

## 配置指南

### 创建检查点

#### 步骤 1: 创建检查点蓝图

1. 创建继承自 `BP_Checkpoint` 的蓝图
2. 设置检查点网格和位置

#### 步骤 2: 配置组件

- **CheckpointMesh**: 检查点网格
- **Sphere**: 碰撞球体
- **MoveToComponent**: 移动目标组件

#### 步骤 3: 配置属性

- **CustomDepthStencilOverride**: 自定义深度值
- **bBindOverlapCallback**: 是否绑定重叠回调

### 实现高亮接口

#### 步骤 1: 实现接口

```cpp
class AURA_API AMyActor : public AActor, public IHighlightInterface
{
    GENERATED_BODY()
    
public:
    virtual void HighlightActor_Implementation() override;
    virtual void UnHighlightActor_Implementation() override;
    virtual void SetMoveToLocation_Implementation(FVector& OutDestination) override;
};
```

#### 步骤 2: 实现方法

```cpp
void AMyActor::HighlightActor_Implementation()
{
    // 设置高亮
    Mesh->SetRenderCustomDepth(true);
    Mesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
}

void AMyActor::UnHighlightActor_Implementation()
{
    // 取消高亮
    Mesh->SetRenderCustomDepth(false);
}

void AMyActor::SetMoveToLocation_Implementation(FVector& OutDestination)
{
    // 设置移动目标位置
    OutDestination = MoveToComponent->GetComponentLocation();
}
```

---

## 使用示例

### 创建可交互对象

```cpp
// 1. 实现 IHighlightInterface
// 2. 实现高亮方法
// 3. 在玩家控制器中检测和调用
```

### 创建检查点

```cpp
// 1. 创建检查点蓝图
// 2. 设置检查点位置
// 3. 配置视觉效果
// 4. 玩家到达时自动保存
```

### 实现存档功能

```cpp
// 1. 实现 ISaveInterface
// 2. 标记需要保存的变量
// 3. 实现加载方法
// 4. 在 GameMode 中自动处理
```

---

## 最佳实践

### 1. 接口设计

- **单一职责**: 每个接口只负责一个功能
- **清晰命名**: 使用清晰的接口和方法命名
- **文档完善**: 为接口添加详细注释

### 2. 高亮系统

- **性能优化**: 避免频繁切换高亮状态
- **视觉效果**: 使用合适的视觉效果
- **用户体验**: 提供清晰的视觉反馈

### 3. 存档系统

- **最小化数据**: 只保存必要的数据
- **版本控制**: 考虑存档版本兼容性
- **错误处理**: 处理存档加载失败的情况

---

## 总结

交互系统提供了完整的游戏交互功能：

- ✅ **接口驱动**: 使用接口实现解耦
- ✅ **高亮系统**: 完整的对象高亮功能
- ✅ **检查点系统**: 自动化的检查点功能
- ✅ **存档集成**: 与存档系统无缝集成
- ✅ **可扩展性**: 易于扩展新的交互类型

通过这个系统，开发者可以创建丰富的游戏交互体验。

