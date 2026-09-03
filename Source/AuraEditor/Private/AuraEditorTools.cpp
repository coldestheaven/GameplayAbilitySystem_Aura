// Copyright Druid Mechanics

#include "AuraEditorTools.h"

#include "AbilitySystem/AuraAttributeSet.h"
#include "Actor/AuraEnemySpawnPoint.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

#include "Editor/EditorEngine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

static void EditorNotify(const FString& Message, bool bSuccess)
{
	FNotificationInfo Info(FText::FromString(Message));
	Info.bUseSuccessFailIcons = true;
	Info.ExpireDuration = 3.f;
	if (TSharedPtr<SNotificationItem> Item = FSlateNotificationManager::Get().AddNotification(Info))
	{
		Item->SetCompletionState(bSuccess ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
	}
}

static UWorld* GetPIEWorld()
{
	if (!GEditor) return nullptr;
	const FWorldContext* Context = GEditor->GetPIEWorldContext();
	return Context ? Context->World() : nullptr;
}

static bool IsPIEServer(UWorld* World)
{
	return World && World->GetNetMode() != NM_Client;
}

static APawn* GetLocalPlayerPawn(UWorld* World)
{
	if (!World) return nullptr;
	APlayerController* PC = GEngine->GetFirstLocalPlayerController(World);
	return PC ? PC->GetPawn() : nullptr;
}

// 反射枚举 AttributeSet 的全部 GAS 属性并打印
static int32 DumpAttributeSet(UAbilitySystemComponent* ASC, const FString& OwnerLabel)
{
	// UE5.8: GetAttributeSet 返回 const 指针（只读快照）
	const UAuraAttributeSet* AttributeSet = Cast<UAuraAttributeSet>(ASC->GetAttributeSet(UAuraAttributeSet::StaticClass()));
	if (!AttributeSet) return 0;

	UE_LOG(LogTemp, Log, TEXT("[AuraEditor] -- %s 的属性快照 --"), *OwnerLabel);
	int32 Count = 0;
	for (TFieldIterator<FStructProperty> PropIt(UAuraAttributeSet::StaticClass()); PropIt; ++PropIt)
	{
		FStructProperty* Prop = *PropIt;
		if (Prop->Struct != FGameplayAttributeData::StaticStruct()) continue;
		const FGameplayAttribute Attribute(Prop);
		UE_LOG(LogTemp, Log, TEXT("[AuraEditor]   %-26s = %.1f"), *Prop->GetName(), ASC->GetNumericAttribute(Attribute));
		++Count;
	}
	return Count;
}

namespace AuraEditorTools
{

bool SpawnEnemies(int32 Count)
{
	UWorld* World = GetPIEWorld();
	if (!World) { EditorNotify(TEXT("需要先运行 PIE"), false); return false; }
	if (!IsPIEServer(World)) { EditorNotify(TEXT("需在 PIE 服务器端执行"), false); return false; }

	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsOfClass(World, AAuraEnemySpawnPoint::StaticClass(), SpawnPoints);
	if (SpawnPoints.Num() == 0) { EditorNotify(TEXT("关卡中没有敌人生成点"), false); return false; }

	const int32 NumToSpawn = FMath::Max(1, Count);
	int32 Total = 0;
	for (AActor* Actor : SpawnPoints)
	{
		if (AAuraEnemySpawnPoint* Point = Cast<AAuraEnemySpawnPoint>(Actor))
		{
			for (int32 i = 0; i < NumToSpawn; ++i) { Point->SpawnEnemy(); ++Total; }
		}
	}
	EditorNotify(FString::Printf(TEXT("已在 %d 个生成点刷出 %d 个敌人"), SpawnPoints.Num(), Total), true);
	return true;
}

bool DumpAttributes(bool bAll)
{
	UWorld* World = GetPIEWorld();
	if (!World) { EditorNotify(TEXT("需要先运行 PIE"), false); return false; }

	int32 Dumped = 0;
	if (bAll)
	{
		TArray<AActor*> Pawns;
		UGameplayStatics::GetAllActorsOfClass(World, APawn::StaticClass(), Pawns);
		for (AActor* Actor : Pawns)
		{
			if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Actor))
			{
				Dumped += DumpAttributeSet(ASC, Actor->GetName());
			}
		}
	}
	else
	{
		APawn* PlayerPawn = GetLocalPlayerPawn(World);
		UAbilitySystemComponent* ASC = PlayerPawn ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerPawn) : nullptr;
		if (!ASC) { EditorNotify(TEXT("玩家 Pawn 没有 ASC"), false); return false; }
		Dumped = DumpAttributeSet(ASC, PlayerPawn->GetName());
	}

	EditorNotify(FString::Printf(TEXT("已打印 %d 项属性（见 Output Log）"), Dumped), Dumped > 0);
	return Dumped > 0;
}

bool HealPlayer()
{
	UWorld* World = GetPIEWorld();
	if (!World || !IsPIEServer(World)) { EditorNotify(TEXT("需在 PIE 服务器端执行"), false); return false; }

	APawn* PlayerPawn = GetLocalPlayerPawn(World);
	UAbilitySystemComponent* ASC = PlayerPawn ? UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(PlayerPawn) : nullptr;
	// UE5.8: GetAttributeSet 返回 const；调试工具需写入，故 const_cast（调试豁免场景）
	const UAuraAttributeSet* ConstAttributeSet = ASC ? Cast<UAuraAttributeSet>(ASC->GetAttributeSet(UAuraAttributeSet::StaticClass())) : nullptr;
	if (!ConstAttributeSet) { EditorNotify(TEXT("玩家没有 AuraAttributeSet"), false); return false; }
	UAuraAttributeSet* AttributeSet = const_cast<UAuraAttributeSet*>(ConstAttributeSet);

	// 调试直改（"AttributeSet 必须经 GE 修改"规则的调试豁免场景）：
	// PIE 单进程即服务器权威，直接 Set 正是 GM 工具的意图
	AttributeSet->SetHealth(AttributeSet->GetMaxHealth());
	AttributeSet->SetMana(AttributeSet->GetMaxMana());
	EditorNotify(TEXT("已回满生命/法力"), true);
	return true;
}

} // namespace AuraEditorTools

static FAutoConsoleCommand GAuraEditorSpawnEnemies(
	TEXT("AuraEditor.SpawnEnemies"),
	TEXT("在关卡所有敌人生成点各刷 N 个敌人（默认 1）。用法: AuraEditor.SpawnEnemies 3"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		AuraEditorTools::SpawnEnemies(Args.Num() > 0 ? FCString::Atoi(*Args[0]) : 1);
	}));

static FAutoConsoleCommand GAuraEditorDumpAttributes(
	TEXT("AuraEditor.DumpAttributes"),
	TEXT("打印 GAS 属性快照。用法: AuraEditor.DumpAttributes [all]"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
	{
		AuraEditorTools::DumpAttributes(Args.Num() > 0 && Args[0].Equals(TEXT("all"), ESearchCase::IgnoreCase));
	}));

static FAutoConsoleCommand GAuraEditorHeal(
	TEXT("AuraEditor.Heal"),
	TEXT("回满本地玩家生命/法力（PIE 服务器端）"),
	FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>&)
	{
		AuraEditorTools::HealPlayer();
	}));
