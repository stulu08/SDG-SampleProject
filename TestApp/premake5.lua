project "TestApp"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"
	targetdir ("bin/" .. outputdir .. "")
	objdir ("bin-int/" .. outputdir .. "")
	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}
	defines
	{
		--"ST_DYNAMIC_LINK",
	}
	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"src",
		"%{ProjectDir.SDG}/src/Stulu/",
		"%{ProjectDir.SDG}/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}"
	}
	
	links
	{
		"SDG"
	}
	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "ST_DEBUG"
		runtime "Debug"
		optimize "off"
		symbols "on"

	filter "configurations:Release"
		defines "ST_RELEASE"
		runtime "Release"
		optimize "on"
		symbols "on"

	filter "configurations:Dist"
		defines "ST_DIST"
		kind "WindowedApp"
		runtime "Release"
		optimize "on"
		symbols "off"
