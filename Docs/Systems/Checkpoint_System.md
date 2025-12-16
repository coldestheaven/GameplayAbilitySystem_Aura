# Checkpoint 系统文档

## 概述

Checkpoint 系统管理游戏中的检查点和地图入口。检查点用于保存玩家进度和设置重生点，地图入口用于在不同地图之间切换。

## 核心组件

### ACheckpoint

检查点类，继承自 `APlayerStart`，实现 `ISaveInterface` 和 `IHighlightInterface`。

#### 类层次结构

```
APlayerStart (UE5 Base)
    ↓
ACheckpoint
    ├── ISaveInterface
    └── IHighlightInterface
```

#### 核心功能

1. **玩家重生点**
   - 作为 PlayerStart 使用
   - 设置玩家重生位置

2. **进度保存**
   - 记录玩家到达的检查点
   - 保存到存档系统

3. **高亮系统**
   - 玩家可以高亮检查点
   - 显示移动目标位置

#### 关键属性

```cpp
// 是否已到达
UPROPERTY(BlueprintReadWrite, SaveGame)
bool bReached = false;

// 是否绑定重叠回调
UPROPERTY(EditAnywhere)
bool bBindOverlapCallback = true;

// 移动目标组件
UPROPERTY(VisibleAnywhere)
TObjectPtr<USceneComponent> MoveToComponent;

// 检查点网格
UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
TObjectPtr<UStaticMeshComponent> CheckpointMesh;

// 碰撞球体
UPROPERTY(VisibleAnywhere)
TObjectPtr<USphereComponent> Sphere;

// 自定义深度模板值
UPROPERTY(EditDefaultsOnly)
int32 CustomDepthStencilOverride = CUSTOM_DEPTH_TAN;
```

#### 关键方法

**Save Interface**:
```cpp
virtual bool ShouldLoadTransform_Implementation() override { return false; }
virtual void LoadActor_Implementation() override;
```

**Highlight Interface**:
```cpp
virtual void SetMoveToLocation_Implementation(FVector& OutDestination) override;
virtual void HighlightActor_Implementation() override;
virtual void UnHighlightActor_Implementation() override;
```

**重叠处理**:
```cpp
virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, 
    AActor* OtherActor, UPrimitiveComponent* OtherComp, 
    int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
```

#### 实现细节

**到达检查点**:
```cpp
void ACheckpoint::OnSphereOverlap(...)
{
    if (APawn* OverlappingPawn = Cast<APawn>(OtherActor))
    {
        if (OverlappingPawn->IsPlayerControlled())
        {
            // 标记为已到达
            bReached = true;
            
            // 保存进度
            if (ISaveInterface* SaveInterface = Cast<ISaveInterface>(OverlappingPawn))
            {
                SaveInterface->SaveProgress_Implementation(GetActorLocation());
            }
            
            // 播放效果
            CheckpointReached(DynamicMaterialInstance);
        }
    }
}
```

**加载检查点状态**:
```cpp
void ACheckpoint::LoadActor_Implementation()
{
    if (bReached)
    {
        // 更新视觉效果
        HandleGlowEffects();
    }
}
```

---

### AMapEntrance

地图入口类，继承自 `ACheckpoint`，用于在不同地图之间切换。

#### 核心功能

1. **地图切换**
   - 指定目标地图
   - 指定目标 PlayerStart Tag
   - 切换地图时保存当前进度

2. **高亮显示**
   - 继承检查点的高亮功能
   - 显示为可交互的地图入口

#### 关键属性

```cpp
// 目标地图
UPROPERTY(EditAnywhere)
TSoftObjectPtr<UWorld> DestinationMap;

// 目标 PlayerStart Tag
UPROPERTY(EditAnywhere)
FName DestinationPlayerStartTag;
```

#### 关键方法

