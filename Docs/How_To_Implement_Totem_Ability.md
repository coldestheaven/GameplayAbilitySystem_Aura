# 图腾召唤技能实现指南

## 概述

本文档详细说明如何实现一个图腾召唤技能，该技能可以：
- 在指定位置放置一个图腾
- 图腾对进入范围内的敌人造成每秒魔法伤害
- 图腾持续一定时间后自动消失

---

## 实现步骤概览

```
1. 创建图腾 Actor 类（C++）
   ↓
2. 创建图腾技能类（C++）
   ↓
3. 添加 GameplayTag
   ↓
4. 创建图腾蓝图
   ↓
5. 创建技能蓝图
   ↓
6. 创建 GameplayEffect（Cost、Cooldown、持续伤害）
   ↓
7. 配置 AbilityInfo
   ↓
8. 测试
```

---

## 步骤 1: 创建图腾 Actor 类

### 1.1 创建头文件

在 `Source/Aura/Public/Actor/` 目录下创建 `AuraTotem.h`：

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AuraAbilityTypes.h"
#include "AuraTotem.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UNiagaraComponent;
class UAbilitySystemComponent;

/**
 * 图腾 Actor
 * 放置在指定位置，对范围内的敌人造成持续伤害
 */
UCLASS()
class AURA_API AAuraTotem : public AActor
{
    GENERATED_BODY()
    
public:	
    AAuraTotem();
    
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    
    // 初始化图腾
    UFUNCTION(BlueprintCallable)
    void InitializeTotem(
        TObjectPtr<UAbilitySystemComponent> InSourceASC,
        TSubclassOf<UGameplayEffect> InDamageEffectClass,
        float InDamagePerSecond,
        FGameplayTag InDamageType,
        float InDamageRadius,
        float InTotemDuration,
        float InAbilityLevel
    );
    
protected:
    // 范围检测组件
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<USphereComponent> SphereComponent;
    
    // 图腾网格
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UStaticMeshComponent> TotemMesh;
    
    // 视觉效果
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    TObjectPtr<UNiagaraComponent> AuraEffect;
    
    // 伤害处理
    UFUNCTION()
    void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, 
        const FHitResult& SweepResult);
    
    UFUNCTION()
    void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
    
    // 定期伤害处理
    UFUNCTION()
    void ApplyDamageToTargets();
    
    // 定时器句柄
    FTimerHandle DamageTimerHandle;
    
private:
    // 伤害相关
    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> SourceASC;
    
    UPROPERTY()
    TSubclassOf<UGameplayEffect> DamageEffectClass;
    
    float DamagePerSecond;
    FGameplayTag DamageType;
    float AbilityLevel;
    
    // 范围
    float DamageRadius;
    
    // 持续时间
    float TotemDuration;
    
    // 当前范围内的目标
    UPROPERTY()
    TArray<TObjectPtr<AActor>> OverlappingTargets;
    
    // 伤害间隔（秒）
    UPROPERTY(EditDefaultsOnly, Category = "Totem")
    float DamageInterval = 1.0f;
};
```

### 1.2 创建实现文件

在 `Source/Aura/Private/Actor/` 目录下创建 `AuraTotem.cpp`：

```cpp
#include "Actor/AuraTotem.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Interaction/EnemyInterface.h"
#include "Aura/Aura.h"

AAuraTotem::AAuraTotem()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;
    
    // 创建根组件
    RootComponent = CreateDefaultSubobject<USceneComponent>("RootComponent");
    
    // 创建范围检测组件
    SphereComponent = CreateDefaultSubobject<USphereComponent>("SphereComponent");
    SphereComponent->SetupAttachment(RootComponent);
    SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
    SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
    SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    
    // 创建图腾网格
    TotemMesh = CreateDefaultSubobject<UStaticMeshComponent>("TotemMesh");
    TotemMesh->SetupAttachment(RootComponent);
    
    // 创建视觉效果组件
    AuraEffect = CreateDefaultSubobject<UNiagaraComponent>("AuraEffect");
    AuraEffect->SetupAttachment(RootComponent);
}

