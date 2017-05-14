void main(void) {
    float t = pow(2, round(6 * iIntensity - 4));
    float a = 0.98;
    if (iIntensity < 0.09 || mod(iTime, t) < 0.1)
        fragColor = texture2D(iFrame, uv);
    else
        fragColor = texture2D(iChannel[1], uv);
}
