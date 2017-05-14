void main(void) {

    fragColor = texture2D(iChannel[1], (uv - 0.5) * 0.98 + 0.5);
    fragColor.a *= exp((iIntensity - 2.) / 50.) * smoothstep(0, 0.01, length((uv - 0.5) * aspectCorrection));

    vec4 c = texture2D(iFrame, uv);
    float s = smoothstep(0.90, 1., 1. - mod(iTime, 1.)) * iAudioLevel;
    c.a *=  min(3. * s, 1.);
    fragColor = composite(fragColor, c);
}