void AAuraTotem::BeginPlay()
{
    Super::BeginPlay();
    
    // 绑定重叠事件
    SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AAuraTotem::OnSphereBeginOverlap);
    SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AAuraTotem::OnSphereEndOverlap);
    
    // 设置生命周期
    if (TotemDuration > 0.f)
    {
        SetLifeSpan(TotemDuration);
    }
    
    // 开始定期伤害
    if (GetWorld() && DamageInterval > 0.f)
    {
        GetWorld()->GetTimerManager().SetTimer(
            DamageTimerHandle,
            this,
            &AAuraTotem::ApplyDamageToTargets,
            DamageInterval,
            true  // 循环
        );
    }
}

void AAuraTotem::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AAuraTotem::InitializeTotem(
    TObjectPtr<UAbilitySystemComponent> InSourceASC,
    TSubclassOf<UGameplayEffect> InDamageEffectClass,
    float InDamagePerSecond,
    FGameplayTag InDamageType,
    float InDamageRadius,
    float InTotemDuration,
    float InAbilityLevel
)
{
    SourceASC = InSourceASC;
    DamageEffectClass = InDamageEffectClass;
    DamagePerSecond = InDamagePerSecond;
    DamageType = InDamageType;
    DamageRadius = InDamageRadius;
    TotemDuration = InTotemDuration;
    AbilityLevel = InAbilityLevel;
    
    // 设置范围
    if (SphereComponent)
    {
        SphereComponent->SetSphereRadius(DamageRadius);
    }
}

void AAuraTotem::OnSphereBeginOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    // 检查是否是敌人
    if (OtherActor && OtherActor->Implements<UEnemyInterface>())
    {
        // 检查是否是友方（避免伤害友军）
        if (UAuraAbilitySystemLibrary::IsNotFriend(GetOwner(), OtherActor))
        {
            OverlappingTargets.AddUnique(OtherActor);
        }
    }
}

void AAuraTotem::OnSphereEndOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex
)
{
    if (OtherActor)
    {
        OverlappingTargets.Remove(OtherActor);
    }
}

void AAuraTotem::ApplyDamageToTargets()
{
    if (!SourceASC || !DamageEffectClass || OverlappingTargets.Num() == 0)
    {
        return;
    }
    
    // 计算单次伤害（每秒伤害 / 伤害间隔）
    const float DamagePerTick = DamagePerSecond * DamageInterval;
    
    // 对每个目标造成伤害
    for (AActor* Target : OverlappingTargets)
    {
        if (!IsValid(Target))
        {
            continue;
        }
        
        // 检查目标是否仍然有效
        if (!Target->Implements<UEnemyInterface>())
        {
            continue;
        }
        
        // 检查是否是友方
        if (!UAuraAbilitySystemLibrary::IsNotFriend(GetOwner(), Target))
        {
            continue;
        }
        
        // 获取目标的 AbilitySystemComponent
        UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
        if (!TargetASC)
        {
            continue;
        }
        
        // 创建伤害效果规格
        FGameplayEffectContextHandle EffectContextHandle = SourceASC->MakeEffectContext();
        EffectContextHandle.AddSourceObject(this);
        
        FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
            DamageEffectClass,
            AbilityLevel,
            EffectContextHandle
        );
        
        // 设置伤害值
        UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(
            SpecHandle,
            DamageType,
            DamagePerTick
        );
        
        // 应用伤害
        SourceASC->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
    }
}
```

---

## 步骤 2: 创建图腾技能类

### 2.1 创建头文件

在 `Source/Aura/Public/AbilitySystem/Abilities/` 目录下创建 `AuraTotemAbility.h`：

```cpp
#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/AuraDamageGameplayAbility.h"
#include "AuraTotemAbility.generated.h"

class AAuraTotem;

/**
 * 图腾召唤技能
 * 在指定位置放置图腾，图腾对范围内的敌人造成持续伤害
 */
UCLASS()
class AURA_API UAuraTotemAbility : public UAuraDamageGameplayAbility
{
    GENERATED_BODY()
    
public:
    UAuraTotemAbility();
    
