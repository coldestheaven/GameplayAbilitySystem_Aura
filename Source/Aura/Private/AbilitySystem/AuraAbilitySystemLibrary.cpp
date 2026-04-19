// Copyright Druid Mechanics
#include "AbilitySystem/AuraAbilitySystemLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AuraAbilityTypes.h"
#include "GameplayTags/AuraGameplayTags.h"
#include "Game/AuraGameModeBase.h"
#include "Game/LoadScreenSaveGame.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Player/AuraPlayerState.h"
#include "Engine/OverlapResult.h"
#include "UI/HUD/AuraHUD.h"
#include "UI/WidgetController/AuraWidgetController.h"
#include "AbilitySystem/AuraEffectContextLibrary.h"

/**
 * 创建 WidgetController 参数结构体
 * 
 * 实现流程：
 * 1. 获取玩家控制器（索引 0）
 * 2. 从控制器获取 HUD 并转换为 AuraHUD
 * 3. 从 PlayerState 获取 ASC 和 AttributeSet
 * 4. 填充 WidgetControllerParams 结构体
 * 
 * @param WorldContextObject 世界上下文对象（用于获取 GameInstance）
 * @param OutWCParams 输出的 WidgetController 参数结构体
 * @param OutAuraHUD 输出的 AuraHUD 指针
 * @return true 表示成功创建参数，false 表示失败
 * 
 * 使用场景：
 * - 创建 WidgetController 时获取必要的参数
 * - 由其他 GetXXXWidgetController 函数调用
 */
bool UAuraAbilitySystemLibrary::MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, AAuraHUD*& OutAuraHUD)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		OutAuraHUD = Cast<AAuraHUD>(PC->GetHUD());
		if (OutAuraHUD)
		{
			AAuraPlayerState* PS = PC->GetPlayerState<AAuraPlayerState>();
			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
			UAttributeSet* AS = PS->GetAttributeSet();

			OutWCParams.AttributeSet = AS;
			OutWCParams.AbilitySystemComponent = ASC;
			OutWCParams.PlayerState = PS;
			OutWCParams.PlayerController = PC;
			return true;
		}
	}
	return false;
}

/**
 * 获取 Overlay WidgetController（覆盖层 UI 控制器）
 * 
 * 实现流程：
 * 1. 创建 WidgetControllerParams
 * 2. 调用 MakeWidgetControllerParams 获取参数
 * 3. 从 AuraHUD 获取或创建 OverlayWidgetController
 * 
 * @param WorldContextObject 世界上下文对象
 * @return OverlayWidgetController 指针，失败返回 nullptr
 * 
 * 使用场景：
 * - 需要访问覆盖层 UI（血条、法力条、经验条等）时
 * - 在 Widget 中调用，获取对应的 WidgetController
 */
UOverlayWidgetController* UAuraAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	AAuraHUD* AuraHUD = nullptr;
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, AuraHUD))
	{
		return AuraHUD->GetOverlayWidgetController(WCParams);
	}
	return nullptr;
}

/**
 * 获取 AttributeMenu WidgetController（属性菜单控制器）
 * 
 * 实现流程：
 * 1. 创建 WidgetControllerParams
 * 2. 调用 MakeWidgetControllerParams 获取参数
 * 3. 从 AuraHUD 获取或创建 AttributeMenuWidgetController
 * 
 * @param WorldContextObject 世界上下文对象
 * @return AttributeMenuWidgetController 指针，失败返回 nullptr
 * 
 * 使用场景：
 * - 打开属性菜单时获取控制器
 * - 在属性菜单 Widget 中调用
 */
UAttributeMenuWidgetController* UAuraAbilitySystemLibrary::GetAttributeMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	AAuraHUD* AuraHUD = nullptr;
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, AuraHUD))
	{
		return AuraHUD->GetAttributeMenuWidgetController(WCParams);
	}
	return nullptr;
}

/**
 * 获取 SpellMenu WidgetController（技能菜单控制器）
 * 
 * 实现流程：
 * 1. 创建 WidgetControllerParams
 * 2. 调用 MakeWidgetControllerParams 获取参数
 * 3. 从 AuraHUD 获取或创建 SpellMenuWidgetController
 * 
 * @param WorldContextObject 世界上下文对象
 * @return SpellMenuWidgetController 指针，失败返回 nullptr
 * 
 * 使用场景：
 * - 打开技能菜单时获取控制器
 * - 在技能菜单 Widget 中调用
 */
