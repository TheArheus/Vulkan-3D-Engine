@echo off
if not defined DevEnvDir (
    call vcvarsall amd64
)

setlocal EnableDelayedExpansion

set INC_VULKAN="C:\VulkanSDK\1.3.216.0\Include"
set LIB_VULKAN="C:\VulkanSDK\1.3.216.0\Lib"

set cFileNames=
for /R %%f in (*.c) do (
    set cFileNames=!cFileNames! "%%f"
)

set Assembly=engine
set CompilerFlags=-g -shared -Wall -Wvarargs -Wno-deprecated -Wno-missing-braces
set IncludeFlags=-Isrc -I../testbet/src/ -I%INC_VULKAN%
set LinkerFlags=-luser32 -lvulkan-1 -L%LIB_VULKAN%
set Defines=-D_MBCS -D_DEBUG -DVENG_EXPORT

clang %cFileNames% %CompilerFlags% -o ../../build/%Assembly%.dll %Defines% %IncludeFlags% %LinkerFlags%