    virtual FString GetDescription(int32 Level) override;
    virtual FString GetNextLevelDescription(int32 Level) override;
    
protected:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData
    ) override;
    
    // 生成图腾
    UFUNCTION(BlueprintCallable, Category = "Totem")
    void SpawnTotem(const FVector& SpawnLocation);
    
    // 图腾类
    UPROPERTY(EditDefaultsOnly, Category = "Totem")
    TSubclassOf<AAuraTotem> TotemClass;
    
    // 伤害半径
    UPROPERTY(EditDefaultsOnly, Category = "Totem")
    float TotemRadius = 500.f;
    
    // 图腾持续时间（秒）
    UPROPERTY(EditDefaultsOnly, Category = "Totem")
    float TotemDuration = 10.f;
    
    // 每秒伤害（Scalable Float，支持按等级缩放）
    UPROPERTY(EditDefaultsOnly, Category = "Totem")
    FScalableFloat DamagePerSecond;
};
```

### 2.2 创建实现文件

在 `Source/Aura/Private/AbilitySystem/Abilities/` 目录下创建 `AuraTotemAbility.cpp`：

```cpp
#include "AbilitySystem/Abilities/AuraTotemAbility.h"
#include "Actor/AuraTotem.h"
#include "AbilitySystem/AuraAbilitySystemLibrary.h"
#include "AuraGameplayTags.h"
#include "Interaction/CombatInterface.h"

UAuraTotemAbility::UAuraTotemAbility()
{
    const FAuraGameplayTags& Tags = FAuraGameplayTags::Get();
    AbilityTags.AddTag(Tags.Abilities_Summon_Totem);
    StartupInputTag = Tags.InputTag_RMB;
}

FString UAuraTotemAbility::GetDescription(int32 Level)
{
    const float ScaledDPS = DamagePerSecond.GetValueAtLevel(Level);
    const float ManaCost = FMath::Abs(GetManaCost(Level));
    const float Cooldown = GetCooldown(Level);
    
    return FString::Printf(
        TEXT(
            "<Title>TOTEM</>\n\n"
            "<Small>Level: </><Level>%d</>\n"
            "<Small>ManaCost: </><ManaCost>%.1f</>\n"
            "<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
            "<Default>Places a totem that deals </>"
            "<Damage>%.1f</>"
            "<Default> magic damage per second to enemies within </>"
            "<Damage>%.0f</>"
            "<Default> units for </>"
            "<Damage>%.0f</>"
            "<Default> seconds.</>"
        ),
        Level,
        ManaCost,
        Cooldown,
        ScaledDPS,
        TotemRadius,
        TotemDuration
    );
}

FString UAuraTotemAbility::GetNextLevelDescription(int32 Level)
{
    const float NextDPS = DamagePerSecond.GetValueAtLevel(Level + 1);
    const float CurrentDPS = DamagePerSecond.GetValueAtLevel(Level);
    const float DPSIncrease = NextDPS - CurrentDPS;
    
    return FString::Printf(
        TEXT(
            "<Title>NEXT LEVEL: </>\n\n"
            "<Small>Level: </><Level>%d</>\n\n"
            "<Default>Damage Per Second: </><Damage>%.1f</><Default> (+%.1f)</>"
        ),
        Level + 1,
        NextDPS,
        DPSIncrease
    );
}

void UAuraTotemAbility::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData
)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
    
    // 获取目标位置（鼠标指向的位置）
    // 这里假设从鼠标点击或目标位置获取
    // 实际实现可能需要根据你的输入系统调整
    
    // 示例：从鼠标位置获取
    FVector TargetLocation = GetAvatarActorFromActorInfo()->GetActorLocation() + 
        GetAvatarActorFromActorInfo()->GetActorForwardVector() * 300.f;
    
    // 或者从 CombatInterface 获取目标位置
    if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo()))
    {
        // 可以添加一个方法来获取目标位置
        // TargetLocation = CombatInterface->GetTargetLocation();
    }
    
    // 地面检测
    FHitResult Hit;
    FVector TraceStart = TargetLocation + FVector(0.f, 0.f, 500.f);
    FVector TraceEnd = TargetLocation - FVector(0.f, 0.f, 500.f);
    
    GetWorld()->LineTraceSingleByChannel(
        Hit,
        TraceStart,
        TraceEnd,
        ECC_Visibility
    );
    
    if (Hit.bBlockingHit)
    {
        TargetLocation = Hit.ImpactPoint;
    }
    
    // 生成图腾
    SpawnTotem(TargetLocation);
    
    // 结束技能
    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

