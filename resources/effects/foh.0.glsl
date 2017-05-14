// First order (expontential) hold

void main(void) {
    vec4 prev = texture2D(iChannel[0], uv);
    vec4 next = texture2D(iChannel[1], uv);

/*
    float t = pow(2, round(6 * iIntensity - 4));
    float a = 0.98;
    if (iIntensity < 0.09 || mod(iTime, t) < 0.1)
        fragColor = next;
        */

    fragColor = mix(next, prev, pow(iIntensity, 0.4));
    fragColor = clamp(fragColor, 0, 1);
}
