// Pixelate/quantize the output

void main(void) {
    vec2 uv = v_uv - 0.5;

    float bins = 256. * pow(2, -9. * iIntensity);

    uv = round(uv * bins) / bins;
    uv += 0.5;

    f_color0 = textureFrame( uv);
}