void UAuraTotemAbility::SpawnTotem(const FVector& SpawnLocation)
{
    const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
    if (!bIsServer || !TotemClass)
    {
        return;
    }
    
    // 创建生成变换
    FTransform SpawnTransform;
    SpawnTransform.SetLocation(SpawnLocation);
    SpawnTransform.SetRotation(FRotator::ZeroRotator.Quaternion());
    
    // 生成图腾
    AAuraTotem* Totem = GetWorld()->SpawnActorDeferred<AAuraTotem>(
        TotemClass,
        SpawnTransform,
        GetOwningActorFromActorInfo(),
        Cast<APawn>(GetOwningActorFromActorInfo()),
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn
    );
    
    if (Totem)
    {
        // 初始化图腾
        const float ScaledDPS = DamagePerSecond.GetValueAtLevel(GetAbilityLevel());
        
        Totem->InitializeTotem(
            GetAbilitySystemComponentFromActorInfo(),  // Source ASC
            DamageEffectClass,                          // Damage Effect Class
            ScaledDPS,                                  // Damage Per Second
            DamageType,                                 // Damage Type
            TotemRadius,                                // Damage Radius
            TotemDuration,                              // Totem Duration
            GetAbilityLevel()                           // Ability Level
        );
        
        // 完成生成
        Totem->FinishSpawning(SpawnTransform);
    }
}
```

---

## 步骤 3: 添加 GameplayTag

### 3.1 在 AuraGameplayTags.h 中添加

```cpp
struct FAuraGameplayTags
{
    // ... 现有 Tags ...
    
    // 图腾技能 Tag
    FGameplayTag Abilities_Summon_Totem;
    