USpellMenuWidgetController* UAuraAbilitySystemLibrary::GetSpellMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	AAuraHUD* AuraHUD = nullptr;
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, AuraHUD))
	{
		return AuraHUD->GetSpellMenuWidgetController(WCParams);
	}
	return nullptr;
}

/**
 * 初始化角色的默认属性（首次加载时使用）
 * 
 * 实现流程：
 * 1. 获取 AvatarActor 和 CharacterClassInfo
 * 2. 获取职业的默认信息
 * 3. 按顺序应用三组属性 GE：
 *    - PrimaryAttributes（主属性：力量、智力、韧性、活力）
 *    - SecondaryAttributes（次属性：护甲、暴击率等）
 *    - VitalAttributes（生命值、法力值设为最大值）
 * 
 * @param WorldContextObject 世界上下文对象
 * @param CharacterClass 角色职业类型
 * @param Level 角色等级（影响属性值）
 * @param ASC 要初始化属性的 AbilitySystemComponent
 * 
 * 使用场景：
 * - 首次加载时初始化角色属性
 * - 在 AuraCharacterBase::InitializeDefaultAttributes 中调用
 * 
 * 注意：
 * - 应用顺序很重要：主属性 → 次属性（依赖主属性）→ 生命/法力（依赖次属性）
 * - 使用 Level 参数影响属性值（通过 GE 的 ScalableFloat 曲线）
 */
void UAuraAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
	AActor* AvatarActor = ASC->GetAvatarActor();

	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	FCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	// 应用主属性 GE
	FGameplayEffectContextHandle PrimaryAttributesContextHandle = ASC->MakeEffectContext();
	PrimaryAttributesContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle PrimaryAttributesSpecHandle = ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, PrimaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*PrimaryAttributesSpecHandle.Data.Get());

	// 应用次属性 GE
	FGameplayEffectContextHandle SecondaryAttributesContextHandle = ASC->MakeEffectContext();
	SecondaryAttributesContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle SecondaryAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes, Level, SecondaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data.Get());

	// 应用生命/法力 GE（设为最大值）
	FGameplayEffectContextHandle VitalAttributesContextHandle = ASC->MakeEffectContext();
	VitalAttributesContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle VitalAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, Level, VitalAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributesSpecHandle.Data.Get());
}

/**
 * 从存档数据初始化角色属性（读档时使用）
 * 
 * 实现流程：
 * 1. 获取 CharacterClassInfo
 * 2. 创建主属性 GE Spec（使用 SetByCaller 模式）
 * 3. 使用 SetByCaller 设置每个主属性的值（从存档读取）：
 *    - 力量、智力、韧性、活力
 * 4. 应用主属性 GE
 * 5. 应用次属性 GE（使用 Infinite 持续时间，自动计算）
 * 6. 应用生命/法力 GE（设为最大值）
 * 
 * @param WorldContextObject 世界上下文对象
 * @param ASC 要初始化属性的 AbilitySystemComponent
 * @param SaveGame 存档数据对象（包含保存的主属性值）
 * 
 * 使用场景：
 * - 从存档加载时恢复角色属性
 * - 在 AuraCharacter::LoadProgress 中调用
 * 
 * 注意：
 * - 主属性使用 SetByCaller 模式，直接从存档数据设置值
 * - 次属性使用 Infinite GE，会根据主属性自动计算
 * - 与 InitializeDefaultAttributes 的区别：主属性值来自存档而非默认值
 */
void UAuraAbilitySystemLibrary::InitializeDefaultAttributesFromSaveData(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ULoadScreenSaveGame* SaveGame)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return;

	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();

	const AActor* SourceAvatarActor = ASC->GetAvatarActor();

	// 创建主属性 GE Spec（使用 SetByCaller 模式）
	FGameplayEffectContextHandle EffectContexthandle = ASC->MakeEffectContext();
	EffectContexthandle.AddSourceObject(SourceAvatarActor);

	const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->PrimaryAttributes_SetByCaller, 1.f, EffectContexthandle);

	// 使用 SetByCaller 设置每个主属性的值（从存档读取）
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Strength, SaveGame->Strength);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Intelligence, SaveGame->Intelligence);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Resilience, SaveGame->Resilience);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Vigor, SaveGame->Vigor);

	ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

	// 应用次属性 GE（Infinite 持续时间，自动根据主属性计算）
	FGameplayEffectContextHandle SecondaryAttributesContextHandle = ASC->MakeEffectContext();
	SecondaryAttributesContextHandle.AddSourceObject(SourceAvatarActor);
	const FGameplayEffectSpecHandle SecondaryAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes_Infinite, 1.f, SecondaryAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttributesSpecHandle.Data.Get());

	// 应用生命/法力 GE（设为最大值）
	FGameplayEffectContextHandle VitalAttributesContextHandle = ASC->MakeEffectContext();
	VitalAttributesContextHandle.AddSourceObject(SourceAvatarActor);
	const FGameplayEffectSpecHandle VitalAttributesSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, 1.f, VitalAttributesContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttributesSpecHandle.Data.Get());
}

