void main(void) {
    vec2 normCoord = 2. * (uv - 0.5) * aspectCorrection;
    fragColor = vec4(abs(normCoord), 0., 1.);
}
