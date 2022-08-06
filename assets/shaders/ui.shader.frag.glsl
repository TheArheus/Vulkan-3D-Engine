#version 450
#extension GL_EXT_debug_printf : enable

layout(location = 0) out vec4 OutColor;

layout(set = 1, binding = 0) uniform local_uniform_object
{
    vec4 DiffuseColor;
} ObjectUBO;
layout(set = 1, binding = 1) uniform sampler2D DiffuseSampler;

layout(location = 1) in struct in_data_tex_coord
{
    vec2 TexCoord;
} InDataTexCoord;

void main()
{
    OutColor = ObjectUBO.DiffuseColor * texture(DiffuseSampler, InDataTexCoord.TexCoord);
}
