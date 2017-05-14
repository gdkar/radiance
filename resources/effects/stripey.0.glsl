// Vertical stripes with a twinkle effect

void main(void) {
    float xv = round(uv.x * 20. * aspectCorrection.x); 
    fragColor = texture2D(iChannel[0], uv);
    fragColor.a *= exp(-iIntensity / 20.);

    if (rand(vec2(xv, iTime)) < exp(-iIntensity * 4.)) {
        fragColor = texture2D(iFrame, uv);
    }
}
