
workspace "gameengine"  
    configurations { "Debug", "Release" } 
    architecture "x64"
    outputdir = "%{cfg.buildcfg}-%{cfg.system}"
    IncludeDirs = {}
    IncludeDirs["spdlog"] = "enginecore/libs/spdlog/include"
    IncludeDirs["glfw"] = "enginecore/libs/glfw/include"
    Library = {}

group "libs"
include "enginecore/libs"
group ""

project "enginecore"  
    kind "StaticLib"   
    language "C++"   
    cppdialect "C++17"
    location "%{prj.name}"
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    files {"%{wks.location}/%{prj.name}/src/**.h", "%{wks.location}/%{prj.name}/src/**.cpp" }
    
    includedirs {
        "%{wks.location}/%{prj.name}/src",
        IncludeDirs["spdlog"],
        IncludeDirs["glfw"] 
    }

    filter "configurations:Debug"
    defines { "DEBUG" }  
    symbols "On" 
    filter "configurations:Release"  
    defines { "NDEBUG"}


project "engineeditor"  
    kind "ConsoleApp"
    language "C++"   
    cppdialect "C++17"
    location "%{prj.name}"
    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")
    files {"%{wks.location}/%{prj.name}/src/**.h", "%{wks.location}/%{prj.name}/src/**.cpp" }
    includedirs {
        "%{wks.location}/%{prj.name}/src",
        "%{wks.location}/enginecore/src",
    }

    links {
        "enginecore"
    }

    filter "configurations:Debug"
    defines { "DEBUG" }  
    symbols "On" 
    filter "configurations:Release"  
    defines { "NDEBUG" }    
    optimize "On" 
