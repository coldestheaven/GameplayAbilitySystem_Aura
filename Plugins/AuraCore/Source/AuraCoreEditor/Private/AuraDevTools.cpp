// Copyright Druid Mechanics

#include "AuraDevTools.h"

#include "Interaction/PlayerInterface.h"

#include "Editor/EditorEngine.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"

#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

IPlayerInterface* AuraDevTools::GetPIEPlayerInterface(UObject** OutObject)
{
	if (!GEditor)
	{
		return nullptr;
	}

	// 取 PIE 世界上下文（仅在"运行中"有效）
	const FWorldContext* PIEContext = GEditor->GetPIEWorldContext();
	if (!PIEContext || !PIEContext->World())
	{
		return nullptr;
	}

	APlayerController* PC = GEngine->GetFirstLocalPlayerController(PIEContext->World());
	if (!PC)
	{
		return nullptr;
	}

	// 玩家 Pawn 实现了 IPlayerInterface（AAuraCharacter 侧）
	APawn* PlayerPawn = PC->GetPawn();
	if (!PlayerPawn || !PlayerPawn->Implements<UPlayerInterface>())
	{
		return nullptr;
	}

	if (OutObject)
	{
		*OutObject = PlayerPawn;
	}
	return Cast<IPlayerInterface>(PlayerPawn);
}

bool AuraDevTools::GiveXP(int32 Amount)
{
	UObject* Obj = nullptr;
	if (GetPIEPlayerInterface(&Obj))
	{
		IPlayerInterface::Execute_AddToXP(Obj, Amount);
		Notify(FString::Printf(TEXT("已增加 %d XP"), Amount), true);
		return true;
	}
	Notify(TEXT("需要先运行 PIE 且存在玩家"), false);
	return false;
}

bool AuraDevTools::GiveAttributePoints(int32 Amount)
{
	UObject* Obj = nullptr;
	if (GetPIEPlayerInterface(&Obj))
	{
		IPlayerInterface::Execute_AddToAttributePoints(Obj, Amount);
		Notify(FString::Printf(TEXT("已增加 %d 属性点"), Amount), true);
		return true;
	}
	Notify(TEXT("需要先运行 PIE 且存在玩家"), false);
	return false;
}

bool AuraDevTools::GiveSpellPoints(int32 Amount)
{
	UObject* Obj = nullptr;
	if (GetPIEPlayerInterface(&Obj))
	{
		IPlayerInterface::Execute_AddToSpellPoints(Obj, Amount);
		Notify(FString::Printf(TEXT("已增加 %d 技能点"), Amount), true);
		return true;
	}
	Notify(TEXT("需要先运行 PIE 且存在玩家"), false);
	return false;
}

bool AuraDevTools::AddLevels(int32 Amount)
{
	UObject* Obj = nullptr;
	if (GetPIEPlayerInterface(&Obj))
	{
		IPlayerInterface::Execute_AddToPlayerLevel(Obj, Amount);
		Notify(FString::Printf(TEXT("已提升 %d 级"), Amount), true);
		return true;
	}
	Notify(TEXT("需要先运行 PIE 且存在玩家"), false);
	return false;
}

bool AuraDevTools::PrintPlayerReport()
{
	UObject* Obj = nullptr;
	if (!GetPIEPlayerInterface(&Obj))
	{
		Notify(TEXT("需要先运行 PIE 且存在玩家"), false);
		return false;
	}

	const int32 XP = IPlayerInterface::Execute_GetXP(Obj);
	const int32 Level = IPlayerInterface::Execute_FindLevelForXP(Obj, XP);
	const int32 AttrPoints = IPlayerInterface::Execute_GetAttributePoints(Obj);
	const int32 SpellPoints = IPlayerInterface::Execute_GetSpellPoints(Obj);

	UE_LOG(LogTemp, Log, TEXT("[AuraDev] 玩家状态: XP=%d | 等级=%d | 属性点=%d | 技能点=%d"),
		XP, Level, AttrPoints, SpellPoints);
	Notify(FString::Printf(TEXT("XP=%d 等级=%d 属性点=%d 技能点=%d（详情见 Output Log）"),
		XP, Level, AttrPoints, SpellPoints), true);
	return true;
}

void AuraDevTools::Notify(const FString& Message, bool bSuccess)
{
	FNotificationInfo Info(FText::FromString(Message));
	Info.bUseSuccessFailIcons = true;
	Info.ExpireDuration = 3.f;

	if (TSharedPtr<SNotificationItem> Item = FSlateNotificationManager::Get().AddNotification(Info))
	{
		Item->SetCompletionState(bSuccess ? SNotificationItem::CS_Success : SNotificationItem::CS_Fail);
	}
}

/* ── 控制台命令处理器 ── */

void AuraDevTools::CmdGiveXP(const TArray<FString>& Args)
{
	GiveXP(Args.Num() > 0 ? FCString::Atoi(*Args[0]) : 1);
}

void AuraDevTools::CmdGiveAttributePoints(const TArray<FString>& Args)
{
	GiveAttributePoints(Args.Num() > 0 ? FCString::Atoi(*Args[0]) : 1);
}

void AuraDevTools::CmdGiveSpellPoints(const TArray<FString>& Args)
{
	GiveSpellPoints(Args.Num() > 0 ? FCString::Atoi(*Args[0]) : 1);
}

void AuraDevTools::CmdAddLevels(const TArray<FString>& Args)
{
	AddLevels(Args.Num() > 0 ? FCString::Atoi(*Args[0]) : 1);
}

void AuraDevTools::CmdPrintPlayerReport(const TArray<FString>& Args)
{
	PrintPlayerReport();
}

/* ── 控制台命令注册（模块加载即注册） ── */

static FAutoConsoleCommand GAuraDevGiveXP(
	TEXT("AuraDev.GiveXP"),
	TEXT("给 PIE 玩家增加 XP。用法: AuraDev.GiveXP 100"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&AuraDevTools::CmdGiveXP));

static FAutoConsoleCommand GAuraDevGiveAttrPoints(
	TEXT("AuraDev.GiveAttrPoints"),
	TEXT("给 PIE 玩家增加属性点。用法: AuraDev.GiveAttrPoints 10"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&AuraDevTools::CmdGiveAttributePoints));

static FAutoConsoleCommand GAuraDevGiveSpellPoints(
	TEXT("AuraDev.GiveSpellPoints"),
	TEXT("给 PIE 玩家增加技能点。用法: AuraDev.GiveSpellPoints 10"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&AuraDevTools::CmdGiveSpellPoints));

static FAutoConsoleCommand GAuraDevLevelUp(
	TEXT("AuraDev.LevelUp"),
	TEXT("提升 PIE 玩家等级。用法: AuraDev.LevelUp 1"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&AuraDevTools::CmdAddLevels));

static FAutoConsoleCommand GAuraDevPlayerStats(
	TEXT("AuraDev.PlayerStats"),
	TEXT("打印 PIE 玩家状态报告（XP/等级/属性点/技能点）"),
	FConsoleCommandWithArgsDelegate::CreateStatic(&AuraDevTools::CmdPrintPlayerReport));
