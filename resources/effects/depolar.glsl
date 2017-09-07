// Convert rings to vertical lines

void main(void) {
    vec2 xy = gl_FragCoord.xy / iResolution;
    float angle  = xy.y * M_PI * 1.0;
    vec2 rtheta = xy.x * sqrt(2.) * vec2(sin(angle), -cos(angle));
    rtheta = (rtheta + 1.) / 2.;

    vec2 uv = mix(xy, rtheta, iIntensity);

    gl_FragColor = texture2D(iInput, uv);
}