/**
 * 赋予角色初始技能（通用技能 + 职业专属技能）
 * 
 * 实现流程：
 * 1. 获取 CharacterClassInfo
 * 2. 赋予通用技能（所有职业共享）：
 *    - 遍历 CommonAbilities，创建 AbilitySpec（等级为 1）
 *    - 赋予技能
 * 3. 赋予职业专属技能：
 *    - 获取职业的默认信息
 *    - 遍历 StartupAbilities，创建 AbilitySpec（等级为角色当前等级）
 *    - 赋予技能
 * 
 * @param WorldContextObject 世界上下文对象
 * @param ASC 要赋予技能的 AbilitySystemComponent
 * @param CharacterClass 角色职业类型
 * 
 * 使用场景：
 * - 敌人角色初始化时赋予技能
 * - 在 AuraEnemy::BeginPlay 中调用
 * 
 * 注意：
 * - 通用技能等级固定为 1
 * - 职业专属技能等级为角色当前等级（通过 CombatInterface 获取）
 */
void UAuraAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return;
	
	// 赋予通用技能（所有职业共享）
	for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
	}
	
	// 赋予职业专属技能
	const FCharacterClassDefaultInfo& DefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultInfo.StartupAbilities)
	{
		if (ASC->GetAvatarActor()->Implements<UCombatInterface>())
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, ICombatInterface::Execute_GetPlayerLevel(ASC->GetAvatarActor()));
			ASC->GiveAbility(AbilitySpec);
		}
	}
}

/**
 * 根据职业和等级获取 XP 奖励
 * 
 * 实现流程：
 * 1. 获取 CharacterClassInfo
 * 2. 获取职业的默认信息
 * 3. 从 XPReward 曲线中根据等级获取 XP 值
 * 4. 转换为整数并返回
 * 
 * @param WorldContextObject 世界上下文对象
 * @param CharacterClass 角色职业类型
 * @param CharacterLevel 角色等级
 * @return XP 奖励值（整数）
 * 
 * 使用场景：
 * - 击杀敌人后计算 XP 奖励
 * - 在 AttributeSet::HandleIncomingXP 中调用
 * 
 * 注意：
 * - XP 奖励通过曲线配置，不同等级奖励不同
 * - 不同职业的 XP 奖励曲线可能不同
 */
int32 UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return 0;

	const FCharacterClassDefaultInfo& Info = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	const float XPReward = Info.XPReward.GetValueAtLevel(CharacterLevel);

	return static_cast<int32>(XPReward);
}

/**
 * 设置范围伤害参数
 * 
 * @param DamageEffectParams 伤害效果参数结构体（引用传递，直接修改）
 * @param bIsRadial 是否为范围伤害
 * @param InnerRadius 内圈半径（满伤害区域）
 * @param OuterRadius 外圈半径（伤害衰减到 0）
 * @param Origin 伤害原点（范围伤害的中心点）
 * 
 * 使用场景：
 * - 配置范围技能（如爆炸、AOE）的伤害参数
 */
void UAuraAbilitySystemLibrary::SetIsRadialDamageEffectParam(FDamageEffectParams& DamageEffectParams, bool bIsRadial, float InnerRadius, float OuterRadius, FVector Origin)
{
	DamageEffectParams.bIsRadialDamage = bIsRadial;
	DamageEffectParams.RadialDamageInnerRadius = InnerRadius;
	DamageEffectParams.RadialDamageOuterRadius = OuterRadius;
	DamageEffectParams.RadialDamageOrigin = Origin;
}

