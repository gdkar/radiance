// Radiate color from the center based on audio

void main(void) {
    fragColor = texture2D(iFrame, uv);
    vec4 c = texture2D(iChannel[1], uv);
    c.a *= smoothstep(0., 0.2, iIntensity);
    fragColor = composite(c, fragColor);
}
