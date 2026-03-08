// Copyright Druid Mechanics


#include "AbilitySystem/Abilities/AuraSummonAbility.h"

/**
 * 获取召唤物生成位置（扇形分布）
 * 
 * 实现流程：
 * 1. 获取角色前进方向和位置
 * 2. 计算每个召唤物之间的角度间隔（DeltaSpread）
 * 3. 计算最左侧方向（Forward 旋转 -SpawnSpread/2）
 * 4. 为每个召唤物：
 *    - 计算方向（从最左侧旋转 DeltaSpread * i）
 *    - 计算生成位置（位置 + 方向 * 随机距离）
 *    - 使用射线检测找到地面位置（从上方 400 单位向下检测）
 *    - 如果命中地面，使用命中点作为生成位置
 *    - 添加到生成位置数组
 * 5. 返回所有生成位置
 * 
 * @return 召唤物生成位置数组（扇形分布）
 * 
 * 使用场景：
 * - 召唤技能激活时调用，确定召唤物生成位置
 * 
 * 注意：
 * - 召唤物以扇形分布在角色前方
 * - 使用射线检测确保召唤物生成在地面上
 * - 生成距离在 MinSpawnDistance 和 MaxSpawnDistance 之间随机
 */
TArray<FVector> UAuraSummonAbility::GetSpawnLocations()
{
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	const float DeltaSpread = SpawnSpread / NumMinions;

	// 计算最左侧方向（扇形起始方向）
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-SpawnSpread / 2.f, FVector::UpVector);
	TArray<FVector> SpawnLocations;
	
	// 为每个召唤物计算生成位置
	for (int32 i = 0; i < NumMinions; i++)
	{
		// 计算方向（从最左侧旋转）
		const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
		FVector ChosenSpawnLocation = Location + Direction * FMath::FRandRange(MinSpawnDistance, MaxSpawnDistance);

		// 使用射线检测找到地面位置
		FHitResult Hit;
		GetWorld()->LineTraceSingleByChannel(Hit, ChosenSpawnLocation + FVector(0.f, 0.f, 400.f), ChosenSpawnLocation - FVector(0.f, 0.f, 400.f), ECC_Visibility);
		if (Hit.bBlockingHit)
		{
			ChosenSpawnLocation = Hit.ImpactPoint;
		}
		SpawnLocations.Add(ChosenSpawnLocation);
	}
	
	return SpawnLocations;
}

/**
 * 获取随机召唤物类
 * 
 * 实现流程：
 * 1. 在 MinionClasses 数组中随机选择一个索引
 * 2. 返回对应的召唤物类
 * 
 * @return 随机选择的召唤物类
 * 
 * 使用场景：
 * - 召唤技能生成召唤物时调用
 * 
 * 注意：
 * - 如果 MinionClasses 为空，会返回 nullptr
 */
TSubclassOf<APawn> UAuraSummonAbility::GetRandomMinionClass()
{
	const int32 Selection = FMath::RandRange(0, MinionClasses.Num() - 1);
	return MinionClasses[Selection];
}
