// Apply smoothing over time with new hits happening instantly

void main(void) {
    vec4 prev = texture2D(iChannel[0], uv);
    vec4 next = texture2D(iFrame, uv);
    fragColor.rgb = next.rgb;
    if (next.a > prev.a) {
        fragColor = next;
    } else {
        prev.a *= pow(iIntensity, 0.1);
        fragColor = composite(next, prev);
    }
    fragColor.a = clamp(fragColor.a, 0, 1);
    
}