/**
 * 设置击退方向
 * 
 * 实现流程：
 * 1. 归一化击退方向向量
 * 2. 如果 Magnitude 为 0，使用 DamageEffectParams 中的默认击退力度
 * 3. 否则使用指定的 Magnitude
 * 
 * @param DamageEffectParams 伤害效果参数结构体
 * @param KnockbackDirection 击退方向（会被归一化）
 * @param Magnitude 击退力度（0 表示使用默认值）
 * 
 * 使用场景：
 * - 配置技能造成的击退效果
 */
void UAuraAbilitySystemLibrary::SetKnockbackDirection(FDamageEffectParams& DamageEffectParams, FVector KnockbackDirection, float Magnitude)
{
	KnockbackDirection.Normalize();
	if (Magnitude == 0.f)
	{
		DamageEffectParams.KnockbackForce = KnockbackDirection * DamageEffectParams.KnockbackForceMagnitude;
	}
	else
	{
		DamageEffectParams.KnockbackForce = KnockbackDirection * Magnitude;
	}
}

/**
 * 设置死亡冲量方向
 * 
 * 实现流程：
 * 1. 归一化冲量方向向量
 * 2. 如果 Magnitude 为 0，使用 DamageEffectParams 中的默认冲量力度
 * 3. 否则使用指定的 Magnitude
 * 
 * @param DamageEffectParams 伤害效果参数结构体
 * @param ImpulseDirection 死亡冲量方向（会被归一化）
 * @param Magnitude 冲量力度（0 表示使用默认值）
 * 
 * 使用场景：
 * - 配置角色死亡时的布娃娃物理击飞方向
 */
void UAuraAbilitySystemLibrary::SetDeathImpulseDirection(FDamageEffectParams& DamageEffectParams, FVector ImpulseDirection, float Magnitude)
{
	ImpulseDirection.Normalize();
	if (Magnitude == 0.f)
	{
		DamageEffectParams.DeathImpulse = ImpulseDirection * DamageEffectParams.DeathImpulseMagnitude;
	}
	else
	{
		DamageEffectParams.DeathImpulse = ImpulseDirection * Magnitude;
	}
}

/**
 * 设置目标 ASC（用于伤害应用）
 * 
 * @param DamageEffectParams 伤害效果参数结构体
 * @param InASC 目标的 AbilitySystemComponent
 * 
 * 使用场景：
 * - 配置伤害效果的目标 ASC
 */
void UAuraAbilitySystemLibrary::SetTargetEffectParamsASC(FDamageEffectParams& DamageEffectParams,
	UAbilitySystemComponent* InASC)
{
	DamageEffectParams.TargetAbilitySystemComponent = InASC;
}

/**
 * 获取 CharacterClassInfo 数据资产
 * 
 * 实现流程：
 * 1. 获取 GameMode 并转换为 AuraGameModeBase
 * 2. 返回 GameMode 中的 CharacterClassInfo
 * 
 * @param WorldContextObject 世界上下文对象
 * @return CharacterClassInfo 指针，失败返回 nullptr
 * 
 * 使用场景：
 * - 需要访问角色职业配置时调用
 * - CharacterClassInfo 包含所有职业的初始属性、技能、XP 奖励等配置
 */
UCharacterClassInfo* UAuraAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	const AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode == nullptr) return nullptr;
	return AuraGameMode->CharacterClassInfo;
}

UAbilityInfo* UAuraAbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContextObject)
{
	const AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode == nullptr) return nullptr;
	return AuraGameMode->AbilityInfo;
}

ULootTiers* UAuraAbilitySystemLibrary::GetLootTiers(const UObject* WorldContextObject)
{
	const AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode == nullptr) return nullptr;
	return AuraGameMode->LootTiers;
}

bool UAuraAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	return UAuraEffectContextLibrary::IsBlockedHit(EffectContextHandle);
}

bool UAuraAbilitySystemLibrary::IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	return UAuraEffectContextLibrary::IsSuccessfulDebuff(EffectContextHandle);
}

float UAuraAbilitySystemLibrary::GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	return UAuraEffectContextLibrary::GetDebuffDamage(EffectContextHandle);
}

float UAuraAbilitySystemLibrary::GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
{
	return UAuraEffectContextLibrary::GetDebuffDuration(EffectContextHandle);
}

float UAuraAbilitySystemLibrary::GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
{
	return UAuraEffectContextLibrary::GetDebuffFrequency(EffectContextHandle);
}

FGameplayTag UAuraAbilitySystemLibrary::GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle)
{
	return UAuraEffectContextLibrary::GetDamageType(EffectContextHandle);
}

