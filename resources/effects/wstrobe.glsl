// White strobe to the beat

void main(void) {
    gl_FragColor = texture2D(iInput, uv);
    vec4 c;

    float freq;
    if(iIntensity < 0.05) freq = 0.;
    else if(iIntensity < 0.25) freq = 4.;
    else if(iIntensity < 0.45) freq = 2.;
    else if(iIntensity < 0.65) freq = 1.;
    else if(iIntensity < 0.85) freq = 0.5;
    else freq = 0.25;

    if(freq > 0) {
        vec3 hsv = rgb2hsv(demultiply(gl_FragColor).rgb);
        hsv.y = hsv.y * (1. - sawtooth(iTime / freq, 0.1));
        gl_FragColor.rgb = hsv2rgb(hsv);
        gl_FragColor = premultiply(gl_FragColor);
    }
}
