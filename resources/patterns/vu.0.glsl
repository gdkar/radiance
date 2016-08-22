// Blue vertical VU meter

void main(void) {
    f_color0 = textureFrame( v_uv);

    float SMOOTH = 0.03;

    vec3 audio = vec3(iAudioLow, iAudioMid, iAudioHi);

    audio = audio * 2. * iIntensity;

    vec3 draw = 1. - smoothstep(audio - SMOOTH, audio, vec3(abs(v_uv.x - 0.5)));

    vec4 c = composite(composite(vec4(0., 0., 0.5, draw.x), vec4(0., 0., 1., draw.y)), vec4(0.3, 0.3, 1., draw.z));
    c = clamp(c, 0., 1.);
    f_color0 = composite(f_color0, c);
}
