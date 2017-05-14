void main(void) {
    fragColor = texture2D(iChannel[1], (uv - 0.5) * 0.99 + 0.5);
    fragColor.a *= exp(-1 / 20.);
    if (rand(vec3(uv, iTime)) < exp((iIntensity - 2.) * 4.))
        fragColor = vec4(1.);
}