    // 图腾冷却 Tag
    FGameplayTag Cooldown_Summon_Totem;
};
```

### 3.2 在 AuraGameplayTags.cpp 中初始化

```cpp
void FAuraGameplayTags::InitializeNativeGameplayTags()
{
    // ... 现有初始化 ...
    
    // 初始化图腾技能 Tag
    GameplayTags.Abilities_Summon_Totem = UGameplayTagsManager::Get()
        .AddNativeGameplayTag(
            FName("Abilities.Summon.Totem"),
            FString("Totem summoning ability")
        );
    
    // 初始化图腾冷却 Tag
    GameplayTags.Cooldown_Summon_Totem = UGameplayTagsManager::Get()
        .AddNativeGameplayTag(
            FName("Cooldown.Summon.Totem"),
            FString("Totem cooldown tag")
        );
}
```

---

## 步骤 4: 创建图腾蓝图

### 4.1 创建蓝图类

1. 在编辑器中，右键点击 `AuraTotem` C++ 类
2. 选择 "Create Blueprint class based on AuraTotem"
3. 命名为 `BP_Totem`
4. 保存到 `Content/Blueprints/Actor/Totems/`

### 4.2 配置图腾

1. **设置网格**
   - 在 `TotemMesh` 中设置图腾的静态网格
   - 调整位置和缩放

2. **设置视觉效果**
   - 在 `AuraEffect` 中设置范围效果的 Niagara 系统
   - 调整效果大小以匹配伤害半径

3. **设置碰撞**
   - `SphereComponent` 的半径会在运行时根据 `DamageRadius` 设置
   - 确保碰撞设置正确

4. **设置材质和外观**
   - 为图腾添加材质
   - 设置视觉效果的颜色和强度

---

## 步骤 5: 创建技能蓝图

### 5.1 创建蓝图类

1. 在编辑器中，右键点击 `AuraTotemAbility` C++ 类
2. 选择 "Create Blueprint class based on AuraTotemAbility"
3. 命名为 `BP_GA_Totem`
4. 保存到 `Content/Blueprints/AbilitySystem/Abilities/Totem/`

### 5.2 配置技能参数

1. **基础设置**
   - **Ability Tags**: 添加 `Abilities.Summon.Totem`
   - **Startup Input Tag**: `InputTag.RMB`

2. **图腾设置**
   - **Totem Class**: 选择 `BP_Totem`
   - **Totem Radius**: 500.0（伤害半径）
   - **Totem Duration**: 10.0（持续时间，秒）

3. **伤害设置**
   - **Damage Per Second**: 配置曲线表
     - Level 1: 10.0
     - Level 2: 15.0
     - Level 3: 20.0
     - ...
   - **Damage Type**: `Damage.Arcane`（或你想要的伤害类型）
   - **Damage Effect Class**: `GE_Totem_Damage`（需要创建）

---

## 步骤 6: 创建 GameplayEffect

### 6.1 创建持续伤害 GameplayEffect

1. **创建 GameplayEffect**
   - 命名为 `GE_Totem_Damage`
   - 保存到 `Content/Blueprints/AbilitySystem/Effects/Damage/`

2. **配置属性**
   - **Duration Policy**: `HasDuration`
   - **Duration Magnitude**: 1.0（1秒，与伤害间隔匹配）
   - **Period**: 1.0（每秒触发一次）
   - **Periodic Execution Calculation**: `ExecCalc_Damage`

3. **添加修改器**
   - **Attribute**: `IncomingDamage`
   - **Modifier Op**: `Additive`
   - **Magnitude Calculation Type**: `SetByCaller`
   - **SetByCaller Magnitude**: `Damage.Arcane`（或对应伤害类型）

**注意**: 这个 GameplayEffect 会被图腾定期应用，每次应用时通过 SetByCaller 设置伤害值。

### 6.2 创建 Cost 和 Cooldown GameplayEffect

按照标准流程创建：
- `GE_Totem_Cost`: 法力消耗
- `GE_Totem_Cooldown`: 冷却时间

---

## 步骤 7: 配置 AbilityInfo

在 `DA_AbilityInfo` 数据资产中添加：

- **Ability Tag**: `Abilities.Summon.Totem`
- **Input Tag**: `InputTag.RMB`
- **Status Tag**: `Abilities.Status.Locked`
- **Cooldown Tag**: `Cooldown.Summon.Totem`
- **Ability Type**: `Abilities.Type.Offensive`
- **Icon**: 图腾技能图标
- **Level Requirement**: 5（或其他等级）
- **Ability**: `BP_GA_Totem`

---

## 步骤 8: 优化和改进

### 8.1 改进目标位置获取

可以在技能蓝图中实现更精确的目标位置获取：

```cpp
// 在蓝图中
void UAuraTotemAbility::ActivateAbility(...)
{
    Super::ActivateAbility(...);
    
    // 从鼠标位置获取目标
    FHitResult HitResult;
    GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
    
    if (HitResult.bBlockingHit)
    {
        SpawnTotem(HitResult.ImpactPoint);
    }
    else
    {
        // 使用默认位置
        FVector DefaultLocation = GetAvatarActorFromActorInfo()->GetActorLocation() + 
            GetAvatarActorFromActorInfo()->GetActorForwardVector() * 300.f;
        SpawnTotem(DefaultLocation);
    }
    
    EndAbility(...);
}
```

### 8.2 添加视觉效果

在图腾生成时添加特效：

```cpp
// 在 AuraTotem.cpp 的 BeginPlay 中
void AAuraTotem::BeginPlay()
{
    Super::BeginPlay();
    
    // 播放生成特效
    if (SpawnEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            this,
            SpawnEffect,
            GetActorLocation()
        );
    }
    
    // ... 其他初始化 ...
}
```

### 8.3 添加音效

在图腾生成和伤害时添加音效：

```cpp
// 在 AuraTotem.cpp 中
UPROPERTY(EditDefaultsOnly, Category = "Totem")
TObjectPtr<USoundBase> SpawnSound;

