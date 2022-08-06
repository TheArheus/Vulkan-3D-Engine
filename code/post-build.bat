@echo off

glslc.exe -fshader-stage=vert ..\assets\shaders\material.shader.vert.glsl -o ..\assets\shaders\material.shader.vert.spv
glslc.exe -fshader-stage=frag ..\assets\shaders\material.shader.frag.glsl -o ..\assets\shaders\material.shader.frag.spv

glslc.exe -fshader-stage=vert ..\assets\shaders\ui.shader.vert.glsl -o ..\assets\shaders\ui.shader.vert.spv
glslc.exe -fshader-stage=frag ..\assets\shaders\ui.shader.frag.glsl -o ..\assets\shaders\ui.shader.frag.spv