// Radiate color from the center based on audio

void main(void) {
    f_color0 = texture(iArray, vec3(v_uv,iFrameLayer));
    vec4 c = texture(iArray,vec3(v_uv,iChannelLayers[1]));
    c.a *= smoothstep(0., 0.2, iIntensity);
    f_color0 = composite(c, f_color0);
}
