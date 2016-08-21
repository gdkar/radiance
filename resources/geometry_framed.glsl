#version 330
#extension GL_ARB_explicit_attrib_location: enable
#extension GL_ARB_explicit_uniform_location: enable

layout(points) in;
layout(triangle_strip, max_vertices=4) out;
layout(location = 12) uniform vec2 u_over_size = vec2(1.1,1.1);
layout(location = 0) uniform vec2 u_global_size;
flat in vec2 vg_corner[];
flat in vec2 vg_size  [];
flat in int  vg_layer [];
flat in int  vg_pid   [];
flat out vec2 v_corner;
flat out vec2 v_size;
flat out int v_layer;
flat out int v_pid;
out vec2 v_uv;

void main()
{
    vec2 size   = vg_size[0]  * 2/ u_global_size;
    vec2 corner = vg_corner[0] * 2 / u_global_size - vec2(1);
    int _layer = vg_layer[0];
    vec2 _corner= vg_corner[0];
    vec2 _size  = vg_size[0];
    {
        v_pid    = vg_pid[0];
        v_layer  = _layer;
        gl_Layer = _layer;
        v_corner = _corner;
        v_size   = _size;
        v_uv     = vec2(0,0) + ( 1. - u_over_size);
        gl_Position = vec4(corner + size * v_uv,0,1);
        EmitVertex();
    }
    {
        v_pid    = vg_pid[0];
        v_layer  = _layer;
        gl_Layer = _layer;
        v_corner = _corner;
        v_size   = vg_size[0];
        v_uv     = vec2(1. - u_over_size.x, u_over_size.y) ;
        gl_Position = vec4(corner + size * v_uv,0,1);
        EmitVertex();
    }
    {
        v_pid    = vg_pid[0];
        v_corner = _corner;
        gl_Layer = _layer;
        v_layer  = _layer;
        v_size   = _size;
        v_uv     = vec2(u_over_size.x,1. - u_over_size.y) ;
        gl_Position = vec4(corner + size * v_uv,0,1);
        EmitVertex();
    }
    {
        v_pid    = vg_pid[0];
        v_layer  = _layer;
        gl_Layer = _layer;
        v_corner = _corner;
        v_size   = _size;
        v_uv     = vec2(1,1) * u_over_size;
        gl_Position = vec4(corner + size * v_uv,0,1);
        EmitVertex();
    }
    EndPrimitive();
}
