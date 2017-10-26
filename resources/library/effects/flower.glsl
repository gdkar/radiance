#property description Convert vertical lines to radial flower pattern

void main(void) {
    vec2 xy_cent = 2. * uv - 1.;
    float angle = atan(xy_cent.y, xy_cent.x);
    float n_sides = (iIntensity * 7.) + 1.;
    float arc = 2. * M_PI / n_sides;
    float a1 = mod(angle, arc);
    //float lengthFactor = sqrt(2.);
    float lengthFactor = 1.0;
    float corr = 1. / (pow(cos(a1 - arc / 2.), 2.) * lengthFactor * cos(arc / 2.));

    vec2 rtheta = vec2(length(xy_cent) * corr, 0.5 + angle / (2. * M_PI));
    vec2 uv2 = mix(uv, rtheta, clamp(iIntensity * 5., 0., 1.));

    fragColor = texture(iInput, uv2);
}