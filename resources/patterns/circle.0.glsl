// Yellow blob that spins to the beat

void main(void) {
    float t = iTime / 4.0;
    vec2 center = vec2(sin(t), cos(t));
    //center *= 0.5;
    center *= iAudioLevel * 0.9 + 0.1;
    center += 0.5;

    vec4 c = vec4(1., 1., 0., 1.);
    c.a = clamp(length(center - v_uv), 0, 1);
    c.a = pow(c.a, iAudioHi * 3 + 0.1);
    c.a = 1.0 - c.a;
    c.a *= iIntensity;

    f_color0 = composite(texture(iArray,vec3(v_uv,iFrameLayer)),c);
}
