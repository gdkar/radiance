import QtQuick 2.8
import radiance 1.0

GraphicalDisplay {
    implicitWidth: 300;
    implicitHeight: 100;

    fragmentShader: "
#version 130
        // Alpha-compsite two colors, putting one on top of the other
        vec4 composite(vec4 under, vec4 over) {
            float a_out = 1. - (1. - over.a) * (1. - under.a);
            return clamp(vec4((over.rgb * over.a  + under.rgb * under.a * (1. - over.a)) / a_out, a_out), vec4(0.), vec4(1.));
        }
        in vec2 uv;
        uniform sampler1D iSpectrum;
        uniform vec2 iResolution;
        out vec4 fragColor;
        void main(void) {
            float g = uv.y * 0.5 + 0.1;
            float w = 4.;

            fragColor = vec4(0.);

            float freq = (uv.x - 0.5) + 0.5;
            float mag = (0.5 - uv.y) + 0.5;
            float d = (texture1D(iSpectrum, freq).r - mag) * 90.;
            float a = smoothstep(0., 1., d);
            float gb = 0.5 * clamp(0., 1., d / 30.);
            fragColor = composite(fragColor, vec4(1., gb, gb, a));
            fragColor.rgb *= fragColor.a;
        }"
}
