// Convert vertical lines to rings

void main(void) {
    vec2 normCoord = 2. * (uv - 0.5) * aspectCorrection;

    vec2 newUV = vec2(length(normCoord) / sqrt(2.), abs(atan(normCoord.x, -normCoord.y) / M_PI)) - 0.5;
    newUV = newUV / aspectCorrection + 0.5;

    fragColor = texture2D(iFrame, mix(uv, newUV, iIntensity));
}
