// Desaturate (make white)

void main(void) {
    float factor = pow(iIntensity, 3.);

    vec4 samp = texture2D(iInput, uv);
    vec3 hsl = rgb2hsv(samp.rgb);
    hsl.g *= 1.0 - factor;
    gl_FragColor.rgb = hsv2rgb(hsl);
    gl_FragColor.a = samp.a;
    gl_FragColor = premultiply(gl_FragColor);
}
