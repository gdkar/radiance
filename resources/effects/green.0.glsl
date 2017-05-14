// Zero out the green channel (green is not a creative color)

void main(void) {
    fragColor = texture2D(iFrame, uv);
    fragColor.r *= 1. - iIntensity;
    fragColor.b *= 1. - iIntensity;
}
