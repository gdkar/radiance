// Pixels radiating from the center

void main(void) {
    f_color0 = texture(iArray,vec3(v_uv,iFrame));
    vec4 c = texture(iArray,vec3(v_uv,iChannelLayers[1]));
    c.a *= smoothstep(0., 0.2, iIntensity);
    f_color0 = composite(f_color0, c);
}
