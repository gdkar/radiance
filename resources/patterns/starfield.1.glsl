void main(void) {
    f_color0 = textureChannel(1,v_uv);
    f_color0.a *= exp(-1 / 20.);
    if (rand(vec3(v_uv , iTime)) < exp((iIntensity - 2.) * 4.))
        f_color0 = vec4(1.);
}
