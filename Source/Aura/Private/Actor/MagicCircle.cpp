// Copyright Druid Mechanics


#include "Actor/MagicCircle.h"
#include "Components/DecalComponent.h"

/**
 * 构造函数：初始化魔法圈组件
 * 
 * 实现流程：
 * 1. 启用 Tick（用于动画更新）
 * 2. 创建 Decal 组件（用于显示魔法圈贴花）
 * 3. 将 Decal 挂载到根组件
 * 
 * 使用场景：
 * - 魔法圈 Actor 在关卡中放置时构造
 */
AMagicCircle::AMagicCircle()
{
	PrimaryActorTick.bCanEverTick = true;

	MagicCircleDecal = CreateDefaultSubobject<UDecalComponent>("MagicCircleDecal");
	MagicCircleDecal->SetupAttachment(GetRootComponent());
}

/**
 * 游戏开始时初始化（空实现）
 */
void AMagicCircle::BeginPlay()
{
	Super::BeginPlay();
}

/**
 * 每帧更新（空实现，可在蓝图中实现动画逻辑）
 */
void AMagicCircle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

