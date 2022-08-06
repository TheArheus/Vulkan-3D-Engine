#version 450
#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec3 InPosition;
layout(location = 1) in vec2 InTexCoord;

layout(set = 0, binding = 0) uniform global_uniform_object
{
    mat4 Projection;
    mat4 View;
} GlobalUBO;

layout(push_constant) uniform constants
{
    mat4 Model;
} PushConstants;

layout(location = 0) out int OutMode;

layout(location = 1) out struct out_data_tex_coord
{
    vec2 TexCoord;
} OutDataTexCoord;

void main()
{
    OutDataTexCoord.TexCoord = InTexCoord;
    gl_Position = GlobalUBO.Projection * GlobalUBO.View * PushConstants.Model * vec4(InPosition, 1.0);
}
