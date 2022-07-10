@echo off
if not defined DevEnvDir (
    call vcvarsall amd64
)

setlocal EnableDelayedExpansion

set CurDir="%cd%"
set Includes="%VCInclude%%INCLUDE%"

set LIB_VCPKG="F:\Env\vcpkg\installed\x64-windows\lib"
set INC_VCPKG="F:\Env\vcpkg\installed\x64-windows\include"

set INC_VULKAN="C:\VulkanSDK\1.2.189.2\Include"
set LIB_VULKAN="C:\VulkanSDK\1.2.189.2\Lib"

set cFileNames=
for /R %%f in (*.c) do (
    set cFileNames=!cFileNames! "%%f"
)

echo %CurDir%

set CommonCompileFlags=-MT -nologo -fp:fast -EHa- -Od -WX- -W4 -Oi -GR- -Gm- -GS -D_DEBUG=1 -D_MBCS -DVENG_EXPORT -wd4100 -wd4201 -wd4505 -FC -Z7 /I%INC_VCPKG%
set CommonLinkFlags=-opt:ref -incremental:no /SUBSYSTEM:windows /LIBPATH:%LIB_VCPKG%  

cl %CommonCompileFlags% /I%CurDir% /I%Includes% /I%INC_VULKAN% %cFileNames% user32.lib winmm.lib gdi32.lib vulkan-1.lib /Fe"..\..\build\engine" /Fd"..\..\build\engine" -LD /link %CommonLinkFlags% /LIBPATH:%LIB_VULKAN%
