#version 130
in vec2 uv;
uniform lowp float iParameter;
uniform sampler2D iLeft;
uniform sampler2D iRight;
out vec4 fragColor;
// Alpha-compsite two colors, putting one on top of the other
vec4 composite(vec4 under, vec4 over) {
    float a_out = 1. - (1. - over.a) * (1. - under.a);
    return clamp(vec4((over.rgb * over.a  + under.rgb * under.a * (1. - over.a)) / a_out, a_out), vec4(0.), vec4(1.));
}

void main() {
    vec4 l = texture2D(iLeft, uv);
    vec4 r = texture2D(iRight,uv);
    fragColor = l * (1. - iParameter) + r * iParameter;
}
