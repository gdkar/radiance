// Radiate color from the center based on audio

void main(void) {
    f_color0 = texture(iFrame, v_uv);
    vec4 c = texture(iChannel[1], v_uv);
    c.a *= smoothstep(0., 0.2, iIntensity);
    f_color0 = composite(c, f_color0);
}
