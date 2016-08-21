
layout(binding=3) buffer Spectrum {
    float data[];
} spectrum;
void main(void) {
    float g = v_uv.y * 0.5 + 0.1;
    float w = 4.;
    vec2 frag = v_size * v_uv;
    f_color0 = vec4(0.);
    f_color0 = composite(f_color0, fancy_rect(frag,vec2(30., 30.), vec2(260., 140.), false));

    float df = max(rounded_rect_df(frag,vec2(30., 30.), vec2(260., 140.), 25.), 0.);

    float shrink_freq = 190. / 200.;
    float shrink_mag = 90. / 100.;
    float freq = (v_uv.x - 0.5) / shrink_freq + 0.5;
    float mag = (v_uv.y - 0.5) * shrink_mag + 0.5;
    float d = (spectrum.data[uint(spectrum.data.length() * freq)] - mag) * 90.;
    float a = smoothstep(0., 1., d) * (1. - step(1., df));
    float gb = 0.5 * clamp(0., 1., d / 30.);
    f_color0 = composite(f_color0, vec4(1., gb, gb, a));
    f_color0 = composite(f_color0, vec4(0.3, 0.3, 0.3, smoothstep(0., 1., df) - smoothstep(2., 5., df)));
//    if(f_color0.a < 0.5)
//        discard;
}
