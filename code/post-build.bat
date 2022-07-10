@echo off

if not exist ..\build\shaders\ mkdir ..\build\shaders\

glslc.exe -fshader-stage=vert ..\assets\shaders\material.shader.vert.glsl -o ..\build\shaders\material.shader.vert.spv
glslc.exe -fshader-stage=frag ..\assets\shaders\material.shader.frag.glsl -o ..\build\shaders\material.shader.frag.spv

xcopy "..\assets" "..\build\assets" /h /i /c /k /e /r /y