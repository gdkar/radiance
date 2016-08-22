// White wave with hard edges

void main(void) {
    float xpos = iIntensityIntegral * 1.5;
    float xfreq = (iIntensity + 0.5) * 2.;
    float x = mod(v_uv.x * xfreq + xpos, 1.);
    f_color0 = textureFrame( v_uv);
    f_color0 = composite(f_color0, vec4(1., 1., 1., step(x, 0.3) * smoothstep(0., 0.5, iIntensity)));
}
