// Copyright Druid Mechanics


#include "Player/AuraPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayTags/AuraGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Actor/MagicCircle.h"
#include "Aura/Aura.h"
#include "Components/DecalComponent.h"
#include "Components/SplineComponent.h"
#include "Input/AuraInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "GameFramework/Character.h"
#include "Interaction/HighlightInterface.h"
#include "UI/Widget/DamageTextComponent.h"

/**
 * 构造函数：初始化玩家控制器
 * 
 * 实现流程：
 * 1. 启用网络复制（PlayerController 需要在客户端和服务端都存在）
 * 2. 创建样条线组件（用于自动寻路路径可视化）
 * 
 * 使用场景：
 * - 玩家控制器在游戏开始时由引擎自动构造
 * - Spline 组件用于绘制和跟踪自动寻路路径
 */
AAuraPlayerController::AAuraPlayerController()
{
	bReplicates = true;
	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

/**
 * 每帧更新：处理光标追踪、自动移动和魔法圆圈位置
 * 
 * 实现流程：
 * 1. 调用父类 PlayerTick
 * 2. 执行光标追踪（检测鼠标下的 Actor，处理高亮）
 * 3. 执行自动移动（如果启用了自动寻路）
 * 4. 更新魔法圆圈位置（跟随鼠标光标）
 * 
 * 使用场景：
 * - 每帧执行，确保光标追踪和自动移动流畅
 */
void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	CursorTrace();
	AutoRun();
	UpdateMagicCircleLocation();
}

/**
 * 显示魔法圆圈（地面贴花）
 * 
 * 实现流程：
 * 1. 如果魔法圆圈不存在，创建新的 AMagicCircle Actor
 * 2. 如果提供了材质，设置贴花材质
 * 
 * @param DecalMaterial 圆圈使用的贴花材质（可选）
 * 
 * 使用场景：
 * - 需要指定目标位置的技能（如范围技能、传送）
 * - 由角色通过 IPlayerInterface::ShowMagicCircle 调用
 */
void AAuraPlayerController::ShowMagicCircle(UMaterialInterface* DecalMaterial)
{
	if (!IsValid(MagicCircle))
	{
		MagicCircle = GetWorld()->SpawnActor<AMagicCircle>(MagicCircleClass);
		if (DecalMaterial)
		{
			MagicCircle->MagicCircleDecal->SetMaterial(0, DecalMaterial);
		}
	}
}

/**
 * 隐藏魔法圆圈
 * 
 * 实现流程：
 * 1. 如果魔法圆圈存在，销毁它
 * 
 * 使用场景：
 * - 技能取消或完成时
 * - 由角色通过 IPlayerInterface::HideMagicCircle 调用
 */
void AAuraPlayerController::HideMagicCircle()
{
	if (IsValid(MagicCircle))
	{
		MagicCircle->Destroy();
	}
}

/**
 * 客户端 RPC：显示伤害数字
 * 
 * 实现流程：
 * 1. 校验目标角色、伤害文本组件类和本地控制器有效性
 * 2. 创建伤害文本组件对象
 * 3. 注册组件
 * 4. 先附加到目标根组件，再分离（保持世界位置）
 * 5. 设置伤害文本（数值、是否格挡、是否暴击）
 * 
 * @param DamageAmount 伤害数值
 * @param TargetCharacter 受到伤害的角色
 * @param bBlockedHit 是否为格挡攻击
 * @param bCriticalHit 是否为暴击
 * 
 * 网络同步说明：
 * - 此函数为客户端 RPC，仅在本地客户端执行
 * - 服务端调用后，只有调用者的客户端会显示伤害数字
 * - 其他客户端不会看到此伤害数字（避免重复显示）
 * 
 * 使用场景：
 * - 伤害计算完成后，在服务端调用此函数显示伤害数字
 */
void AAuraPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit)
{
	if (IsValid(TargetCharacter) && DamageTextComponentClass && IsLocalController())
	{
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DamageText->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);
	}
}

/**
 * IDamageTextDisplayInterface 实现：
 * 让上层（AttributeSet）通过接口调用，无需依赖 AAuraPlayerController 具体类型
 * 内部直接转发到原有的 Client RPC ShowDamageNumber，保留所有网络行为
 */
void AAuraPlayerController::DisplayDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit)
{
	ShowDamageNumber(DamageAmount, TargetCharacter, bBlockedHit, bCriticalHit);
}

/**
 * IMagicCircleController 实现：转发到已有的 ShowMagicCircle，并隐藏鼠标光标
 * 让 Character 通过接口调用，无需 Cast 到 AAuraPlayerController 具体类型
 */
void AAuraPlayerController::ShowMagicCircleUI_Implementation(UMaterialInterface* DecalMaterial)
{
	ShowMagicCircle(DecalMaterial);
	bShowMouseCursor = false;
}

