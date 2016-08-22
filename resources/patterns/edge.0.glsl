// Spatial edge detect filter (HPF)

void main(void) {
    const int d = int(0.05 * 300);
    vec4 center = textureFrame( v_uv);
    vec4 left = textureFrameOffset( v_uv, - ivec2(d, 0));
    vec4 right = textureFrameOffset( v_uv, ivec2(d, 0));
    vec4 up = textureFrameOffset( v_uv,  ivec2(0, d));
    vec4 down = textureFrameOffset(v_uv, - ivec2(0, d));
    left.rgb *= left.a;
    right.rgb *= right.a;
    up.rgb *= up.a;
    down.rgb *= down.a;
    vec4 outc = abs(left - right) + abs(up - down);
    f_color0 = clamp(outc * 1.5, 0, 1);
    f_color0.a = center.a;
    f_color0 = mix(center, f_color0, vec4(iIntensity));
}
