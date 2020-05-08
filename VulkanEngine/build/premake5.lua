--Generate Solution

windows_sdk_version = "10.0.18362.0"

workspace "VulkanEngine"
    configurations { "Debug", "Release" }
    platforms {"x64"}
    location "../project"
    cppdialect "C++17"

files {
    
    --VulkanEngine
    "../include/*.h",
    "../src/*.cpp",
    --
    
    -- GLFW
    "../deps/GLFW/*.h",

    -- TO BE INCLUDED AFTER DOWNLOADING GLFW CODE
    -- "../deps/GLFW/egl_context.h",
    -- "../deps/GLFW/glfw_config.h",
    -- "../deps/GLFW/internal.h",
    -- "../deps/GLFW/wgl_context.h",
    -- "../deps/GLFW/win32_joystick.h",
    -- "../deps/GLFW/win32_platform.h",
    -- "../deps/GLFW/context.c",
    -- "../deps/GLFW/egl_context.c",
    -- "../deps/GLFW/init.c",
    -- "../deps/GLFW/input.c",
    -- "../deps/GLFW/monitor.c",
    -- "../deps/GLFW/vulkan.c",
    -- "../deps/GLFW/wgl_context.c",
    -- "../deps/GLFW/win32_init.c",
    -- "../deps/GLFW/win32_joystick.c",
    -- "../deps/GLFW/win32_monitor.c",
    -- "../deps/GLFW/win32_time.c",
    -- "../deps/GLFW/win32_time.c",
    -- "../deps/GLFW/win32_time.c",
    -- "../deps/GLFW/win32_tls.c",
    -- "../deps/GLFW/win32_window.c",
    -- "../deps/GLFW/window.c",
    --

    -- GLM
    "../deps/glm/*.hpp",

    "../deps/glm/detail/*.hpp",
    "../deps/glm/detail/*.inl",

    "../deps/glm/ext/*.hpp",
    "../deps/glm/ext/*.inl",

    "../deps/glm/gtc/*.hpp",
    "../deps/glm/gtc/*.inl",

    "../deps/glm/gtx/*.hpp",
    "../deps/glm/gtx/*.inl",

    "../deps/glm/simd/*.hpp",
    "../deps/glm/simd/*.inl",

    "../deps/glm/detail/*.cpp",
    --

    -- Dear ImGUI
    "../deps/imgui/*.h",
    "../deps/imgui/*.cpp",
    --

    "../deps/stb_image.h",
    "../deps/tiny_gltf.h",

    "../resources/**.*"
}

removefiles{
    "../src/triangle_main.cpp",
    "../resources/shaders/*.spv",
}


systemversion(windows_sdk_version)

project "MainApp"
    kind "ConsoleApp"
    language "C++"
    links {"glfw3.lib", "vulkan-1.lib", "kernel32.lib",
     "user32.lib", "gdi32.lib", "winspool.lib",
     "shell32.lib", "ole32.lib", "oleaut32.lib",
     "uuid.lib", "comdlg32.lib", "advapi32.lib"}

     
    defines {"WIN64", "WINDOWS"}
    buildoptions { "/W4" }
		linkoptions {"/SUBSYSTEM:CONSOLE", "/NODEFAULTLIB:library"}
		prebuildcommands {"../build/compile_shaders.bat"}
		debugenvs {"VK_ICD_FILENAMES=C:\\Windows\\System32\\DriverStore\\FileRepository\\nvhmi.inf_amd64_f5abf44622a9ff68\\nv-vk64.json"}
            
    targetdir "../bin/%{cfg.buildcfg}"
    objdir "../build/obj/main"


    includedirs {"C:/VulkanSDK/1.1.121.2/Include", "../include", "../deps"}
    libdirs {"C:/VulkanSDK/1.1.121.2/Lib", "../libs/GLFW"}
    files {
        "../src/main.cpp"
    }

    filter "configurations:Debug"
        defines {"_DEBUG", "_CONSOLE"}
        symbols "On"
        debugdir "../bin/Debug"

    filter "configurations:Release"
        defines {"_DEBUG", "_CONSOLE"}
        optimize "On"

    filter "platforms:x64"
        kind "WindowedApp"
        architecture "x64"



    filter {"system:windows", "action:vs*"}

--End generate Solution