**地图切换**:
```cpp
void AMapEntrance::OnSphereOverlap(...)
{
    Super::OnSphereOverlap(...);
    
    if (APawn* OverlappingPawn = Cast<APawn>(OtherActor))
    {
        if (OverlappingPawn->IsPlayerControlled())
        {
            // 保存当前地图进度
            // ...
            
            // 切换地图
            UGameplayStatics::OpenLevelBySoftObjectPtr(
                this, DestinationMap, true, DestinationPlayerStartTag.ToString()
            );
        }
    }
}
```

---

## 检查点系统流程

### 1. 玩家到达检查点

```
玩家进入 Sphere 碰撞范围
    ↓
OnSphereOverlap 触发
    ↓
检查是否是玩家
    ↓
设置 bReached = true
    ↓
保存进度到存档
    ↓
播放视觉效果
```

### 2. 玩家重生

```
玩家死亡
    ↓
从存档加载最后到达的检查点
    ↓
使用检查点作为 PlayerStart
    ↓
在检查点位置重生玩家
```

### 3. 地图切换

```
玩家进入地图入口
    ↓
保存当前地图进度
    ↓
加载目标地图
    ↓
在目标地图的指定 PlayerStart 生成玩家
```

---

## 存档系统集成

### 保存检查点

```cpp
// 在 SaveGame 中
UPROPERTY(SaveGame)
TMap<FName, bool> CheckpointsReached;

// 保存时
CheckpointsReached.Add(Checkpoint->GetFName(), Checkpoint->bReached);
```

### 加载检查点

```cpp
// 加载时
if (bool* bReached = CheckpointsReached.Find(Checkpoint->GetFName()))
{
    Checkpoint->bReached = *bReached;
    Checkpoint->LoadActor_Implementation();
}
```

---

## 高亮系统

### 高亮检查点

```cpp
void ACheckpoint::HighlightActor_Implementation()
{
    // 设置自定义深度
    CheckpointMesh->SetRenderCustomDepth(true);
    CheckpointMesh->SetCustomDepthStencilValue(CustomDepthStencilOverride);
}

void ACheckpoint::UnHighlightActor_Implementation()
{
    // 取消自定义深度
    CheckpointMesh->SetRenderCustomDepth(false);
}
```

### 移动目标

```cpp
void ACheckpoint::SetMoveToLocation_Implementation(FVector& OutDestination)
{
    OutDestination = MoveToComponent->GetComponentLocation();
}
```

---

## 视觉效果

### 到达效果

```cpp
UFUNCTION(BlueprintImplementableEvent)
void CheckpointReached(UMaterialInstanceDynamic* DynamicMaterialInstance);

UFUNCTION(BlueprintCallable)
void HandleGlowEffects();
```

在蓝图中实现：
- 播放粒子效果
- 更新材质
- 播放音效

---

## 配置指南

### 创建检查点

1. **在编辑器中放置**
   - 从 Content Browser 拖拽 Checkpoint 到场景
   - 设置位置和旋转

2. **配置属性**
   - **bBindOverlapCallback**: 是否自动绑定重叠回调
   - **CustomDepthStencilOverride**: 高亮时的模板值
   - **CheckpointMesh**: 检查点网格

3. **设置 PlayerStart Tag**
   - 在检查点的 PlayerStart 属性中设置 Tag
   - 用于地图切换时指定重生点

### 创建地图入口

1. **继承检查点**
   - 使用 MapEntrance 类
   - 继承所有检查点功能

2. **配置目标**
   - **DestinationMap**: 目标地图（Soft Object Reference）
   - **DestinationPlayerStartTag**: 目标 PlayerStart 的 Tag

---

## 相关文档

- [交互系统](./Interaction_System.md) - Save Interface 和 Highlight Interface
- [Gameplay 框架](../Gameplay/Gameplay_Framework.md) - 存档系统和地图切换

---

## 总结

Checkpoint 系统提供了：

1. ✅ **进度保存** - 记录玩家到达的检查点
2. ✅ **重生系统** - 玩家在检查点重生
3. ✅ **地图切换** - 在不同地图之间切换
4. ✅ **高亮系统** - 可交互的检查点显示
5. ✅ **存档集成** - 完整的存档系统支持

通过这个系统，可以完整地管理玩家的游戏进度和地图切换。

