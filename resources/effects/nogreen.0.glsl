// Zero out the green channel (green is not a creative color)

void main(void) {
    fragColor = texture2D(iFrame, uv);
    fragColor.g *= 1. - iIntensity;
}
