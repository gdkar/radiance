void main(void) {
    f_color0 = texture(iArray,vec3((v_uv - 0.5) * 0.99 + 0.5, iChannelLayers[1])  );
    f_color0.a *= exp(-1 / 20.);
    if (rand(vec3(v_uv , iTime)) < exp((iIntensity - 2.) * 4.))
        f_color0 = vec4(1.);
}
