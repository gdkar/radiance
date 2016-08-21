// First order (expontential) hold

void main(void) {

    vec4 prev = texture(iArray,vec3(v_uv,iChannelLayers[0]));
    vec4 next = texture(iArray,vec3(v_uv,iChannelLayers[1]));

/*
    float t = pow(2, round(6 * iIntensity - 4));
    float a = 0.98;
    if (iIntensity < 0.09 || mod(iTime, t) < 0.1)
        f_color0 = next;
        */

    f_color0 = mix(next, prev, (iIntensity >= 0.05 ? pow(iIntensity, 0.25) : 0.));
    f_color0 = clamp(f_color0, 0, 1);
}
