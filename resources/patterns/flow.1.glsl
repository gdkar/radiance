void main(void) {

    f_color0 = texture(iArray,vec3((v_uv - 0.5) * 0.98 + 0.5,iChannelLayers[1]));
    f_color0.a *= exp((iIntensity - 2.) / 50.) * smoothstep(0, 0.01, length(v_uv - 0.5));

    vec4 c = texture(iArray,vec3(v_uv,iFrameLayer));
    float s = smoothstep(0.90, 1., 1. - mod(iTime, 1.)) * iAudioLevel;
    c.a *=  min(3. * s, 1.);
    f_color0 = composite(f_color0, c);
}
