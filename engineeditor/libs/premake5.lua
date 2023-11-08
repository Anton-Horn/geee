project "imgui"
kind "StaticLib"
language "C++"
systemversion "latest"
staticruntime "on"

location "%{prj.name}"

files {
    "imgui/imgui.cpp",
    "imgui/imgui.h",
    "imgui/imconfig.h",
    "imgui/imgui_demo.cpp",
    "imgui/imgui_draw.cpp",
    "imgui/imgui_widgets.cpp",
    "imgui/imgui_tables.cpp",
    "imgui/imgui_internal.h",
    "imgui/imstb_rectpack.h",
    "imgui/imstb_textedit.h",
    "imgui/imstb_truetype.h"
}

targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    
includedirs {
    "%{prj.name}/"
}
 
defines {
	"_CRT_SECURE_NO_WARNINGS" 
}

filter "configurations:Debug"
    runtime "Debug"
    symbols "on"

filter "configurations:Release"
    runtime "Release"
    optimize "on"
    