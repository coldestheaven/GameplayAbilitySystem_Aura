// Copyright Druid Mechanics

#include "AbilitySystem/AuraEffectContextLibrary.h"
#include "AuraAbilityTypes.h"

// 内部辅助：从 Handle 中安全获取只读 AuraEffectContext 指针
static const FAuraGameplayEffectContext* GetAuraContext(const FGameplayEffectContextHandle& Handle)
{
	return static_cast<const FAuraGameplayEffectContext*>(Handle.Get());
}

// 内部辅助：从 Handle 中安全获取可写 AuraEffectContext 指针
static FAuraGameplayEffectContext* GetMutableAuraContext(FGameplayEffectContextHandle& Handle)
{
	return static_cast<FAuraGameplayEffectContext*>(Handle.Get());
}

bool UAuraEffectContextLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* Ctx = GetAuraContext(EffectContextHandle)) return Ctx->IsBlockedHit();
	return false;
}

bool UAuraEffectContextLibrary::IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* Ctx = GetAuraContext(EffectContextHandle)) return Ctx->IsSuccessfulDebuff();
	return false;
}

float UAuraEffectContextLibrary::GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* Ctx = GetAuraContext(EffectContextHandle)) return Ctx->GetDebuffDamage();
	return 0.f;
}

float UAuraEffectContextLibrary::GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* Ctx = GetAuraContext(EffectContextHandle)) return Ctx->GetDebuffDuration();
	return 0.f;
}

float UAuraEffectContextLibrary::GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* Ctx = GetAuraContext(EffectContextHandle)) return Ctx->GetDebuffFrequency();
	return 0.f;
}

FGameplayTag UAuraEffectContextLibrary::GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* Ctx = GetAuraContext(EffectContextHandle))
	{
		if (Ctx->GetDamageType().IsValid()) return *Ctx->GetDamageType();
	}
	return FGameplayTag();
}

FVector UAuraEffectContextLibrary::GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* Ctx = GetAuraContext(EffectContextHandle)) return Ctx->GetDeathImpulse();
	return FVector::ZeroVector;
}

FVector UAuraEffectContextLibrary::GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* Ctx = GetAuraContext(EffectContextHandle)) return Ctx->GetKnockbackForce();
	return FVector::ZeroVector;
}

bool UAuraEffectContextLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* Ctx = GetAuraContext(EffectContextHandle)) return Ctx->IsCriticalHit();
	return false;
}

bool UAuraEffectContextLibrary::IsRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* Ctx = GetAuraContext(EffectContextHandle)) return Ctx->IsRadialDamage();
	return false;
}

float UAuraEffectContextLibrary::GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* Ctx = GetAuraContext(EffectContextHandle)) return Ctx->GetRadialDamageInnerRadius();
	return 0.f;
}

float UAuraEffectContextLibrary::GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* Ctx = GetAuraContext(EffectContextHandle)) return Ctx->GetRadialDamageOuterRadius();
	return 0.f;
}

FVector UAuraEffectContextLibrary::GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FAuraGameplayEffectContext* Ctx = GetAuraContext(EffectContextHandle)) return Ctx->GetRadialDamageOrigin();
	return FVector::ZeroVector;
}

void UAuraEffectContextLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	if (FAuraGameplayEffectContext* Ctx = GetMutableAuraContext(EffectContextHandle)) Ctx->SetIsBlockedHit(bInIsBlockedHit);
}

void UAuraEffectContextLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit)
{
	if (FAuraGameplayEffectContext* Ctx = GetMutableAuraContext(EffectContextHandle)) Ctx->SetIsCriticalHit(bInIsCriticalHit);
}

void UAuraEffectContextLibrary::SetIsSuccessfulDebuff(FGameplayEffectContextHandle& EffectContextHandle, bool bInSuccessfulDebuff)
{
	if (FAuraGameplayEffectContext* Ctx = GetMutableAuraContext(EffectContextHandle)) Ctx->SetIsSuccessfulDebuff(bInSuccessfulDebuff);
}

void UAuraEffectContextLibrary::SetDebuffDamage(FGameplayEffectContextHandle& EffectContextHandle, float InDamage)
{
	if (FAuraGameplayEffectContext* Ctx = GetMutableAuraContext(EffectContextHandle)) Ctx->SetDebuffDamage(InDamage);
}

void UAuraEffectContextLibrary::SetDebuffDuration(FGameplayEffectContextHandle& EffectContextHandle, float InDuration)
{
	if (FAuraGameplayEffectContext* Ctx = GetMutableAuraContext(EffectContextHandle)) Ctx->SetDebuffDuration(InDuration);
}

void UAuraEffectContextLibrary::SetDebuffFrequency(FGameplayEffectContextHandle& EffectContextHandle, float InFrequency)
{
	if (FAuraGameplayEffectContext* Ctx = GetMutableAuraContext(EffectContextHandle)) Ctx->SetDebuffFrequency(InFrequency);
}

void UAuraEffectContextLibrary::SetDamageType(FGameplayEffectContextHandle& EffectContextHandle, const FGameplayTag& InDamageType)
{
	if (FAuraGameplayEffectContext* Ctx = GetMutableAuraContext(EffectContextHandle))
	{
		Ctx->SetDamageType(MakeShared<FGameplayTag>(InDamageType));
	}
}

void UAuraEffectContextLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectContextHandle, const FVector& InImpulse)
{
	if (FAuraGameplayEffectContext* Ctx = GetMutableAuraContext(EffectContextHandle)) Ctx->SetDeathImpulse(InImpulse);
}

void UAuraEffectContextLibrary::SetKnockbackForce(FGameplayEffectContextHandle& EffectContextHandle, const FVector& InForce)
{
	if (FAuraGameplayEffectContext* Ctx = GetMutableAuraContext(EffectContextHandle)) Ctx->SetKnockbackForce(InForce);
}

void UAuraEffectContextLibrary::SetIsRadialDamage(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsRadialDamage)
{
	if (FAuraGameplayEffectContext* Ctx = GetMutableAuraContext(EffectContextHandle)) Ctx->SetIsRadialDamage(bInIsRadialDamage);
}

void UAuraEffectContextLibrary::SetRadialDamageInnerRadius(FGameplayEffectContextHandle& EffectContextHandle, float InInnerRadius)
{
	if (FAuraGameplayEffectContext* Ctx = GetMutableAuraContext(EffectContextHandle)) Ctx->SetRadialDamageInnerRadius(InInnerRadius);
}

void UAuraEffectContextLibrary::SetRadialDamageOuterRadius(FGameplayEffectContextHandle& EffectContextHandle, float InOuterRadius)
{
	if (FAuraGameplayEffectContext* Ctx = GetMutableAuraContext(EffectContextHandle)) Ctx->SetRadialDamageOuterRadius(InOuterRadius);
}

void UAuraEffectContextLibrary::SetRadialDamageOrigin(FGameplayEffectContextHandle& EffectContextHandle, const FVector& InOrigin)
{
	if (FAuraGameplayEffectContext* Ctx = GetMutableAuraContext(EffectContextHandle)) Ctx->SetRadialDamageOrigin(InOrigin);
}
