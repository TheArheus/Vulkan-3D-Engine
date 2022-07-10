@echo off
if not defined DevEnvDir (
   call vcvarsall amd64
)

setlocal EnableDelayedExpansion

set cFileNames=
for /R %%f in (*.c) do (
    set cFileNames=!cFileNames! %%f
)

set Assembly=testbet
set CompilerFlags=-g
set IncludeFlags=-Isrc -I../engine/src
set LinkerFlags=-L../../build -lengine.lib
set Defines=-D_MBCS -DVENG_IMPORT -D_DEBUG=1

clang %cFileNames% %CompilerFlags% -o ../../build/%Assembly%.exe %Defines% %IncludeFlags% %LinkerFlags%
