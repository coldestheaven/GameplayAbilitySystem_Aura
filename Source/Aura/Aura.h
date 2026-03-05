// Copyright Druid Mechanics

#pragma once

#include "CoreMinimal.h"

/**
 * 自定义深度模板值（用于高亮描边效果）
 * 后处理材质根据此值绘制不同颜色的轮廓描边
 * 在 Project Settings → Rendering 中启用 Custom Depth-Stencil Pass
 */
#define CUSTOM_DEPTH_RED 250   // 红色描边（敌人高亮）
#define CUSTOM_DEPTH_BLUE 251  // 蓝色描边（地图入口高亮）
#define CUSTOM_DEPTH_TAN 252   // 棕褐色描边（检查点高亮）

/**
 * 自定义碰撞通道定义
 * 在 Project Settings → Collision 中配置对应的通道名称
 */
#define ECC_Projectile ECollisionChannel::ECC_GameTraceChannel1  // 投射物碰撞通道（只与角色和地形碰撞）
#define ECC_Target ECollisionChannel::ECC_GameTraceChannel2      // 目标选择通道（用于鼠标点击目标检测）
#define ECC_ExcludePlayers ECollisionChannel::ECC_GameTraceChannel3  // 排除玩家通道（范围检测时忽略玩家）