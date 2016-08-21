#version 330

#extension GL_ARB_explicit_attrib_location: enable
#extension GL_ARB_explicit_uniform_location: enable
layout(location = 0) in vec2 a_corner;
layout(location = 1)in vec2  a_size;
layout(location = 2) in int a_layer;
layout(location = 3) in int a_pid;
flat out vec2 vg_corner;
flat out vec2 vg_size;
flat out int  vg_layer;
flat out int  vg_pid;
void main()
{
    vg_corner = a_corner;
    vg_size   = a_size;
    vg_layer  = a_layer;
    vg_pid    = a_pid;
}
