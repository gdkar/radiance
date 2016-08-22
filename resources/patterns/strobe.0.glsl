// Strobe alpha to the beat

void main(void) {
    f_color0 = textureFrame( v_uv);
    vec4 c;

    float freq = step(0.05, iIntensity) * 2. - step(0.45, iIntensity);

    if(freq > 0) {
        f_color0.a *= 1. - ((1. - sawtooth(iTime / freq, 0.2)) * iIntensity * min(3. * iAudioLevel, 1.));
    }
}
