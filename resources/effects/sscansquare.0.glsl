// White slit for testing

void main(void) {
    vec2 normCoord = (uv - 0.5) * aspectCorrection;
    float x = iIntensity;
    vec4 color = vec4(1.0, 1.0, 1.0, 1. - step(0.5 * onePixel, abs(x - 0.5 - normCoord)));
    fragColor = texture2D(iFrame, uv);
    fragColor = composite(fragColor, color);
}
