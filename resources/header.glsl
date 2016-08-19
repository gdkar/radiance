#version 430

in flat vec2 v_corner;
in flat vec2 v_size;
in      vec2 v_uv;
in flat float v_layer;
out layout(location = 0) vec4 f_color0;

// Time, measured in beats. Wraps around to 0 every 16 beats, [0.0, 16.0)
uniform layout(location = 1) float iTime;
// Audio levels, high/mid/low/level, [0.0, 1.0]
uniform layout(location = 2) vec4 iAudio;

#define iAudioLow iAudio.x
#define iAudioMid iAudio.y
#define iAudioHi  iAudio.z
#define iAudioLevel iAudio.w

// Intensity slider, [0.0, 1.0]

uniform layout(location = 3) float iIntensity;

// Intensity slider integrated with respect to wall time mod 1024, [0.0, 1024.0)
uniform layout(location = 4) float iIntensityIntegral;

// (Ideal) output rate in frames per second
uniform layout(location = 5) float iFPS;

// Output of the previous pattern
uniform layout(location = 6) sampler2D iFrame;

// Previous outputs of the other channels (e.g. foo.1.glsl) 
uniform layout(location = 7) sampler2D iChannel[3];

uniform layout(location = 10) sampler2DArray iAllPatterns;
uniform layout(location = 11) int  iPatternIndex;
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
