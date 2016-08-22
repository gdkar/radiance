// Vertical stripes with a twinkle effect

void main(void) {
    float xv = round(v_uv.x * 20); 
    f_color0 = textureChannel(0, v_uv);
    f_color0.a *= exp(-iIntensity / 20.);

    if (rand(vec2(xv, iTime)) < exp(-iIntensity * 4.))
        f_color0 = textureFrame( v_uv);
}
