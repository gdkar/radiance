// Snowcrash: white static noise

void main(void) {
    fragColor = texture2D(iFrame, uv);
    float x = rand(vec3(uv * iResolution, iTime));
    vec4 c = vec4(x, x, x, 1.0);
    fragColor = mix(fragColor, c, iIntensity);
}
