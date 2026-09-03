// Copyright Druid Mechanics

using UnrealBuildTool;

public class AuraEditor : ModuleRules
{
	public AuraEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 游戏级编辑器工具：可依赖游戏模块（敌人/属性集等游戏类）
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Aura",
				"AuraCore",
				"GameplayAbilities",
				"GameplayTags",
				"Slate",
				"SlateCore",
			}
		);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",
				"EditorStyle",
			}
		);
	}
}
