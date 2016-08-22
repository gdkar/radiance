// Cycle the color (in HSV) over time

void main(void) {
    f_color0 = textureFrame( v_uv);

    float deviation;
    deviation = mod(iIntensityIntegral, 1.);

    vec3 hsv = rgb2hsv(f_color0.rgb);
    hsv.r = mod(hsv.r + 1. + deviation, 1.);
    f_color0.rgb = mix(f_color0.rgb, hsv2rgb(hsv), smoothstep(0, 0.2, iIntensity));
}
