// Black sine wave from left to right.

void main(void) {
    float x = (v_uv.x + v_uv.y) * 15 + iTime * 1;
    f_color0 = texture(iArray,vec3(v_uv,iFrameLayer));
    f_color0.a *= 1.0 - iIntensity * (sin(x) / 2.0 + 0.5);
}
