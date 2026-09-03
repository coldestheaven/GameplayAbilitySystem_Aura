// Copyright Druid Mechanics

using UnrealBuildTool;

public class AuraCoreEditor : ModuleRules
{
	public AuraCoreEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// 插件运行时模块（接口/类型）与 Slate UI 基础
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"AuraCore",
				"Slate",
				"SlateCore",
				"InputCore",
			}
		);

		// 编辑器框架：GEditor(UnrealEd)、工具菜单、编辑器样式、通知
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"UnrealEd",
				"ToolMenus",
				"EditorStyle",
				"Projects",
			}
		);
	}
}
