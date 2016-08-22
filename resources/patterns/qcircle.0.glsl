// Big purple soft circle 

void main(void) {
    float t = iTime / 4.0;
    vec2 center = vec2(sin(t), cos(t));
    //center *= 0.5;
    center *= 0.1;
    center += 0.5;

    vec4 c = vec4(0.2, 0.1, 0.5, 1.);
    c.a = clamp(length(center - v_uv), 0, 1);
    c.a = pow(c.a, 2);
    c.a = 1.0 - c.a;
    c.a *= iIntensity;

    f_color0 = composite(textureFrame( v_uv), c);
}
