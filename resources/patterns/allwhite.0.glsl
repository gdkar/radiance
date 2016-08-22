// Basic white fill

void main(void) {
    vec4 c = vec4(1., 1., 1., iIntensity);
    f_color0 = composite(texture(iArray,vec3(v_uv,iFrameLayer)), c);
}