UPROPERTY(EditDefaultsOnly, Category = "Totem")
TObjectPtr<USoundBase> DamageSound;

void AAuraTotem::BeginPlay()
{
    // ... 其他代码 ...
    
    if (SpawnSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, SpawnSound, GetActorLocation());
    }
}

void AAuraTotem::ApplyDamageToTargets()
{
    // ... 伤害逻辑 ...
    
    if (DamageSound && OverlappingTargets.Num() > 0)
    {
        UGameplayStatics::PlaySoundAtLocation(this, DamageSound, GetActorLocation());
    }
}
```

### 8.4 支持多个图腾

如果需要支持同时存在多个图腾，可以在技能中添加限制：

```cpp
// 在 AuraTotemAbility.h 中
UPROPERTY(EditDefaultsOnly, Category = "Totem")
int32 MaxTotems = 1;

// 在 AuraTotemAbility.cpp 中
UPROPERTY()
TArray<TObjectPtr<AAuraTotem>> ActiveTotems;

void UAuraTotemAbility::SpawnTotem(const FVector& SpawnLocation)
{
    // 检查图腾数量限制
    ActiveTotems.RemoveAll([](AAuraTotem* Totem) { return !IsValid(Totem); });
    
    if (ActiveTotems.Num() >= MaxTotems)
    {
        // 销毁最旧的图腾
        if (ActiveTotems.Num() > 0)
        {
            ActiveTotems[0]->Destroy();
            ActiveTotems.RemoveAt(0);
        }
    }
    
    // ... 生成新图腾 ...
    
    if (Totem)
    {
        ActiveTotems.Add(Totem);
    }
}
```

---

## 测试清单

- ✅ 技能可以激活
- ✅ 图腾在正确位置生成
- ✅ 图腾视觉效果正确
- ✅ 敌人进入范围时开始受到伤害
- ✅ 伤害值正确（每秒伤害）
- ✅ 敌人离开范围时停止受到伤害
- ✅ 图腾在持续时间结束后自动消失
- ✅ 多个敌人可以同时受到伤害
- ✅ 友方单位不受伤害
- ✅ 法力消耗正确
- ✅ 冷却时间正确

---

## 常见问题

### Q1: 图腾不生成

**可能原因**:
- `TotemClass` 未设置
- 不在服务器上执行
- 生成位置无效

**解决方法**:
- 检查 `TotemClass` 是否在技能蓝图中设置
- 确保 `SpawnTotem` 在服务器上执行（使用 `HasAuthority()` 检查）
- 检查生成位置是否有效

### Q2: 图腾不造成伤害

**可能原因**:
- `DamageEffectClass` 未设置
- `SourceASC` 未正确传递
- 目标没有 AbilitySystemComponent

**解决方法**:
- 检查 `DamageEffectClass` 是否设置
- 检查 `InitializeTotem` 是否被正确调用
- 确保目标有 AbilitySystemComponent

### Q3: 伤害频率不正确

**可能原因**:
- `DamageInterval` 设置错误
- 定时器未正确设置

**解决方法**:
- 检查 `DamageInterval` 值（应该与 GameplayEffect 的 Period 匹配）
- 检查定时器是否在 `BeginPlay` 中正确设置

### Q4: 多个敌人同时受到伤害时性能问题

**优化方法**:
- 使用对象池管理伤害效果规格
- 减少伤害检查频率
- 使用更高效的碰撞检测

---

## 总结

实现图腾召唤技能需要：

1. ✅ **创建图腾 Actor** - 处理范围检测和持续伤害
2. ✅ **创建技能类** - 处理图腾生成
3. ✅ **配置 GameplayEffect** - 持续伤害效果
4. ✅ **创建蓝图** - 图腾和技能蓝图
5. ✅ **配置 AbilityInfo** - 技能信息
6. ✅ **测试** - 全面测试所有功能

遵循这些步骤，可以成功实现一个功能完整的图腾召唤技能。

