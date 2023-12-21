
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
    IncludeDirs["cgltf"] = "enginecore/libs/cgltf"
    IncludeDirs["entt"] = "enginecore/libs/entt/single_include"
    IncludeDirs["json"] = "enginecore/libs/json/single_include"
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
        IncludeDirs["glm"],
        IncludeDirs["cgltf"],
        IncludeDirs["entt"],
        IncludeDirs["json"]
    }

    libdirs {
        Library["Vulkan"]
    }

    links {
        "glfw",
        "vulkan-1.lib",
    }

    defines {"_CRT_SECURE_NO_WARNINGS"}

    disablewarnings {"4099"}

    filter "configurations:Debug"
        defines { "DEBUG"}  
        runtime "Debug"
        symbols "on" 
        links {
            "spirv-cross-cored.lib"
        }

    filter "configurations:Release"  
        defines { "NDEBUG"}
        runtime "Release"
        symbols "Off"
        links {
            "spirv-cross-core.lib"
        }

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
        "engineeditor/libs/imgui",
        IncludeDirs["spdlog"],
        IncludeDirs["Vulkan"],
        IncludeDirs["glfw"],
        IncludeDirs["glm"],
        IncludeDirs["vma"],
        IncludeDirs["entt"]
    }

    links {
        "enginecore",
        "imgui"
    }

    defines {"_CRT_SECURE_NO_WARNINGS"}
    ignoredefaultlibraries { "LIBCMTD" }

    filter "configurations:Debug"
        defines { "DEBUG" }  
        runtime "Debug"
        symbols "On" 
    filter "configurations:Release"  
        defines { "NDEBUG" }    
        runtime "Release"
        optimize "On" 

    filter "system:windows"
        defines {"EC_WINDOWS"}

project "runtime"  
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
        IncludeDirs["entt"]
    }
    
    links {
        "enginecore",
    }
    
    defines {"_CRT_SECURE_NO_WARNINGS"}
    ignoredefaultlibraries { "LIBCMTD" }
    
    filter "configurations:Debug"
        defines { "DEBUG" }  
        runtime "Debug"
        symbols "On" 
    filter "configurations:Release"  
        defines { "NDEBUG" }    
        runtime "Release"
        optimize "On" 
    
    filter "system:windows"
        defines {"EC_WINDOWS"}