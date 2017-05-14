// Pink polka dots

void main(void) {
    fragColor = texture2D(iFrame, uv);
    vec4 c;

    float r = 0.2;

    vec2 normCoord = (uv - 0.5) * aspectCorrection;

    c = vec4(1., 0.5, 0.5, 1. - smoothstep(r - onePixel, r, length(mod(normCoord * 5. * iIntensity - 0.5, 1.) - 0.5)));
    c.a *= smoothstep(0., 0.1, iIntensity);
    fragColor = composite(fragColor, c);
}
