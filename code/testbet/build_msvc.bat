@echo off
if not defined DevEnvDir (
    call vcvarsall amd64
)

setlocal EnableDelayedExpansion

set Includes="%VCInclude%%INCLUDE%"

set VEngInclude="..\engine"
set VEngLibrary="..\..\build"

set LIB_VCPKG="F:\Env\vcpkg\installed\x64-windows\lib"
set INC_VCPKG="F:\Env\vcpkg\installed\x64-windows\include"

set INC_VULKAN="C:\VulkanSDK\1.2.189.2\Include"
set LIB_VULKAN="C:\VulkanSDK\1.2.189.2\Lib"

set cFileNames=
for %%f in (*.c) do (
    set cFileNames=!cFileNames! %%f
)

echo %cFileNames%

set CommonCompileFlags=-MT -nologo -fp:fast -EHa -Od -WX- -W4 -Oi -GR- -Gm- -GS -DUNICODE -D_UNICODE -DVENG_IMPORT -D_DEBUG=1 -wd4100 -wd4201 -wd4505 -FC -Z7 /I%INC_VCPKG%
set CommonLinkFlags=-opt:ref -incremental:no /NODEFAULTLIB:msvcrt /SUBSYSTEM:CONSOLE /LIBPATH:%LIB_VCPKG%  

cl %CommonCompileFlags% /I%Includes% /I%VEngInclude% /I%INC_VULKAN% %cFileNames% user32.lib winmm.lib gdi32.lib engine.lib vulkan-1.lib /Fe"..\..\build\testbet" /Fd"..\..\build\testbet" /link %CommonLinkFlags% /LIBPATH:%LIB_VULKAN% /LIBPATH:%VEngLibrary%
