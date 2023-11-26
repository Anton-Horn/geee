
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
    IncludeDirs["cgltf"] = "enginecore/libs/cgltf"
    Library = {}
    Library["Vulkan"] = "%{VULKAN_SDK}/Lib"

group "engine_libs"
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
        IncludeDirs["glm"],
        IncludeDirs["cgltf"]
    }

    libdirs {
        Library["Vulkan"]
    }

    links {
        "glfw",
        "spirv-reflect",
        "vulkan-1.lib"
    }

    defines {"_CRT_SECURE_NO_WARNINGS"}

    filter "configurations:Debug"
    defines { "DEBUG"}  
    symbols "On" 
    filter "configurations:Release"  
    defines { "NDEBUG"}
    
    filter "system:windows"
    defines {"EC_WINDOWS"}

group "editor_libs"
include "engineeditor/libs"
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
        IncludeDirs["spdlog"],
        IncludeDirs["Vulkan"],
        IncludeDirs["glfw"],
        IncludeDirs["glm"],
        IncludeDirs["vma"],
        "engineeditor/libs/imgui"
    }

    links {
        "enginecore",
        "imgui"
    }

    defines {"_CRT_SECURE_NO_WARNINGS"}
    ignoredefaultlibraries { "LIBCMTD" }

    filter "configurations:Debug"
    defines { "DEBUG" }  
    symbols "On" 
    filter "configurations:Release"  
    defines { "NDEBUG" }    
    optimize "On" 

    filter "system:windows"
    defines {"EC_WINDOWS"}

