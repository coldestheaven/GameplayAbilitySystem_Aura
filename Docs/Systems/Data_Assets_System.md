# 数据资产系统文档

## 概述

数据资产系统使用 Unreal Engine 的 Data Asset 系统来配置游戏数据，包括角色职业信息、能力信息、属性信息、等级信息等。这些数据资产提供了数据驱动的配置方式，无需修改代码即可调整游戏平衡。

## 核心数据资产

### UCharacterClassInfo

角色职业信息数据资产，定义每个职业的默认属性和初始能力。

#### 结构

```cpp
UCLASS()
class AURA_API UCharacterClassInfo : public UDataAsset
{
    // 职业信息映射
    UPROPERTY(EditDefaultsOnly, Category = "Character Class Defaults")
    TMap<ECharacterClass, FCharacterClassDefaultInfo> CharacterClassInformation;
    
    // 通用属性（所有职业共享）
    UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
    TSubclassOf<UGameplayEffect> PrimaryAttributes_SetByCaller;
    
    UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
    TSubclassOf<UGameplayEffect> SecondaryAttributes;
    
    UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
    TSubclassOf<UGameplayEffect> VitalAttributes;
    
    // 通用能力（所有职业共享）
    UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults")
    TArray<TSubclassOf<UGameplayAbility>> CommonAbilities;
    
    // 伤害计算系数
    UPROPERTY(EditDefaultsOnly, Category = "Common Class Defaults|Damage")
    TObjectPtr<UCurveTable> DamageCalculationCoefficients;
};
```

#### FCharacterClassDefaultInfo

```cpp
USTRUCT(BlueprintType)
struct FCharacterClassDefaultInfo
{
    GENERATED_BODY()
    
    // 主属性 GameplayEffect
    UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
    TSubclassOf<UGameplayEffect> PrimaryAttributes;
    
    // 初始能力
    UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
    TArray<TSubclassOf<UGameplayAbility>> StartupAbilities;
    
    // 经验值奖励（按等级）
    UPROPERTY(EditDefaultsOnly, Category = "Class Defaults")
    FScalableFloat XPReward = FScalableFloat();
};
```

#### 使用示例

```cpp
// 获取职业信息
UCharacterClassInfo* CharacterClassInfo = 
    UAuraAbilitySystemLibrary::GetCharacterClassInfo(GetWorld());

FCharacterClassDefaultInfo ClassInfo = 
    CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

// 应用主属性
ApplyGameplayEffectToSelf(ClassInfo.PrimaryAttributes, 1.f);

// 添加初始能力
for (TSubclassOf<UGameplayAbility> AbilityClass : ClassInfo.StartupAbilities)
{
    GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, -1, this));
}
```

---

### UAbilityInfo

能力信息数据资产，存储所有能力的元数据。

#### 结构

```cpp
UCLASS()
class AURA_API UAbilityInfo : public UDataAsset
{
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Ability Information")
    TArray<FAuraAbilityInfo> AbilityInformation;
    
    FAuraAbilityInfo FindAbilityInfoForTag(
        const FGameplayTag& AbilityTag, 
        bool bLogNotFound = false
    ) const;
};
```

#### FAuraAbilityInfo

```cpp
USTRUCT(BlueprintType)
struct FAuraAbilityInfo
{
    GENERATED_BODY()
    
    // 能力标签
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag AbilityTag = FGameplayTag();
    
    // 输入标签
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag InputTag = FGameplayTag();
    
    // 状态标签
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag StatusTag = FGameplayTag();
    
    // 冷却标签
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag CooldownTag = FGameplayTag();
    
    // 能力类型
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag AbilityType = FGameplayTag();
    
    // UI 信息
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<const UTexture2D> Icon = nullptr;
    
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TObjectPtr<const UMaterialInterface> BackgroundMaterial = nullptr;
    
    // 解锁要求
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    int32 LevelRequirement = 1;
    
    // 能力类
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TSubclassOf<UAuraGameplayAbility> Ability = nullptr;
};
```

#### 使用示例

```cpp
// 获取能力信息
UAbilityInfo* AbilityInfo = UAuraAbilitySystemLibrary::GetAbilityInfo(GetWorld());
FAuraAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);

// 在 UI 中显示
IconWidget->SetBrushFromTexture(Info.Icon);
LevelText->SetText(FText::AsNumber(Info.LevelRequirement));
```

---

### UAttributeInfo

属性信息数据资产，存储属性的 UI 显示信息。

#### 结构

```cpp
UCLASS()
class AURA_API UAttributeInfo : public UDataAsset
{
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    TArray<FAuraAttributeInfo> AttributeInformation;
    
    FAuraAttributeInfo FindAttributeInfoForTag(
        const FGameplayTag& AttributeTag,
        bool bLogNotFound = false
    ) const;
};
```

#### FAuraAttributeInfo

```cpp
USTRUCT(BlueprintType)
struct FAuraAttributeInfo
{
    GENERATED_BODY()
    
    // 属性标签
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FGameplayTag AttributeTag = FGameplayTag();
    
    // 显示名称
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText AttributeName = FText();
    
    // 描述
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
    FText AttributeDescription = FText();
};
```

#### 使用示例

```cpp
// 获取属性信息
UAttributeInfo* AttributeInfo = 
    UAuraAbilitySystemLibrary::GetAttributeInfo(GetWorld());
FAuraAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);

// 在 UI 中显示
NameText->SetText(Info.AttributeName);
DescriptionText->SetText(Info.AttributeDescription);
```

---

### ULevelUpInfo

