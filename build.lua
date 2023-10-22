
workspace "gameengine"  
    configurations { "Debug", "Release" } 
    architecture "x64"
    VULKAN_SDK = os.getenv("VULKAN_SDK")
    outputdir = "%{cfg.buildcfg}-%{cfg.system}"
    IncludeDirs = {}
    IncludeDirs["spdlog"] = "enginecore/libs/spdlog/include"
    IncludeDirs["glfw"] = "enginecore/libs/glfw/include"
    IncludeDirs["Vulkan"] = "%{VULKAN_SDK}/Include"
    IncludeDirs["vma"] = "enginecore/libs/vma/include"
    IncludeDirs["glm"] = "enginecore/libs/glm"
    IncludeDirs["spirv-reflect"] = "enginecore/libs/spirv-reflect"
    Library = {}
    Library["Vulkan"] = "%{VULKAN_SDK}/Lib"

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
        IncludeDirs["glfw"],
        IncludeDirs["Vulkan"],
        IncludeDirs["vma"],
        IncludeDirs["spirv-reflect"],
        IncludeDirs["glm"]
    }

    libdirs {
        Library["Vulkan"]
    }

    links {
        "glfw",
        "spirv-reflect",
        "vulkan-1.lib"
    }

    filter "configurations:Debug"
    defines { "DEBUG"}  
    symbols "On" 
    filter "configurations:Release"  
    defines { "NDEBUG"}
    
    filter "system:windows"
    defines {"EC_WINDOWS"}


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
        IncludeDirs["spdlog"]
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

