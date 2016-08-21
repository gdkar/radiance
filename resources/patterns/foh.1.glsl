void main(void) {
    vec4 prev = texture(iArray,vec3(v_uv,iChannelLayers[1]));
    vec4 next = texture(iArray,vec3(v_uv,iFrameLayer));

    float t = pow(2, round(6 * iIntensity - 4));
    float a = float((iIntensity >= 0.09) && (mod(iTime,t) >= max(t /15.,0.05)));
    f_color0 = mix(next, prev, a);
}
