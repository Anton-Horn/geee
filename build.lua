
workspace "gameengine"  
    configurations { "Debug", "Release" } 
    architecture "x64"
    outputdir = "%{cfg.buildcfg}-%{cfg.system}"
    IncludeDirs = {}
    Library = {}


group "libs"
group ""

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
    }

    filter "configurations:Debug"
    defines { "DEBUG" }  
    symbols "On" 
    filter "configurations:Release"  
    defines { "NDEBUG"}

