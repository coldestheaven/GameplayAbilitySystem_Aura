# 近战技能实现指南

## 概述

本文档详细说明如何实现一个近战攻击技能。近战技能通过动画蒙太奇触发，使用武器碰撞检测来判定命中，对目标造成物理伤害。

### 技能特点

- ✅ 使用动画蒙太奇播放攻击动画
- ✅ 通过武器碰撞检测判定命中
- ✅ 支持多种攻击动画（连击）
- ✅ 造成物理伤害
- ✅ 支持击退和死亡冲量效果

---

## 实现步骤概览

```
1. 创建近战技能 C++ 类
   ↓
2. 添加 GameplayTag
   ↓
3. 创建技能蓝图
   ↓
4. 创建动画蒙太奇
   ↓
5. 创建动画通知（处理伤害检测）
   ↓
6. 创建 GameplayEffect（Cost、Cooldown、Damage）
   ↓
7. 配置 AbilityInfo
   ↓
8. 测试
```

---

## 步骤 1: 创建近战技能 C++ 类

### 1.1 创建头文件

在 `Source/Aura/Public/AbilitySystem/Abilities/` 目录下创建 `AuraMeleeSlash.h`：

```cpp
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "AuraMeleeSlash.generated.h"

/**
 * 近战斩击技能
 * 使用武器进行近战攻击，造成物理伤害
 */
UCLASS()
class AURA_API UAuraMeleeSlash : public UAuraDamageGameplayAbility
{
    GENERATED_BODY()
    
public:
    UAuraMeleeSlash();
    
    virtual FString GetDescription(int32 Level) override;
    virtual FString GetNextLevelDescription(int32 Level) override;
    
protected:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;
    
    // 攻击动画蒙太奇数组
    UPROPERTY(EditDefaultsOnly, Category = "Melee")
    TArray<FTaggedMontage> AttackMontages;
};
```

### 1.2 创建实现文件

在 `Source/Aura/Private/AbilitySystem/Abilities/` 目录下创建 `AuraMeleeSlash.cpp`：

```cpp
#include "AbilitySystem/Abilities/AuraMeleeSlash.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AuraGameplayTags.h"
#include "Interaction/CombatInterface.h"

UAuraMeleeSlash::UAuraMeleeSlash()
{
    const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
    AbilityTags.AddTag(Tags.Abilities_Attack_MeleeSlash);
    StartupInputTag = Tags.InputTag_LMB;
}

FString UAuraMeleeSlash::GetDescription(int32 Level)
{
    const int32 ScaledDamage = Damage.GetValueAtLevel(Level);
    const float ManaCost = FMath::Abs(GetManaCost(Level));
    const float Cooldown = GetCooldown(Level);
    
    return FString::Printf(
        TEXT(
            "<Title>MELEE SLASH</>\n\n"
            "<Small>Level: </><Level>%d</>\n"
            "<Small>ManaCost: </><ManaCost>%.1f</>\n"
            "<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
            "<Default>Performs a melee slash attack, dealing </>"
            "<Damage>%d</>"
            "<Default> physical damage.</>"
        ),
        Level,
        ManaCost,
        Cooldown,
        ScaledDamage
    );
}

FString UAuraMeleeSlash::GetNextLevelDescription(int32 Level)
{
    const int32 NextDamage = Damage.GetValueAtLevel(Level + 1);
    const int32 CurrentDamage = Damage.GetValueAtLevel(Level);
    const int32 DamageIncrease = NextDamage - CurrentDamage;
    
    return FString::Printf(
        TEXT(
            "<Title>NEXT LEVEL: </>\n\n"
            "<Small>Level: </><Level>%d</>\n\n"
            "<Default>Damage: </><Damage>%d</><Default> (+%d)</>"
        ),
        Level + 1,
        NextDamage,
        DamageIncrease
    );
}

void UAuraMeleeSlash::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData
)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    // 获取随机攻击动画
    FTaggedMontage AttackMontage = GetRandomTaggedMontageFromArray(AttackMontages);
    
    if (!AttackMontage.Montage)
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    // 播放动画蒙太奇
    if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo()))
    {
        if (UAnimInstance* AnimInstance = ICombatInterface::Execute_GetAnimInstance(GetAvatarActorFromActorInfo()))
        {
            AnimInstance->Montage_Play(AttackMontage.Montage);
            
            // 绑定动画通知事件（如果使用 GameplayEvent）
            // 伤害检测会在动画通知中触发
        }
    }
    
    // 注意：技能不会立即结束，需要等待动画完成或通过动画通知结束
    // 可以在动画蒙太奇中设置通知来结束技能
}
```

