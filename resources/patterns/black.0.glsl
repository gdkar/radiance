// Reduce alpha

void main(void) {
    f_color0 = texture(iArray,vec3( v_uv,iFrameLayer));
    f_color0.a *= (1. - iIntensity);
}
