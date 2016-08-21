// Perlin noise green smoke

void main(void) {

    vec3 noise_input = vec3((v_uv + (noise(iIntensity * iTime * 0.01))) * iIntensity * 4, iIntensity + iIntensityIntegral * 0.1 ) + v_layer;
    float n = noise(noise_input) - 0.1;
    n += (noise(2. * noise_input) - 0.5) * 0.5;
    n += (noise(4. * noise_input) - 0.5) * 0.25;
    n += (noise(8. * noise_input) - 0.5) * 0.125;
    n += (noise(16. * noise_input) - 0.5) * 0.0625;
    n = n / 3.;

    float a = clamp(n * n * 5., 0., 1.) * smoothstep(0., 0.2, iIntensity);

    f_color0 = texture(iArray,vec3( v_uv,iFrameLayer));
    f_color0 = composite(f_color0, vec4(0., 1., 0., a));
}
