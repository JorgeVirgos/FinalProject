@echo off
echo.
echo Compiling shaders...

del D:\WORKSPACE\Vulkan\FinalProjectRepo\VulkanEngine\resources\shaders\*.spv
C:/VulkanSDK/1.1.121.2/Bin/glslc.exe ./../../resources/shaders/test.vert -o ./../../resources/shaders/test_vert.spv
C:/VulkanSDK/1.1.121.2/Bin/glslc.exe ./../../resources/shaders/test.frag -o ./../../resources/shaders/test_frag.spv

C:/VulkanSDK/1.1.121.2/Bin/glslc.exe ./../../resources/shaders/cubemap.vert -o ./../../resources/shaders/cubemap_vert.spv
C:/VulkanSDK/1.1.121.2/Bin/glslc.exe ./../../resources/shaders/cubemap.frag -o ./../../resources/shaders/cubemap_frag.spv

C:/VulkanSDK/1.1.121.2/Bin/glslc.exe ./../../resources/shaders/quadscreen.vert -o ./../../resources/shaders/quadscreen_vert.spv
C:/VulkanSDK/1.1.121.2/Bin/glslc.exe ./../../resources/shaders/quadscreen.frag -o ./../../resources/shaders/quadscreen_frag.spv

C:/VulkanSDK/1.1.121.2/Bin/glslc.exe ./../../resources/shaders/shadowmap.vert -o ./../../resources/shaders/shadowmap_vert.spv
C:/VulkanSDK/1.1.121.2/Bin/glslc.exe ./../../resources/shaders/shadowmap.frag -o ./../../resources/shaders/shadowmap_frag.spv
echo Done.
echo.
@echo on