/**
 * IMagicCircleController 实现：转发到已有的 HideMagicCircle，并恢复鼠标光标
 */
void AAuraPlayerController::HideMagicCircleUI_Implementation()
{
	HideMagicCircle();
	bShowMouseCursor = true;
}

/**
 * 自动寻路移动
 * 
 * 实现流程：
 * 1. 如果未启用自动移动，直接返回
 * 2. 获取受控 Pawn
 * 3. 在样条线上找到最接近 Pawn 当前位置的点
 * 4. 计算该点的方向（沿样条线前进的方向）
 * 5. 向该方向添加移动输入
 * 6. 计算到目标点的距离，如果小于接受半径，停止自动移动
 * 
 * 使用场景：
 * - 玩家点击地面后，角色自动沿寻路路径移动到目标位置
 * - 在 PlayerTick 中每帧调用
 * 
 * 注意：
 * - 样条线路径在 AbilityInputTagReleased 中通过导航系统生成
 * - AutoRunAcceptanceRadius 控制到达目标的判定距离
 */
void AAuraPlayerController::AutoRun()
{
	if (!bAutoRunning) return;
	if (APawn* ControlledPawn = GetPawn())
	{
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		ControlledPawn->AddMovementInput(Direction);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}
}

/**
 * 更新魔法圆圈位置（跟随鼠标光标）
 * 
 * 实现流程：
 * 1. 如果魔法圆圈存在，将其位置设置为光标碰撞点
 * 
 * 使用场景：
 * - 在 PlayerTick 中每帧调用，使魔法圆圈跟随鼠标移动
 */
void AAuraPlayerController::UpdateMagicCircleLocation()
{
	if (IsValid(MagicCircle))
	{
		MagicCircle->SetActorLocation(CursorHit.ImpactPoint);
	}
}

/**
 * 高亮显示 Actor
 * 
 * 实现流程：
 * 1. 校验 Actor 有效且实现了 HighlightInterface
 * 2. 调用接口函数执行高亮逻辑
 * 
 * @param InActor 要高亮的 Actor
 * 
 * 使用场景：
 * - 鼠标悬停在可交互对象上时高亮显示
 */
void AAuraPlayerController::HighlightActor(AActor* InActor)
{
	if (IsValid(InActor) && InActor->Implements<UHighlightInterface>())
	{
		IHighlightInterface::Execute_HighlightActor(InActor);
	}
}

/**
 * 取消高亮显示 Actor
 * 
 * 实现流程：
 * 1. 校验 Actor 有效且实现了 HighlightInterface
 * 2. 调用接口函数执行取消高亮逻辑
 * 
 * @param InActor 要取消高亮的 Actor
 * 
 * 使用场景：
 * - 鼠标移开可交互对象时取消高亮
 */
void AAuraPlayerController::UnHighlightActor(AActor* InActor)
{
	if (IsValid(InActor) && InActor->Implements<UHighlightInterface>())
	{
		IHighlightInterface::Execute_UnHighlightActor(InActor);
	}
}

/**
 * 光标追踪：检测鼠标下的 Actor 并处理高亮
 * 
 * 实现流程：
 * 1. 检查是否被阻止光标追踪（如眩晕状态）
 *    - 如果被阻止，取消所有高亮并返回
 * 2. 根据是否有魔法圆圈选择碰撞通道：
 *    - 有魔法圆圈：使用 ECC_ExcludePlayers（排除玩家，避免魔法圆圈被玩家遮挡）
 *    - 无魔法圆圈：使用 ECC_Visibility（标准可见性通道）
 * 3. 执行光标下的碰撞检测
 * 4. 如果未命中，直接返回
 * 5. 更新 LastActor 和 ThisActor：
 *    - LastActor = 上一帧的 ThisActor
 *    - ThisActor = 当前命中的 Actor（如果实现了 HighlightInterface）
 * 6. 如果 Actor 发生变化，取消旧 Actor 高亮，高亮新 Actor
 * 
 * 使用场景：
 * - 在 PlayerTick 中每帧调用，实现鼠标悬停高亮效果
 * 
 * 注意：
 * - 魔法圆圈存在时使用 ECC_ExcludePlayers，确保魔法圆圈不被玩家角色遮挡
 * - 只有实现了 HighlightInterface 的 Actor 才会被高亮
 */
