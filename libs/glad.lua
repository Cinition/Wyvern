project "GLAD"
	kind "StaticLib"
	language "C"
	
	location "../build/%{prj.name}"
	targetdir "../build/%{prj.name}/bin/%{cfg.platform}_%{cfg.buildcfg}"
	objdir "../build/%{prj.name}/obj/%{cfg.platform}_%{cfg.buildcfg}"
	
    includedirs { "../libs/glad/include/" }

	files { "../libs/glad/src/glad.c" }
    
	filter "system:linux"
		pic "On"

		systemversion "latest"
		staticruntime "On"

		defines
		{
			"_GLAD_X11"
		}

	filter "system:windows"
		systemversion "latest"
		staticruntime "On"

		defines 
		{ 
			"_GLAD_WIN32",
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"