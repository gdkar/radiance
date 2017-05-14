// Reduce alpha

void main(void) {
    fragColor = texture2D(iFrame, uv);
    fragColor.a *= (1. - iIntensity);
}