---

## 步骤 2: 添加 GameplayTag

### 2.1 在 AuraGameplayTags.h 中添加

```cpp
struct FAuraGameplayTags
{
    // ... 现有 Tags ...
    
    // 近战技能 Tag
    FGameplayTag Abilities_Attack_MeleeSlash;
    
    // 近战技能冷却 Tag
    FGameplayTag Cooldown_Attack_MeleeSlash;
};
```

### 2.2 在 AuraGameplayTags.cpp 中初始化

```cpp
void FAuraGameplayTags::InitializeNativeGameplayTags()
{
    // ... 现有初始化 ...
    
    // 初始化近战技能 Tag
    GameplayTags.Abilities_Attack_MeleeSlash = UGameplayTagsManager::Get()
        .AddNativeGameplayTag(
            FName("Abilities.Attack.MeleeSlash"),
            FString("Melee slash attack ability")
        );
    
    // 初始化冷却 Tag
    GameplayTags.Cooldown_Attack_MeleeSlash = UGameplayTagsManager::Get()
        .AddNativeGameplayTag(
            FName("Cooldown.Attack.MeleeSlash"),
            FString("Melee slash cooldown tag")
        );
}
```

---

## 步骤 3: 创建技能蓝图

### 3.1 创建蓝图类

1. 在编辑器中，右键点击 `AuraMeleeSlash` C++ 类
2. 选择 "Create Blueprint class based on AuraMeleeSlash"
3. 命名为 `BP_GA_MeleeSlash`
4. 保存到 `Content/Blueprints/AbilitySystem/Abilities/Melee/`

### 3.2 配置技能参数

1. **基础设置**
   - **Ability Tags**: 添加 `Abilities.Attack.MeleeSlash`
   - **Startup Input Tag**: `InputTag.LMB`

2. **伤害设置**
   - **Damage**: 配置曲线表（Level 1: 50, Level 2: 75, ...）
   - **Damage Type**: `Damage.Physical`
   - **Damage Effect Class**: `GE_MeleeSlash_Damage`

3. **攻击动画设置**
   - **Attack Montages**: 添加多个 `FTaggedMontage` 条目
     - **Montage**: 选择攻击动画蒙太奇
     - **Montage Tag**: 设置标签（例如 `Montage.Attack.Slash1`）
     - **Socket Tag**: 设置插槽标签（例如 `CombatSocket.RightHand`）
     - **Impact Sound**: 设置碰撞音效

---

## 步骤 4: 创建动画蒙太奇

### 4.1 创建动画蒙太奇

1. 在编辑器中创建新的动画蒙太奇
2. 命名为 `AM_MeleeSlash1`
3. 设置动画序列（攻击动画）

### 4.2 配置动画蒙太奇

1. **设置插槽**
   - 在蒙太奇中设置插槽（例如 `DefaultSlot`）

2. **添加通知轨道**
   - 添加 `GameplayEvent` 通知轨道
   - 在攻击命中时刻添加通知
   - 设置通知标签（例如 `Event.Melee.Hit`）

3. **添加结束通知**
   - 在动画结束时添加通知
   - 用于结束技能

---

## 步骤 5: 创建动画通知（处理伤害检测）

### 5.1 创建 C++ 通知类（可选）

如果需要自定义逻辑，可以创建 C++ 通知类：

**AuraMeleeHitNotify.h**:
```cpp
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AuraMeleeHitNotify.generated.h"

/**
 * 近战攻击命中通知
 * 在动画播放到指定时刻时触发伤害检测
 */
UCLASS()
class AURA_API UAuraMeleeHitNotify : public UAnimNotify
{
    GENERATED_BODY()
    
public:
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
    
    // 伤害检测范围
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DamageRadius = 100.f;
};
```

