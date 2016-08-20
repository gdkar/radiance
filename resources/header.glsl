#version 330
#extension GL_ARB_explicit_attrib_location: enable
#extension GL_ARB_explicit_uniform_location: enable

flat in vec2 v_corner;
flat in  vec2 v_size;
in      vec2 v_uv;
flat in int  v_layer;
layout(location = 0) out vec4 f_color0;

// Time, measured in beats. Wraps around to 0 every 16 beats, [0.0, 16.0)
layout(location = 1) uniform float iTime;
// Audio levels, high/mid/low/level, [0.0, 1.0]
layout(location = 2) uniform vec4 iAudio;

#define iAudioLow iAudio.x
#define iAudioMid iAudio.y
#define iAudioHi  iAudio.z
#define iAudioLevel iAudio.w

// Intensity slider, [0.0, 1.0]

layout(location = 3) uniform float iIntensity;

// Intensity slider integrated with respect to wall time mod 1024, [0.0, 1024.0)
layout(location = 4) uniform float iIntensityIntegral;

// (Ideal) output rate in frames per second
layout(location = 5) uniform float iFPS;

// Output of the previous pattern
layout(location = 6) uniform sampler2D iFrame;

// Previous outputs of the other channels (e.g. foo.1.glsl) 
layout(location = 7) uniform sampler2D iChannel[3];

layout(location = 10) uniform sampler2DArray iAllPatterns;
layout(location = 11) uniform int  iPatternIndex;
#define M_PI 3.1415926535897932384626433832795

//
// The following is only used for UI; not for patterns
//
// Utilities to convert from an RGB vec3 to an HSV vec3
vec3 rgb2hsv(vec3 c);
vec3 hsv2rgb(vec3 c);
// Alpha-compsite two colors, putting one on top of the other
vec4 composite(vec4 under, vec4 over);
// Sawtooth wave
float sawtooth(float x, float t_up);
// Predictable randomness
float rand(float c);
float rand(vec2 c);
float rand(vec3 c);
float noise(float p);
float noise(vec2 p);
float noise(vec3 p);

#define RADIUS (25.)
#line 0
