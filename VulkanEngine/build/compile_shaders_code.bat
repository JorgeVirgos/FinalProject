@echo off
echo.
echo Compiling shaders...
C:/VulkanSDK/1.1.121.2/Bin/glslc.exe ./../../resources/shaders/test.vert -o ./../../resources/shaders/vert.spv
C:/VulkanSDK/1.1.121.2/Bin/glslc.exe ./../../resources/shaders/test.frag -o ./../../resources/shaders/frag.spv
echo Done.
echo.
@echo on