FVector UAuraAbilitySystemLibrary::GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle)
{
	return UAuraEffectContextLibrary::GetDeathImpulse(EffectContextHandle);
}

FVector UAuraAbilitySystemLibrary::GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle)
{
	return UAuraEffectContextLibrary::GetKnockbackForce(EffectContextHandle);
}

bool UAuraAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	return UAuraEffectContextLibrary::IsCriticalHit(EffectContextHandle);
}

bool UAuraAbilitySystemLibrary::IsRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	return UAuraEffectContextLibrary::IsRadialDamage(EffectContextHandle);
}

float UAuraAbilitySystemLibrary::GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	return UAuraEffectContextLibrary::GetRadialDamageInnerRadius(EffectContextHandle);
}

float UAuraAbilitySystemLibrary::GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	return UAuraEffectContextLibrary::GetRadialDamageOuterRadius(EffectContextHandle);
}

FVector UAuraAbilitySystemLibrary::GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectContextHandle)
{
	return UAuraEffectContextLibrary::GetRadialDamageOrigin(EffectContextHandle);
}

void UAuraAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	UAuraEffectContextLibrary::SetIsBlockedHit(EffectContextHandle, bInIsBlockedHit);
}

void UAuraAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInIsCriticalHit)
{
	UAuraEffectContextLibrary::SetIsCriticalHit(EffectContextHandle, bInIsCriticalHit);
}

void UAuraAbilitySystemLibrary::SetIsSuccessfulDebuff(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInSuccessfulDebuff)
{
	UAuraEffectContextLibrary::SetIsSuccessfulDebuff(EffectContextHandle, bInSuccessfulDebuff);
}

void UAuraAbilitySystemLibrary::SetDebuffDamage(FGameplayEffectContextHandle& EffectContextHandle, float InDamage)
{
	UAuraEffectContextLibrary::SetDebuffDamage(EffectContextHandle, InDamage);
}

void UAuraAbilitySystemLibrary::SetDebuffDuration(FGameplayEffectContextHandle& EffectContextHandle, float InDuration)
{
	UAuraEffectContextLibrary::SetDebuffDuration(EffectContextHandle, InDuration);
}

void UAuraAbilitySystemLibrary::SetDebuffFrequency(FGameplayEffectContextHandle& EffectContextHandle, float InFrequency)
{
	UAuraEffectContextLibrary::SetDebuffFrequency(EffectContextHandle, InFrequency);
}

void UAuraAbilitySystemLibrary::SetDamageType(FGameplayEffectContextHandle& EffectContextHandle,
	const FGameplayTag& InDamageType)
{
	UAuraEffectContextLibrary::SetDamageType(EffectContextHandle, InDamageType);
}

void UAuraAbilitySystemLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectContextHandle,
	const FVector& InImpulse)
{
	UAuraEffectContextLibrary::SetDeathImpulse(EffectContextHandle, InImpulse);
}

void UAuraAbilitySystemLibrary::SetKnockbackForce(FGameplayEffectContextHandle& EffectContextHandle,
	const FVector& InForce)
{
	UAuraEffectContextLibrary::SetKnockbackForce(EffectContextHandle, InForce);
}

void UAuraAbilitySystemLibrary::SetIsRadialDamage(FGameplayEffectContextHandle& EffectContextHandle,
	bool bInIsRadialDamage)
{
	UAuraEffectContextLibrary::SetIsRadialDamage(EffectContextHandle, bInIsRadialDamage);
}

void UAuraAbilitySystemLibrary::SetRadialDamageInnerRadius(FGameplayEffectContextHandle& EffectContextHandle,
	float InInnerRadius)
{
	UAuraEffectContextLibrary::SetRadialDamageInnerRadius(EffectContextHandle, InInnerRadius);
}

void UAuraAbilitySystemLibrary::SetRadialDamageOuterRadius(FGameplayEffectContextHandle& EffectContextHandle,
	float InOuterRadius)
{
	UAuraEffectContextLibrary::SetRadialDamageOuterRadius(EffectContextHandle, InOuterRadius);
}

void UAuraAbilitySystemLibrary::SetRadialDamageOrigin(FGameplayEffectContextHandle& EffectContextHandle,
	const FVector& InOrigin)
{
	UAuraEffectContextLibrary::SetRadialDamageOrigin(EffectContextHandle, InOrigin);
}

void UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject,
                                                           TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius,
                                                           const FVector& SphereOrigin)
{
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);
	
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		TArray<FOverlapResult> Overlaps;
		World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(Radius), SphereParams);
		for (FOverlapResult& Overlap : Overlaps)
		{
			if (Overlap.GetActor()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(Overlap.GetActor()))
			{
				OutOverlappingActors.AddUnique(ICombatInterface::Execute_GetAvatar(Overlap.GetActor()));
			}
		}
	}
}

void UAuraAbilitySystemLibrary::GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets, const FVector& Origin)
{
	if (Actors.Num() <= MaxTargets)
	{
		OutClosestTargets = Actors;
		return;
	}

	TArray<AActor*> ActorsToCheck = Actors;
	int32 NumTargetsFound = 0;

	while (NumTargetsFound < MaxTargets)
	{
		if (ActorsToCheck.Num() == 0) break;
		double ClosestDistance = TNumericLimits<double>::Max();
		AActor* ClosestActor = nullptr;
		for (AActor* PotentialTarget : ActorsToCheck)
		{
			const double Distance = (PotentialTarget->GetActorLocation() - Origin).Length();
			if (Distance < ClosestDistance)
			{
				ClosestDistance = Distance;
				ClosestActor = PotentialTarget;
			}
		}
		ActorsToCheck.Remove(ClosestActor);
		OutClosestTargets.AddUnique(ClosestActor);
		++NumTargetsFound;
	}
}

bool UAuraAbilitySystemLibrary::IsNotFriend(AActor* FirstActor, AActor* SecondActor)
{
	const bool bBothArePlayers = FirstActor->ActorHasTag(FName("Player")) && SecondActor->ActorHasTag(FName("Player"));
	const bool bBothAreEnemies = FirstActor->ActorHasTag(FName("Enemy")) && SecondActor->ActorHasTag(FName("Enemy"));
	const bool bFriends = bBothArePlayers || bBothAreEnemies;
	return !bFriends;
}

FGameplayEffectContextHandle UAuraAbilitySystemLibrary::ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams)
{
	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	const AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();
	
	FGameplayEffectContextHandle EffectContexthandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();
	EffectContexthandle.AddSourceObject(SourceAvatarActor);
	UAuraEffectContextLibrary::SetDeathImpulse(EffectContexthandle, DamageEffectParams.DeathImpulse);
	UAuraEffectContextLibrary::SetKnockbackForce(EffectContexthandle, DamageEffectParams.KnockbackForce);

	UAuraEffectContextLibrary::SetIsRadialDamage(EffectContexthandle, DamageEffectParams.bIsRadialDamage);
	UAuraEffectContextLibrary::SetRadialDamageInnerRadius(EffectContexthandle, DamageEffectParams.RadialDamageInnerRadius);
	UAuraEffectContextLibrary::SetRadialDamageOuterRadius(EffectContexthandle, DamageEffectParams.RadialDamageOuterRadius);
	UAuraEffectContextLibrary::SetRadialDamageOrigin(EffectContexthandle, DamageEffectParams.RadialDamageOrigin);
	
	const FGameplayEffectSpecHandle SpecHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeOutgoingSpec(DamageEffectParams.DamageGameplayEffectClass, DamageEffectParams.AbilityLevel, EffectContexthandle);

	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageEffectParams.DamageType, DamageEffectParams.BaseDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Chance, DamageEffectParams.DebuffChance);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Damage, DamageEffectParams.DebuffDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Duration, DamageEffectParams.DebuffDuration);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Frequency, DamageEffectParams.DebuffFrequency);
	
	DamageEffectParams.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
	return EffectContexthandle;
}

TArray<FRotator> UAuraAbilitySystemLibrary::EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators)
{
	TArray<FRotator> Rotators;
	
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);
	if (NumRotators > 1)
	{
		const float DeltaSpread = Spread / (NumRotators - 1);
		for (int32 i = 0; i < NumRotators; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
			Rotators.Add(Direction.Rotation());
		}
	}
	else
	{
		Rotators.Add(Forward.Rotation());
	}
	return Rotators;
}

TArray<FVector> UAuraAbilitySystemLibrary::EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors)
{
	TArray<FVector> Vectors;
	
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);
	if (NumVectors > 1)
	{
		const float DeltaSpread = Spread / (NumVectors - 1);
		for (int32 i = 0; i < NumVectors; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
			Vectors.Add(Direction);
		}
	}
	else
	{
		Vectors.Add(Forward);
	}
	return Vectors;
}
