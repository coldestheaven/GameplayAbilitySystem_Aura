// Copyright Druid Mechanics
#include "AbilitySystem/AuraAbilitySystemLibrary.h"

#include "AbilitySystem/AuraGameplayMechanicsLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AuraAbilityTypes.h"
#include "GameplayTags/AuraGameplayTags.h"
#include "Game/AuraGameModeBase.h"
#include "Game/LoadScreenSaveGame.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/OverlapResult.h"

// EffectContext 读写已迁移到 AuraEffectContextLibrary，此处 include 由头文件传递

// ─────────────────────────────────────────────────────────────────────────────
// 内部辅助：将指定 GE 类应用到 ASC（封装重复的 MakeEffectContext + MakeOutgoingSpec + Apply 三步）
// ─────────────────────────────────────────────────────────────────────────────
static void ApplyGEToASC(UAbilitySystemComponent* ASC, TSubclassOf<UGameplayEffect> GEClass, float Level)
{
	check(ASC && GEClass);
	FGameplayEffectContextHandle ContextHandle = ASC->MakeEffectContext();
	ContextHandle.AddSourceObject(ASC->GetAvatarActor());
	const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GEClass, Level, ContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
}

void UAuraAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
	AActor* AvatarActor = ASC->GetAvatarActor();

	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	FCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);

	// 按顺序应用三组属性 GE：主属性 → 次属性（依赖主属性）→ 生命/法力（依赖次属性）
	ApplyGEToASC(ASC, ClassDefaultInfo.PrimaryAttributes, Level);
	ApplyGEToASC(ASC, CharacterClassInfo->SecondaryAttributes, Level);
	ApplyGEToASC(ASC, CharacterClassInfo->VitalAttributes, Level);
}

void UAuraAbilitySystemLibrary::InitializeDefaultAttributesFromSaveData(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ULoadScreenSaveGame* SaveGame)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return;

	const FAuraGameplayTags& GameplayTags = FAuraGameplayTags::Get();
	const AActor* SourceAvatarActor = ASC->GetAvatarActor();

	// 主属性使用 SetByCaller 模式，直接从存档数据设置值
	FGameplayEffectContextHandle EffectContexthandle = ASC->MakeEffectContext();
	EffectContexthandle.AddSourceObject(SourceAvatarActor);
	const FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->PrimaryAttributes_SetByCaller, 1.f, EffectContexthandle);

	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Strength, SaveGame->Strength);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Intelligence, SaveGame->Intelligence);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Resilience, SaveGame->Resilience);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Vigor, SaveGame->Vigor);
	ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

	// 次属性和生命/法力使用辅助函数
	ApplyGEToASC(ASC, CharacterClassInfo->SecondaryAttributes_Infinite, 1.f);
	ApplyGEToASC(ASC, CharacterClassInfo->VitalAttributes, 1.f);
}

void UAuraAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return;
	
	for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->CommonAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
	}
	
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

// ─────────────────────────────────────────────────────────────────────────────
// 以下游戏机制函数均为「兼容转发」：实际实现位于 AuraCore 插件的
// UAuraGameplayMechanicsLibrary（该库为游戏机制的权威实现）。
// 保留此处转发以维持既有 C++/蓝图调用点不变，避免重复维护两份相同逻辑。
//
// 例外：GetXPRewardForClassAndLevel 依赖本模块的 AAuraGameModeBase /
// UCharacterClassInfo（插件不能反向依赖游戏模块），故直接在本模块实现。
// ─────────────────────────────────────────────────────────────────────────────
int32 UAuraAbilitySystemLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel)
{
	UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return 0;

	const FCharacterClassDefaultInfo& Info = CharacterClassInfo->GetClassDefaultInfo(CharacterClass);
	const float XPReward = Info.XPReward.GetValueAtLevel(CharacterLevel);
	return static_cast<int32>(XPReward);
}

void UAuraAbilitySystemLibrary::SetIsRadialDamageEffectParam(FDamageEffectParams& DamageEffectParams, bool bIsRadial, float InnerRadius, float OuterRadius, FVector Origin)
{
	UAuraGameplayMechanicsLibrary::SetIsRadialDamageEffectParam(DamageEffectParams, bIsRadial, InnerRadius, OuterRadius, Origin);
}

void UAuraAbilitySystemLibrary::SetKnockbackDirection(FDamageEffectParams& DamageEffectParams, FVector KnockbackDirection, float Magnitude)
{
	UAuraGameplayMechanicsLibrary::SetKnockbackDirection(DamageEffectParams, KnockbackDirection, Magnitude);
}

void UAuraAbilitySystemLibrary::SetDeathImpulseDirection(FDamageEffectParams& DamageEffectParams, FVector ImpulseDirection, float Magnitude)
{
	UAuraGameplayMechanicsLibrary::SetDeathImpulseDirection(DamageEffectParams, ImpulseDirection, Magnitude);
}

void UAuraAbilitySystemLibrary::SetTargetEffectParamsASC(FDamageEffectParams& DamageEffectParams, UAbilitySystemComponent* InASC)
{
	UAuraGameplayMechanicsLibrary::SetTargetEffectParamsASC(DamageEffectParams, InASC);
}

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

/**
 * 获取 AttributeInfo 数据资产（C1 重构新增 · 2026-06-14）
 *
 * 实现：从当前 World 对应的 AAuraGameModeBase 读取 AttributeInfo 字段
 *
 * 失败处理：
 * - 非 AuraGameMode（如客户端独立测试场景）→ 返回 nullptr
 * - GameMode 上未配置该字段 → 返回 nullptr（蓝图侧可后续补配置）
 *
 * 调用方应自行判空，避免空指针访问
 */
UAttributeInfo* UAuraAbilitySystemLibrary::GetAttributeInfo(const UObject* WorldContextObject)
{
	const AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode == nullptr) return nullptr;
	return AuraGameMode->AttributeInfo;
}

ULootTiers* UAuraAbilitySystemLibrary::GetLootTiers(const UObject* WorldContextObject)
{
	const AAuraGameModeBase* AuraGameMode = Cast<AAuraGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (AuraGameMode == nullptr) return nullptr;
	return AuraGameMode->LootTiers;
}

void UAuraAbilitySystemLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject,
	TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin)
{
	UAuraGameplayMechanicsLibrary::GetLivePlayersWithinRadius(WorldContextObject, OutOverlappingActors, ActorsToIgnore, Radius, SphereOrigin);
}

void UAuraAbilitySystemLibrary::GetClosestTargets(int32 MaxTargets, const TArray<AActor*>& Actors, TArray<AActor*>& OutClosestTargets, const FVector& Origin)
{
	UAuraGameplayMechanicsLibrary::GetClosestTargets(MaxTargets, Actors, OutClosestTargets, Origin);
}

bool UAuraAbilitySystemLibrary::IsNotFriend(AActor* FirstActor, AActor* SecondActor)
{
	return UAuraGameplayMechanicsLibrary::IsNotFriend(FirstActor, SecondActor);
}

FGameplayEffectContextHandle UAuraAbilitySystemLibrary::ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams)
{
	return UAuraGameplayMechanicsLibrary::ApplyDamageEffect(DamageEffectParams);
}

TArray<FRotator> UAuraAbilitySystemLibrary::EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators)
{
	return UAuraGameplayMechanicsLibrary::EvenlySpacedRotators(Forward, Axis, Spread, NumRotators);
}

TArray<FVector> UAuraAbilitySystemLibrary::EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors)
{
	return UAuraGameplayMechanicsLibrary::EvenlyRotatedVectors(Forward, Axis, Spread, NumVectors);
}