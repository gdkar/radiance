// Per-pixel twinkle effect

void main(void) {
    fragColor = texture2D(iChannel[0], uv);
    fragColor.a *= exp(-iIntensity / 20.);
    if (rand(vec3(uv, iTime)) < exp(-iIntensity * 4.)) {
        fragColor = texture2D(iFrame, uv);
    }
}