**AuraMeleeHitNotify.cpp**:
```cpp
#include "Animation/AnimNotifies/AuraMeleeHitNotify.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Interaction/CombatInterface.h"
#include "Interaction/EnemyInterface.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "Aura/Aura.h"

void UAuraMeleeHitNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    Super::Notify(MeshComp, Animation);
    
    if (!MeshComp || !MeshComp->GetOwner())
    {
        return;
    }
    
    AActor* Owner = MeshComp->GetOwner();
    
    // 获取角色的 AbilitySystemComponent
    UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
    if (!ASC)
    {
        return;
    }
    
    // 获取武器位置（从 CombatInterface）
    FVector WeaponLocation = Owner->GetActorLocation();
    if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Owner))
    {
        // 可以添加获取武器位置的方法
        // WeaponLocation = CombatInterface->GetWeaponLocation();
    }
    
    // 检测范围内的敌人
    TArray<FOverlapResult> OverlapResults;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Owner);
    
    GetWorld()->OverlapMultiByObjectType(
        OverlapResults,
        WeaponLocation,
        FQuat::Identity,
        FCollisionObjectQueryParams(ECC_Pawn),
        FCollisionShape::MakeSphere(DamageRadius),
        QueryParams
    );
    
    // 对每个命中的敌人造成伤害
    for (const FOverlapResult& Overlap : OverlapResults)
    {
        if (AActor* HitActor = Overlap.GetActor())
        {
            // 检查是否是敌人
            if (!HitActor->Implements<UEnemyInterface>())
            {
                continue;
            }
            
            // 检查是否是友方
            if (!UAuraAbilitySystemLibrary::IsNotFriend(Owner, HitActor))
            {
                continue;
            }
            
            // 获取目标的 ASC
            UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
            if (!TargetASC)
            {
                continue;
            }
            
            // 触发 GameplayEvent 来应用伤害
            // 这需要在技能中监听事件
            FGameplayEventData EventData;
            EventData.Instigator = Owner;
            EventData.Target = HitActor;
            
            ASC->HandleGameplayEvent(
                FGameplayTag::RequestGameplayTag(FName("Event.Melee.Hit")),
                &EventData
            );
        }
    }
}
```

### 5.2 在技能中监听事件

在技能蓝图中或 C++ 中监听 `Event.Melee.Hit` 事件：

```cpp
// 在技能激活时绑定事件
void UAuraMeleeSlash::ActivateAbility(...)
{
    Super::ActivateAbility(...);
    
    // 绑定事件监听
    if (UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo())
    {
        ASC->GenericGameplayEventCallbacks.FindOrAdd(
            FGameplayTag::RequestGameplayTag(FName("Event.Melee.Hit"))
        ).AddUObject(this, &UAuraMeleeSlash::OnMeleeHit);
    }
    
    // ... 播放动画 ...
}

// 处理命中事件
void UAuraMeleeSlash::OnMeleeHit(const FGameplayEventData* EventData)
{
    if (EventData && EventData->Target)
    {
        // 对目标造成伤害
        CauseDamage(EventData->Target);
    }
}
```

---

## 步骤 6: 创建 GameplayEffect

### 6.1 创建 Cost GameplayEffect

1. 创建 `GE_MeleeSlash_Cost`
2. 设置：
   - **Duration Policy**: `Instant`
   - **Modifier**: `Mana`, `Subtract`, `SetByCaller(Data.ManaCost)`

### 6.2 创建 Cooldown GameplayEffect

1. 创建 `GE_MeleeSlash_Cooldown`
2. 设置：
   - **Duration Policy**: `HasDuration`
   - **Duration Magnitude**: 0.5（0.5秒冷却，或更短）
   - **Granted Tags**: `Cooldown.Attack.MeleeSlash`
   - **Ignore Tags**: `Cooldown.Attack.MeleeSlash`

### 6.3 创建 Damage GameplayEffect

1. 创建 `GE_MeleeSlash_Damage`
2. 设置：
   - **Duration Policy**: `Instant`
   - **Execution Calculation**: `ExecCalc_Damage`
   - **Modifier**: `IncomingDamage`, `Additive`, `SetByCaller(Damage.Physical)`

