using UnrealBuildTool;
using System.Collections.Generic;

public class rocket_jumpEditorTarget : TargetRules
{
	public rocket_jumpEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.AddRange(new string[] { "RocketJump" });
	}
}