void AAuraPlayerController::CursorTrace()
{
	// 检查是否被阻止光标追踪（如眩晕状态）
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_CursorTrace))
	{
		UnHighlightActor(LastActor);
		UnHighlightActor(ThisActor);
		LastActor = nullptr;
		ThisActor = nullptr;
		return;
	}
	
	// 根据是否有魔法圆圈选择碰撞通道
	const ECollisionChannel TraceChannel = IsValid(MagicCircle) ? ECC_ExcludePlayers : ECC_Visibility;
	GetHitResultUnderCursor(TraceChannel, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	// 更新当前和上一个 Actor
	LastActor = ThisActor;
	if (IsValid(CursorHit.GetActor()) && CursorHit.GetActor()->Implements<UHighlightInterface>())
	{
		ThisActor = CursorHit.GetActor();
	}
	else
	{
		ThisActor = nullptr;
	}

	// 如果 Actor 发生变化，更新高亮状态
	if (LastActor != ThisActor)
	{
		UnHighlightActor(LastActor);
		HighlightActor(ThisActor);
	}
}

/**
 * 技能输入标签按下事件处理
 * 
 * 实现流程：
 * 1. 检查是否被阻止输入按下（如眩晕状态）
 * 2. 如果是左键（LMB）：
 *    - 根据当前光标下的 Actor 设置目标状态：
 *      * 如果是敌人：设置为 TargetingEnemy
 *      * 如果是非敌人：设置为 TargetingNonEnemy
 *      * 如果没有 Actor：设置为 NotTargeting
 *    - 停止自动移动
 * 3. 将输入标签传递给 ASC 处理（触发对应技能）
 * 
 * @param InputTag 输入标签（如 InputTag_LMB、InputTag_RMB 等）
 * 
 * 使用场景：
 * - 玩家按下技能快捷键时调用
 * - 由 AuraInputComponent 的输入绑定触发
 * 
 * 注意：
 * - 左键按下时会停止自动移动，避免与技能输入冲突
 * - 目标状态用于区分是攻击敌人还是移动到非敌人位置
 */
void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed))
	{
		return;
	}
	
	if (InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (IsValid(ThisActor))
		{
			TargetingStatus = ThisActor->Implements<UEnemyInterface>() ? ETargetingStatus::TargetingEnemy : ETargetingStatus::TargetingNonEnemy;
		}
		else
		{
			TargetingStatus = ETargetingStatus::NotTargeting;
		}
		bAutoRunning = false;
	}
	
	if (GetASC())
	{
		GetASC()->AbilityInputTagPressed(InputTag);
	}
}

/**
 * 技能输入标签释放事件处理
 * 
 * 实现流程：
 * 1. 检查是否被阻止输入释放
 * 2. 如果不是左键，直接传递给 ASC 并返回
 * 3. 将输入标签传递给 ASC（释放技能）
 * 4. 如果是左键且未瞄准敌人且未按住 Shift：
 *    - 如果是短按（FollowTime <= ShortPressThreshold）：
 *      * 如果点击的是可交互对象，设置其移动目标位置
 *      * 否则在地面生成点击特效（Niagara）
 *      * 使用导航系统计算寻路路径
 *      * 将路径点添加到样条线
 *      * 启用自动移动
 *    - 重置 FollowTime 和目标状态
 * 
 * @param InputTag 输入标签
 * 
 * 使用场景：
 * - 玩家释放技能快捷键时调用
 * - 左键释放时处理移动逻辑（短按移动，长按攻击）
 * 
 * 注意：
 * - 短按左键会触发自动寻路移动
 * - 长按左键（超过 ShortPressThreshold）会触发攻击技能
 * - Shift + 左键会直接触发技能，不触发移动
 */
void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputReleased))
	{
		return;
	}
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag);
		return;
	}

	if (GetASC()) GetASC()->AbilityInputTagReleased(InputTag);
	
	// 左键释放：如果不是瞄准敌人且未按住 Shift，处理移动逻辑
	if (TargetingStatus != ETargetingStatus::TargetingEnemy && !bShiftKeyDown)
	{
		const APawn* ControlledPawn = GetPawn();
		// 短按：触发移动
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			// 如果点击的是可交互对象，设置其移动目标
			if (IsValid(ThisActor) && ThisActor->Implements<UHighlightInterface>())
			{
				IHighlightInterface::Execute_SetMoveToLocation(ThisActor, CachedDestination);
			}
			// 否则在地面生成点击特效
			else if (GetASC() && !GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed))
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ClickNiagaraSystem, CachedDestination);
			}
			// 计算寻路路径并启用自动移动
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				Spline->ClearSplinePoints();
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
				}
				if (NavPath->PathPoints.Num() > 0)
				{
					CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
					bAutoRunning = true;
				}
			}
		}
		FollowTime = 0.f;
		TargetingStatus = ETargetingStatus::NotTargeting;
	}
}

