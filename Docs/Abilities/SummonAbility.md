# SummonAbility (召唤技能)

## 技能概述

**SummonAbility** 是一个召唤技能，在角色周围生成多个召唤物（Minions）。召唤物可以帮助战斗，攻击敌人。

### 基本信息

- **技能类型**: 召唤技能 (Summon Ability)
- **基础类**: `UAuraSummonAbility`
- **技能标签**: `Abilities.Summon`

### 技能特点

- ✅ 生成多个召唤物
- ✅ 支持多种召唤物类型
- ✅ 召唤物自动战斗
- ✅ 管理召唤物数量
- ✅ 扇形分布生成

---

## 技能描述

召唤技能通常没有详细的技能描述，具体描述取决于实现。

---

## 技能参数

### 基础参数

| 参数 | 类型 | 默认值 | 说明 |
|------|------|--------|------|
| `NumMinions` | int32 | 5 | 召唤物数量 |
| `MinionClasses` | TArray<TSubclassOf<APawn>> | - | 召唤物类数组 |
| `MinSpawnDistance` | float | 50.0 | 最小生成距离 |
| `MaxSpawnDistance` | float | 250.0 | 最大生成距离 |
| `SpawnSpread` | float | 90.0 | 生成散布角度（度） |

---

## 实现细节

### 技能激活流程

```cpp
void UAuraSummonAbility::ActivateAbility(...)
{
    Super::ActivateAbility(...);
    
    // 1. 获取生成位置
    TArray<FVector> SpawnLocations = GetSpawnLocations();
    
    // 2. 生成每个召唤物
    for (int32 i = 0; i < SpawnLocations.Num(); i++)
    {
        // 获取随机召唤物类
        TSubclassOf<APawn> MinionClass = GetRandomMinionClass();
        
        // 生成召唤物
        APawn* Minion = GetWorld()->SpawnActor<APawn>(
            MinionClass,
            SpawnLocations[i],
            FRotator::ZeroRotator
        );
        
        // 设置召唤物属性
        // ...
    }
    
    // 3. 更新召唤物计数
    ICombatInterface::Execute_IncremenetMinionCount(
        GetAvatarActorFromActorInfo(),
        SpawnLocations.Num()
    );
    
    EndAbility(...);
}
```

### 生成位置计算

```cpp
TArray<FVector> UAuraSummonAbility::GetSpawnLocations()
{
    // 1. 获取角色位置和朝向
    const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
    const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
    
    // 2. 计算扇形分布的起始方向
    const float DeltaSpread = SpawnSpread / NumMinions;
    const FVector LeftOfSpread = Forward.RotateAngleAxis(-SpawnSpread / 2.f, FVector::UpVector);
    
    // 3. 计算每个召唤物的生成位置
    TArray<FVector> SpawnLocations;
    for (int32 i = 0; i < NumMinions; i++)
    {
        // 计算方向
        const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
        
        // 计算距离（随机）
        float Distance = FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance);
        
        // 计算位置
        FVector ChosenSpawnLocation = Location + Direction * Distance;
        
        // 地面检测
        FHitResult Hit;
        GetWorld()->LineTraceSingleByChannel(
            Hit,
            ChosenSpawnLocation + FVector(0.f, 0.f, 400.f),
            ChosenSpawnLocation - FVector(0.f, 0.f, 400.f),
            ECC_Visibility
        );
        
        if (Hit.bBlockingHit)
        {
            ChosenSpawnLocation = Hit.ImpactPoint;
        }
        
        SpawnLocations.Add(ChosenSpawnLocation);
    }
    
    return SpawnLocations;
}
```

### 随机召唤物类

```cpp
TSubclassOf<APawn> UAuraSummonAbility::GetRandomMinionClass()
{
    if (MinionClasses.Num() == 0)
    {
        return nullptr;
    }
    
    const int32 Selection = FMath::RandRange(0, MinionClasses.Num() - 1);
    return MinionClasses[Selection];
}
```

### 召唤物管理

召唤物数量通过 `ICombatInterface` 管理：

```cpp
// 增加召唤物计数
void IncremenetMinionCount(int32 Amount);

// 获取召唤物数量
int32 GetMinionCount();
```

---

## 召唤物行为

### 自动战斗

召唤物通常具有以下行为：

1. **自动寻敌**: 寻找附近的敌人
2. **自动攻击**: 对敌人进行攻击
3. **跟随主人**: 在一定范围内跟随主人
4. **死亡处理**: 死亡时更新计数

### AI 控制

召唤物使用 AI 控制器：

- **行为树**: 使用行为树控制行为
- **目标选择**: 自动选择攻击目标
- **移动**: 自动移动到合适位置

---

## 升级效果

### 等级提升

- 召唤物数量可以随等级增加
- 召唤物属性可以提升
- 可以解锁更多召唤物类型

### 升级建议

- 优先增加召唤物数量
- 提升召唤物属性
- 解锁更强力的召唤物类型

---

## 使用技巧

### 战斗技巧

1. **位置选择**: 在安全位置召唤，避免被打断
2. **数量管理**: 注意召唤物数量上限
3. **类型搭配**: 使用不同类型的召唤物形成配合
4. **时机把握**: 在战斗前召唤，提前准备

### 最佳使用场景

- ✅ 需要额外火力支援
- ✅ 需要吸引敌人注意力
- ✅ 需要控制战场
- ❌ 不适合在狭窄空间使用

---

## 配置指南

### 在编辑器中配置

1. **创建技能蓝图**
   - 继承自 `UAuraSummonAbility`
   - 配置召唤物参数

2. **配置召唤物**
   - 设置 `NumMinions`（召唤物数量）
   - 添加 `MinionClasses`（召唤物类数组）

3. **配置生成参数**
   - 设置 `MinSpawnDistance` 和 `MaxSpawnDistance`
   - 设置 `SpawnSpread`（散布角度）

4. **创建召唤物类**
   - 创建召唤物 Pawn 类
   - 配置 AI 控制器
   - 设置行为树

5. **创建 GameplayEffect**
   - Cost GameplayEffect
   - Cooldown GameplayEffect

---

## 代码示例

### 自定义生成位置

```cpp
// 在子类中重写
TArray<FVector> UMySummonAbility::GetSpawnLocations()
{
    // 自定义生成逻辑
    // 例如：圆形分布
    TArray<FVector> Locations;
    const FVector Center = GetAvatarActorFromActorInfo()->GetActorLocation();
    
    for (int32 i = 0; i < NumMinions; i++)
    {
        float Angle = (360.f / NumMinions) * i;
        float Distance = FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance);
        
        FVector Location = Center + FVector(
            FMath::Cos(FMath::DegreesToRadians(Angle)) * Distance,
            FMath::Sin(FMath::DegreesToRadians(Angle)) * Distance,
            0.f
        );
        
        Locations.Add(Location);
    }
    
    return Locations;
}
```

---

## 相关文件

- **头文件**: `Source/Aura/Public/AbilitySystem/Abilities/AuraSummonAbility.h`
- **实现文件**: `Source/Aura/Private/AbilitySystem/Abilities/AuraSummonAbility.cpp`
- **基类**: `UAuraGameplayAbility`
- **战斗接口**: `ICombatInterface`

---

## 总结

SummonAbility 是一个强大的召唤技能，通过生成多个召唤物提供了额外的战斗支援。技能适合需要额外火力或控制的场景，可以通过配置不同类型的召唤物形成多样化的战斗风格。

