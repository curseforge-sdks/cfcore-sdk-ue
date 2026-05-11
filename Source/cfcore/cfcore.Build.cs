/*MIT License

Copyright (c) 2022 Overwolf Ltd.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
using UnrealBuildTool;

#pragma warning disable CS8981 // The type name only contains lower-cased ascii characters. Such names may become reserved for the language.
public class cfcore : ModuleRules {
#pragma warning restore CS8981 // The type name only contains lower-cased ascii characters. Such names may become reserved for the language.
  public cfcore(ReadOnlyTargetRules Target) : base(Target) {
    PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

    //if (Target.Platform == UnrealTargetPlatform.Win32) {
    //  PrivateIncludePaths.Add
    //} else {

    //}

    PublicIncludePaths.AddRange(
      new string[] {
        // ... add public include paths required here ...
      }
    );

    PrivateIncludePaths.AddRange(
      new string[] {
        // ... add other private include paths required here ...
      }
    );

    PublicDependencyModuleNames.AddRange(
      new string[] {
        "Core",
        "InputCore"
        // ... add other public dependencies that you statically link with here ...
      }
    );

    PrivateDependencyModuleNames.AddRange(
      new string[] {
        "CoreUObject",
        "Engine",
        "HTTP",
        "Projects",
        "Json",
        "JsonUtilities",
        "OpenSSL"
      }
    );

    AddEngineThirdPartyPrivateStaticDependencies(Target, "zlib");

    DynamicallyLoadedModuleNames.AddRange(
      new string[] {
        // ... add any modules that your module loads dynamically here ...
      }
    );

    PublicDefinitions.Add("CFCORE_OBSOLETE_TESTS=0");

    // For running tests
    if (Target.Platform == UnrealTargetPlatform.Win64 &&
        (Target.Configuration == UnrealTargetConfiguration.DebugGame ||
         Target.Configuration == UnrealTargetConfiguration.Debug ||
         Target.Configuration == UnrealTargetConfiguration.Development))
    {
      bUseRTTI = true;
    }
  }
}
