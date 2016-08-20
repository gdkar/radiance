#version 330
#extension GL_ARB_explicit_attrib_location: enable
#extension GL_ARB_explicit_uniform_location: enable

layout(location = 0) uniform vec2 u_global_size;
layout(location = 0) in vec2 a_position;

out vec2 v_uv;

void main()
{
    v_uv = a_position / u_global_size;
    gl_Position = vec4((v_uv * 2) - 1, 0, 1);
}
