#version 430

//in  flat vec2 v_corner;
in  flat vec2 v_size;
in      vec2 v_uv;
in  flat int v_layer;
out layout(location = 0) vec4 f_color0;

uniform layout(location = 3) float iIntensity;

#define M_PI 3.1415926535897932384626433832795

//
// The following is only used for UI; not for patterns
//
uniform bool iLeftOnTop;
uniform bool iSelection;
uniform ivec3 iSelector;
uniform int  iIndicator;
#define iLeftDeckSelector iSelector.x
#define iRightDeckSelector iSelector.y
#define iSelected iSelector.z

uniform sampler1D iSpectrum;
uniform sampler1D iWaveform;
uniform sampler1D iBeats;
uniform sampler2D iFrameLeft;
uniform sampler2D iFrameRight;
uniform sampler2D iStrips;
uniform sampler2D iTexture;
uniform sampler2D iText;

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

float rounded_rect_df(vec2 coord, vec2 center, vec2 size, float radius);
vec3 dataColor(ivec3 data);
float inBox(vec2 coord, vec2 bottomLeft, vec2 topRight);
float smoothBox(vec2 coord, vec2 bottomLeft, vec2 topRight, float width);
vec4 fancy_rect(vec2,vec2,vec2,bool);

const float RADIUS=25.;
const vec2  PAT_SIZE = vec2(45., 75.);

float sdCapsule(vec2 p, vec2 a, vec2 b, vec2 r);
void  glow(vec2 frag,vec2 p, inout vec4 color);

struct slider_params {
    vec4    track_color;
    vec4    handle_color;
    vec2    corner;
    vec2    size;
    float   track_weight;
    float   handle_size;
    float   range_min;
    float   range_max;

};

void make_slider(
    inout vec4       color
  , in slider_params params
  , in float         value
  );
#line 0
