// Change the color (in HSV) to red

void main(void) {
    vec4 c = texture2D(iFrame, uv);
    fragColor.r = mix(c.r, (c.r + c.g + c.b) / 3., iIntensity);
    fragColor.g = c.g * (1. - iIntensity);
    fragColor.b = c.b * (1. - iIntensity);
    fragColor.a = c.a;
}
