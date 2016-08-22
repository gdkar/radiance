// Pixels radiating from the center

void main(void) {
    f_color0 = textureFrame(v_uv);
    vec4 c = textureChannel(1,v_uv);
    c.a *= smoothstep(0., 0.2, iIntensity);
    f_color0 = composite(f_color0, c);
}
