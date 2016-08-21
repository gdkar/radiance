void main(void) {

    float left_alpha = min((1. - iIntensity) * 2., 1.);
    float right_alpha = min(iIntensity * 2., 1.);

    vec4 left = texture(iArray,vec3(v_uv,iFrameLeft));
    vec4 right = texture(iArray,vec3(v_uv,iFrameRight));

    left.a *= left_alpha;
    right.a *= right_alpha;

    f_color0 = iLeftOnTop ? composite(right, left) : composite(left,right);
}
