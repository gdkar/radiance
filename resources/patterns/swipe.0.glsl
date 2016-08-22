// Only update a vertical slice that slides across

void main(void) {
    vec4 prev = textureChannel(0,, v_uv);
    vec4 next = textureFrame( v_uv);
    float factor = pow(iIntensity, 2.0);

    float t = mod(iTime / 4.0 - v_uv.x , 1.0);
    float x = ( t < 0.5 ? pow(0.5-t,3.0) : 0.0);

    factor = clamp(min(factor, 1.0 - x), 0.0, 1.0);

    f_color0 = mix(next, prev, factor);
}