---

## 步骤 7: 配置 AbilityInfo

在 `DA_AbilityInfo` 数据资产中添加：

- **Ability Tag**: `Abilities.Attack.MeleeSlash`
- **Input Tag**: `InputTag.LMB`
- **Status Tag**: `Abilities.Status.Equipped`（通常近战攻击是初始技能）
- **Cooldown Tag**: `Cooldown.Attack.MeleeSlash`
- **Ability Type**: `Abilities.Type.Offensive`
- **Icon**: 近战技能图标
- **Level Requirement**: 1（初始技能）
- **Ability**: `BP_GA_MeleeSlash`

---

## 步骤 8: 优化和改进

### 8.1 支持连击系统

可以扩展技能以支持连击：

```cpp
// 在技能类中添加
UPROPERTY(EditDefaultsOnly, Category = "Melee")
int32 MaxComboCount = 3;

UPROPERTY()
int32 CurrentComboCount = 0;

// 在激活时检查连击
void UAuraMeleeSlash::ActivateAbility(...)
{
    // 如果正在连击，使用下一个动画
    if (CurrentComboCount > 0 && CurrentComboCount < MaxComboCount)
    {
        // 使用下一个连击动画
        FTaggedMontage NextMontage = AttackMontages[CurrentComboCount];
        // ...
        CurrentComboCount++;
    }
    else
    {
        // 开始新的连击
        CurrentComboCount = 1;
        // ...
    }
}
```

### 8.2 添加范围攻击

可以添加范围伤害支持：

```cpp
// 在技能参数中
UPROPERTY(EditDefaultsOnly, Category = "Melee")
bool bIsAreaAttack = false;

UPROPERTY(EditDefaultsOnly, Category = "Melee")
float AttackRadius = 200.f;

// 在伤害检测时
if (bIsAreaAttack)
{
    // 检测范围内的所有敌人
    // 对每个敌人造成伤害
}
```

### 8.3 添加击退效果

在技能蓝图中配置：
- **Knockback Force Magnitude**: 击退力度
- **Knockback Chance**: 击退几率

---

## 测试清单

- ✅ 技能可以激活
- ✅ 动画正确播放
- ✅ 伤害检测正确触发
- ✅ 对敌人造成伤害
- ✅ 友方不受伤害
- ✅ 击退效果正确（如配置）
- ✅ 连击系统正常（如实现）
- ✅ 技能在动画结束后正确结束
- ✅ 冷却时间正确
- ✅ 法力消耗正确（如配置）

---

## 常见问题

### Q1: 伤害检测不触发

**可能原因**:
- 动画通知未正确设置
- 通知标签不匹配
- 事件监听未绑定

**解决方法**:
- 检查动画蒙太奇中的通知设置
- 确保通知标签与技能中监听的标签匹配
- 确保事件监听在技能激活时正确绑定

### Q2: 伤害值不正确

**可能原因**:
- `Damage` 曲线表未配置
- `DamageEffectClass` 未设置
- SetByCaller 标签不匹配

**解决方法**:
- 检查 `Damage` 曲线表配置
- 检查 `DamageEffectClass` 是否设置
- 确保 SetByCaller 标签为 `Damage.Physical`

### Q3: 动画不播放

**可能原因**:
- 动画蒙太奇未设置
- 角色没有 AnimInstance
- 动画插槽不匹配

**解决方法**:
- 检查 `AttackMontages` 数组是否配置
- 确保角色有 AnimInstance
- 检查动画插槽设置

---

## 总结

实现近战技能需要：

1. ✅ **创建技能类** - 继承自 `UAuraDamageGameplayAbility`
2. ✅ **配置动画蒙太奇** - 设置攻击动画
3. ✅ **创建动画通知** - 处理伤害检测
4. ✅ **配置 GameplayEffect** - Cost、Cooldown、Damage
5. ✅ **配置 AbilityInfo** - 技能信息
6. ✅ **测试** - 全面测试所有功能

遵循这些步骤，可以成功实现一个功能完整的近战攻击技能。