等级信息数据资产，存储每个等级的经验值要求和奖励。

#### 结构

```cpp
UCLASS()
class AURA_API ULevelUpInfo : public UDataAsset
{
    UPROPERTY(EditDefaultsOnly)
    TArray<FLevelUpInfo> LevelUpInformation;
    
    int32 FindLevelForXP(int32 XP) const;
    int32 GetXPRequirementForLevel(int32 Level) const;
    int32 GetAttributePointReward(int32 Level) const;
    int32 GetSpellPointReward(int32 Level) const;
};
```

#### FLevelUpInfo

```cpp
USTRUCT(BlueprintType)
struct FLevelUpInfo
{
    GENERATED_BODY()
    
    // 升级所需经验值
    UPROPERTY(EditDefaultsOnly)
    int32 LevelUpRequirement = 0;
    
    // 属性点奖励
    UPROPERTY(EditDefaultsOnly)
    int32 AttributePointReward = 0;
    
    // 法术点奖励
    UPROPERTY(EditDefaultsOnly)
    int32 SpellPointReward = 0;
};
```

#### 使用示例

```cpp
// 获取等级信息
ULevelUpInfo* LevelUpInfo = PlayerState->LevelUpInfo;

// 查找当前等级
int32 CurrentLevel = LevelUpInfo->FindLevelForXP(PlayerState->GetXP());

// 获取升级奖励
int32 AttributePoints = LevelUpInfo->GetAttributePointReward(CurrentLevel);
int32 SpellPoints = LevelUpInfo->GetSpellPointReward(CurrentLevel);
```

---

### ULootTiers

战利品等级数据资产，定义不同等级的掉落物。

#### 结构

```cpp
UCLASS()
class AURA_API ULootTiers : public UDataAsset
{
    UPROPERTY(EditDefaultsOnly)
    TArray<FLootTier> LootTiers;
    
    TArray<FLootTier> GetLootTiersForLevel(int32 Level) const;
};
```

#### FLootTier

```cpp
USTRUCT(BlueprintType)
struct FLootTier
{
    GENERATED_BODY()
    
    // 掉落物类
    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<AActor> LootClass = nullptr;
    
    // 掉落几率
    UPROPERTY(EditDefaultsOnly)
    float DropChance = 0.f;
    
    // 最小数量
    UPROPERTY(EditDefaultsOnly)
    int32 MinCount = 1;
    
    // 最大数量
    UPROPERTY(EditDefaultsOnly)
    int32 MaxCount = 1;
};
```

---

## 数据资产访问

### UAuraAbilitySystemLibrary

提供静态方法访问数据资产：

```cpp
class AURA_API UAuraAbilitySystemLibrary
{
public:
    // 获取 CharacterClassInfo
    UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|ClassInfo")
    static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);
    
    // 获取 AbilityInfo
    UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|AbilityInfo")
    static UAbilityInfo* GetAbilityInfo(const UObject* WorldContextObject);
    
    // 获取 AttributeInfo
    UFUNCTION(BlueprintPure, Category = "AuraAbilitySystemLibrary|AttributeInfo")
    static UAttributeInfo* GetAttributeInfo(const UObject* WorldContextObject);
};
```

### 实现

```cpp
UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(
    const UObject* WorldContextObject
)
{
    AAuraGameModeBase* AuraGameMode = 
        Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
    
    if (AuraGameMode)
    {
        return AuraGameMode->CharacterClassInfo;
    }
    
    return nullptr;
}
```

---

## 数据资产配置

### 在编辑器中配置

1. **创建数据资产**
   - 右键点击 Content Browser
   - 选择 "Miscellaneous" → "Data Asset"
   - 选择对应的类（如 `CharacterClassInfo`）

2. **配置数据**
   - 打开数据资产
   - 填写各个字段
   - 设置数组和映射

3. **在 GameMode 中设置**
   - 打开 GameMode 蓝图
   - 设置数据资产引用

### 配置示例

**CharacterClassInfo**:
```
CharacterClassInformation:
  Elementalist:
    PrimaryAttributes: GE_PrimaryAttributes_Elementalist
    StartupAbilities:
      - BP_GA_FireBolt
      - BP_GA_Electrocute
    XPReward: (Level 1: 10, Level 2: 15, ...)
  
  Warrior:
    PrimaryAttributes: GE_PrimaryAttributes_Warrior
    StartupAbilities:
      - BP_GA_MeleeAttack
    XPReward: (Level 1: 10, Level 2: 15, ...)
```

---

## 数据驱动优势

1. **无需代码修改** - 调整游戏平衡只需修改数据资产
2. **快速迭代** - 设计师可以直接调整数值
3. **版本控制友好** - 数据资产可以单独版本控制
4. **易于测试** - 可以创建测试用的数据资产

---

## 相关文档

- [角色系统](./Character_System.md) - 角色使用 CharacterClassInfo
- [技能系统](../Core/Ability_System.md) - 技能使用 AbilityInfo
- [属性系统](./Attribute_System.md) - 属性使用 AttributeInfo
- [玩家系统](./Player_System.md) - 玩家使用 LevelUpInfo

---

## 总结

数据资产系统提供了：

1. ✅ **数据驱动配置** - 无需代码修改即可调整游戏
2. ✅ **类型安全** - 使用 C++ 结构确保数据正确性
3. ✅ **易于访问** - 通过静态库函数轻松访问
4. ✅ **编辑器集成** - 完整的编辑器支持
5. ✅ **版本控制** - 数据资产可以单独管理

通过这个系统，可以高效地管理和配置游戏数据。

