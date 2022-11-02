workspace "SDG"
	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}
	flags
	{
		"MultiProcessorCompile"
	}
	architecture "x86_64"
	startproject "TestApp"
	filter "configurations:Debug"
		defines     "_DEBUG"

	filter "configurations:Release or Dist"
		defines     "NDEBUG"

	filter { "system:windows", "configurations:Dist", "toolset:not mingw" }
		flags		{ "LinkTimeOptimization" }

staticRuntime = true
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
vulkanSDK = os.getenv("VULKAN_SDK")

ProjectDir = {}
ProjectDir["SDG"] = "%{wks.location}/SDG"

IncludeDir = {}
IncludeDir["GLFW"] = "%{ProjectDir.SDG}/vendor/GLFW/include"
IncludeDir["Glad"] = "%{ProjectDir.SDG}/vendor/Glad/include"
IncludeDir["ImGui"] = "%{ProjectDir.SDG}/vendor/imgui"
IncludeDir["glm"] = "%{ProjectDir.SDG}/vendor/glm"
IncludeDir["stb_image"] = "%{ProjectDir.SDG}/vendor/stb_image"
IncludeDir["Vulkan"] = "%{vulkanSDK}/Include"

group "Dependencies"
include "SDG/vendor/GLFW"
include "SDG/vendor/Glad"
include "SDG/vendor/imgui"
group ""

include "SDG"
include "TestApp"