/**
 * 技能输入标签按住事件处理
 * 
 * 实现流程：
 * 1. 检查是否被阻止输入按住
 * 2. 如果不是左键，直接传递给 ASC 并返回
 * 3. 如果是左键：
 *    - 如果瞄准敌人或按住 Shift：传递给 ASC（触发攻击技能）
 *    - 否则：
 *      * 累加按住时间（FollowTime）
 *      * 更新缓存的目标位置（跟随鼠标）
 *      * 直接向目标方向移动（不等待寻路）
 * 
 * @param InputTag 输入标签
 * 
 * 使用场景：
 * - 玩家按住技能快捷键时调用
 * - 左键按住时：瞄准敌人或按住 Shift 触发攻击，否则直接移动
 * 
 * 注意：
 * - FollowTime 用于区分短按和长按
 * - 按住移动是直接移动，不经过寻路系统（更流畅）
 * - 释放时会根据 FollowTime 决定是移动还是攻击
 */
void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputHeld))
	{
		return;
	}
	if (!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
		return;
	}

	// 左键按住：瞄准敌人或按住 Shift 时触发技能，否则直接移动
	if (TargetingStatus == ETargetingStatus::TargetingEnemy || bShiftKeyDown)
	{
		if (GetASC())
		{
			GetASC()->AbilityInputTagHeld(InputTag);
		}
	}
	else
	{
		// 累加按住时间（用于区分短按和长按）
		FollowTime += GetWorld()->GetDeltaSeconds();
		// 更新目标位置（跟随鼠标）
		if (CursorHit.bBlockingHit){ CachedDestination = CursorHit.ImpactPoint;}

		// 直接向目标方向移动（不等待寻路）
		if (APawn* ControlledPawn = GetPawn())
		{
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection);
		}
	}
}

/**
 * 获取 AbilitySystemComponent（延迟初始化）
 * 
 * 实现流程：
 * 1. 如果 ASC 未缓存，从 Pawn 获取并缓存
 * 2. 返回缓存的 ASC
 * 
 * @return AbilitySystemComponent 指针
 * 
 * 使用场景：
 * - 需要访问 ASC 时调用（避免重复查找）
 * 
 * 注意：
 * - 使用延迟初始化，确保 Pawn 已创建后再获取 ASC
 */
UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
	if (AuraAbilitySystemComponent == nullptr)
	{
		AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return AuraAbilitySystemComponent;
}

/**
 * 游戏开始时初始化
 * 
 * 实现流程：
 * 1. 调用父类 BeginPlay
 * 2. 校验输入映射上下文有效
 * 3. 添加输入映射上下文到本地玩家子系统
 * 4. 显示鼠标光标
 * 5. 设置输入模式为 GameAndUI（允许鼠标和键盘输入）
 * 
 * 使用场景：
 * - 玩家控制器创建后自动调用
 * 
 * 注意：
 * - 输入映射上下文优先级为 0（可在蓝图中调整）
 * - GameAndUI 模式允许同时使用鼠标和键盘
 */
void AAuraPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(AuraContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if (Subsystem)
	{
		Subsystem->AddMappingContext(AuraContext, 0);
	}

	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

/**
 * 设置输入组件绑定
 * 
 * 实现流程：
 * 1. 调用父类 SetupInputComponent
 * 2. 将 InputComponent 转换为 AuraInputComponent
 * 3. 绑定移动输入（WASD）
 * 4. 绑定 Shift 键按下/释放
 * 5. 绑定技能输入（通过 InputConfig 配置的技能快捷键）
 * 
 * 使用场景：
 * - 玩家控制器创建后自动调用
 * 
 * 注意：
 * - BindAbilityActions 会遍历 InputConfig 中的所有技能输入配置
 * - 每个技能输入会绑定到对应的按下/释放/按住事件
 */
void AAuraPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);
	AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AAuraPlayerController::ShiftPressed);
	AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AAuraPlayerController::ShiftReleased);
	AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}

/**
 * 处理移动输入（WASD）
 * 
 * 实现流程：
 * 1. 检查是否被阻止输入
 * 2. 获取输入轴向量（X=左右，Y=前后）
 * 3. 获取控制器旋转（仅使用 Yaw，忽略俯仰和翻滚）
 * 4. 计算前进和右方向向量
 * 5. 向对应方向添加移动输入
 * 
 * @param InputActionValue 输入动作值（包含 X 和 Y 轴）
 * 
 * 使用场景：
 * - 玩家按下 WASD 键时调用
 * - 由 Enhanced Input 系统触发
 * 
 * 注意：
 * - 使用控制器的 Yaw 旋转计算方向，确保移动方向与摄像机一致
 * - 俯视角下，角色移动方向与摄像机朝向一致
 */
void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{
	if (GetASC() && GetASC()->HasMatchingGameplayTag(FAuraGameplayTags::Get().Player_Block_InputPressed))
	{
		return;
	}
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}
