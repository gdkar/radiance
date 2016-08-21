// Organic purple waves

void main(void) {
    //mat2 rot = mat2(cos(iTime), -sin(iTime), sin(iTime), cos(iTime));
    vec4 c;

    float y = pow(sin(cos(iTime / 4) * v_uv.y * 8 + v_uv.x), 2);
    float x = mod(sin(v_uv.x * 4) + cos(v_uv.y * v_uv.x * 5) * (y * 0.2 + 0.8) + 3.0, 1.0);

    c.r = mix(x, y, 0.3);
    c.b = pow(mix(x, y, 0.7), 0.6);
    c.g = 0;
    c.a = iIntensity;

    f_color0 = composite(texture(iArray,vec3(v_uv,iFrameLayer)), c);
}
