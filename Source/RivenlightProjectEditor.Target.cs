using UnrealBuildTool;
using System.Collections.Generic;

public class RivenlightProjectEditorTarget : TargetRules
{
	public RivenlightProjectEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("RivenlightProject");
	}
}
