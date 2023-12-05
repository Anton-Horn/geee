project "spdlog"
    kind "None"
    cppdialect "C++11"
    location "spdlog"
    staticruntime "on"

    files {"%{prj.name}/include/**.h"}

    includedirs {
        "%{prj.name}/include/",
    }

project "glfw"
    location "%{prj.name}"
    language "C"
    kind "StaticLib"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "glfw/include/GLFW/glfw3.h",
        "glfw/include/GLFW/glfw3native.h",
        "glfw/src/internal.h",
        "glfw/src/platform.h",
        "glfw/src/mappings.h",
        "glfw/src/context.c",
        "glfw/src/init.c",
        "glfw/src/input.c",
        "glfw/src/monitor.c",
        "glfw/src/platform.c",
        "glfw/src/vulkan.c",
        "glfw/src/window.c",
        "glfw/src/egl_context.c",
        "glfw/src/osmesa_context.c",
        "glfw/src/null_platform.h",
        "glfw/src/null_joystick.h",
        "glfw/src/null_init.c",

        "glfw/src/null_monitor.c",
        "glfw/src/null_window.c",
        "glfw/src/null_joystick.c",
    }

    includedirs {
        "%{prj.name}/include/",
    }

    filter "system:linux"
        pic "On"

        systemversion "latest"
        staticruntime "On"

        files
        {
            "glfw/src/x11_init.c",
            "glfw/src/x11_monitor.c",
            "glfw/src/x11_window.c",
            "glfw/src/xkb_unicode.c",
            "glfw/src/posix_time.c",
            "glfw/src/posix_thread.c",
            "glfw/src/glx_context.c",
            "glfw/src/egl_context.c",
            "glfw/src/osmesa_context.c",
            "glfw/src/linux_joystick.c"
        }

        defines
        {
            "_GLFW_X11"
            
        }

    filter "system:windows"
        systemversion "latest"
        staticruntime "On"

        files
        {
            "glfw/src/win32_init.c",
            "glfw/src/win32_module.c",
            "glfw/src/win32_joystick.c",
            "glfw/src/win32_monitor.c",
            "glfw/src/win32_time.h",
            "glfw/src/win32_time.c",
            "glfw/src/win32_thread.h",
            "glfw/src/win32_thread.c",
            "glfw/src/win32_window.c",
            "glfw/src/wgl_context.c",
            "glfw/src/egl_context.c",
            "glfw/src/osmesa_context.c"
        }

        defines 
        { 
            "_GLFW_WIN32",
            "_CRT_SECURE_NO_WARNINGS"

        }

    filter "configurations:Debug"
        symbols "On"

    filter "configurations:Release"
        optimize "On"
        symbols "off"

    filter "configurations:Dist" 
        optimize "On"
        symbols "off"

project "vma"
    kind "None"
    language "C++"
    location "%{prj.name}"
    staticruntime "on"
    files {"%{prj.name}/include/**.h"}

    filter "configurations:Debug"
    symbols "On"

    filter "configurations:Release"
    optimize "On"
    symbols "off"

project "glm"
    kind "None"
    cppdialect "C++11"
    location "%{prj.name}"
    staticruntime "on"
    
    files {"%{prj.name}/glm/**.hpp" }
    
    includedirs {
        "%{prj.name}/include",
    }
    
    filter "configurations:Debug"
        symbols "On"
    
    filter "configurations:Release"
        optimize "On"
        symbols "off"
    