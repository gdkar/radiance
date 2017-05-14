// Reduce number of colors

void main(void) {
    fragColor = texture2D(iFrame, uv);

    //float bins = 256. * pow(2, -8. * iIntensity);
    float bins = min(256., 1. / iIntensity);

    fragColor = round(fragColor * bins) / bins;
}
