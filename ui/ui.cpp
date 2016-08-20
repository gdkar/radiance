#include "util/common.h"
#include "util/opengl_debug.hpp"
#include "ui/ui.h"

#include "main.h"

static SDL_Window * window;
static SDL_GLContext context;
static SDL_Renderer * renderer;
static bool quit;
static GLuint main_shader;
static GLuint pat_shader;
static GLuint blit_shader;
static GLuint crossfader_shader;
static GLuint strip_shader;

static GLuint select_fb;
static GLuint strip_fb;
static GLuint select_tex;
static GLuint pattern_array;
static GLuint buf_spectrum_data;
static GLuint spectrum_shader;
static GLuint buf_waveform_data;
static GLuint buf_waveform_beats_data;
static GLuint waveform_shader;
static GLuint strip_texture;
static GLuint strip_vao      = 0;
static GLuint strip_vbo      = 0;
static int    strip_vbo_size = 0;
static GLuint vao;
static GLuint vbo;
static GLuint pat_vbo;
static GLuint pat_vao;
// Window
static int ww; // Window width
static int wh; // Window height

// Mouse
static int mx; // Mouse X
static int my; // Mouse Y
static int mcx; // Mouse click X
static int mcy; // Mouse click Y
static enum {MOUSE_NONE, MOUSE_DRAG_INTENSITY, MOUSE_DRAG_CROSSFADER} ma; // Mouse action
static int mp; // Mouse pattern (index)
static double mci; // Mouse click intensity

// Selection
static int selected = 0;

// Strip indicators
static enum {STRIPS_NONE, STRIPS_SOLID, STRIPS_COLORED} strip_indicator = STRIPS_NONE;

// False colors
#define HIT_NOTHING 0
#define HIT_PATTERN 1
#define HIT_INTENSITY 2
#define HIT_CROSSFADER 3
#define HIT_CROSSFADER_POSITION 4

// Mapping from UI pattern -> deck & slot
// TODO make this live in the INI file
static const int map_x[16] = {100, 275, 450, 625, 1150, 1325, 1500, 1675,
                              100, 275, 450, 625, 1150, 1325, 1500, 1675,};
static const int map_y[16] = {295, 295, 295, 295, 295, 295, 295, 295,
                              55, 55, 55, 55, 55, 55, 55, 55};
static const int map_pe_x[16] = {100, 275, 450, 625, 1150, 1325, 1500, 1675,
                                 100, 275, 450, 625, 1150, 1325, 1500, 1675};
static const int map_pe_y[16] = {420, 420, 420, 420, 420, 420, 420, 420,
                                180, 180, 180, 180, 180, 180, 180, 180};
static const int map_deck[16] = {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3};
static const int map_pattern[16] = {0, 1, 2, 3, 3, 2, 1, 0, 0, 1, 2, 3, 3, 2, 1, 0};
static const int map_selection[16] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

static const int crossfader_selection_top = 17;
static const int crossfader_selection_bot = 18;

//                                0   1   2   3   4   5   6   7   8   9  10   11  12  13  14  15  16  17  18
static const int map_left[19] =  {8,  1,  1,  2,  3,  17, 5,  6,  7,  9,  9,  10, 11, 18, 13, 14, 15, 4,  12};
static const int map_right[19] = {1,  2,  3,  4,  17, 6,  7,  8,  8,  10, 11, 12, 18, 14, 15, 16, 16, 5,  13};
static const int map_up[19] =    {1,  1,  2,  3,  4,  5,  6,  7,  8,  1,  2,  3,  4,  5,  6,  7,  8,  17, 17};
static const int map_down[19] =  {9,  9,  10, 11, 12, 13, 14, 15, 16, 9,  10, 11, 12, 13, 14, 15, 16, 18, 18};
static const int map_space[19] = {17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 17, 18};
static const int map_tab[19] =   {1,  2,  3,  4,  17, 17, 5,  6,  7,  10, 11, 12, 18, 18, 13, 14, 15, 17, 18};
static const int map_stab[19] =  {15, 1,  1,  2,  3,  6,  7,  8,  8,  9,  9,  10, 11, 14, 15, 16, 16, 17, 18};
static const int map_home[19] =  {1,  1,  1,  1,  1,  1,  1,  1,  1,  9,  9,  9,  9,  9,  9,  9,  9,  1,  9};
static const int map_end[19] =   {8,  8,  8,  8,  8,  8,  8,  8,  8, 16, 16, 16, 16, 16, 16, 16, 16,  8, 16};

// Font
embedded_renderer gl_font{};
embedded_renderer textbox_font{};
static const SDL_Color font_color = {255, 255, 255, 255};

// Pat entry
static bool pat_entry;
static char pat_entry_text[255];

// Timing
static double l_t;

// Deck selector
static int left_deck_selector = 0;
static int right_deck_selector = 1;

// Forward declarations
static void handle_text(const char * text);
//
static void bind_vao_fill_vbo(float x, float y, float w, float h)
{
    GLfloat vertices[] = { x, y, w, h };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    void *_vbo = glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(vertices), GL_MAP_INVALIDATE_BUFFER_BIT|GL_MAP_WRITE_BIT);
    memcpy(_vbo,vertices,sizeof(vertices));
    glUnmapBuffer(GL_ARRAY_BUFFER);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glBindVertexArray(vao);
}
static void fill(float w, float h) {
    bind_vao_fill_vbo(0., 0., w, h);
    glDrawArrays(GL_POINTS, 0, 1);
}

static void debug_callback(GLenum source, GLenum type, GLuint id,
    GLenum severity, GLsizei length, const char *message, const void *opaque)
{
    using std::to_string;
    if (get_loglevel() <= to_loglevel(severity)) {
        ::fprintf(stderr, "[%-24s] [GL DEBUG source=\"%s\",type=\"%s\"] ( id = %u ) \"%s\"\n", to_string(to_loglevel(severity)).c_str(),uiDebugSource(source),uiDebugType(type),id,message );
        fflush(stderr);
    }
}
void ui_init() {
    // Init SDL

    if(SDL_Init(SDL_INIT_VIDEO) < 0) FAIL("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK
      , SDL_GL_CONTEXT_PROFILE_CORE
        );
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_RELEASE_BEHAVIOR
      , SDL_GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH
        );
    SDL_GL_SetAttribute(
        SDL_GL_SHARE_WITH_CURRENT_CONTEXT
      , GL_FALSE
        );
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_FLAGS
      ,(int(get_loglevel()) <= int(LOGLEVEL_DEBUG)
            ? SDL_GL_CONTEXT_DEBUG_FLAG
            : 0)
       |SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG
        );
    ww = config.ui.window_width;
    wh = config.ui.window_height;

    window = SDL_CreateWindow("Radiance", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, ww, wh, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if(window == NULL) FAIL("Window could not be created: %s\n", SDL_GetError());
    context = SDL_GL_CreateContext(window);
    if(context == NULL) FAIL("OpenGL context could not be created: %s\n", SDL_GetError());
    if(SDL_GL_SetSwapInterval(1) < 0) fprintf(stderr, "Warning: Unable to set VSync: %s\n", SDL_GetError());

    SDL_GL_MakeCurrent(window,context);
    if(gl3wInit()) {
        FAIL("Could not initialize gl3w and load OpenGL functions.");
    }
    glDebugMessageCallback(debug_callback,nullptr);
    CHECK_GL();
    glEnable(GL_DEBUG_OUTPUT);
    glGenBuffers(1,&vbo);
    glGenBuffers(1,&pat_vbo);
    GLfloat pat_vbo_data[16 * 5];
    for(auto i = 0; i < 16; ++i) {
        pat_vbo_data[5*i+0] = map_x[i];
        pat_vbo_data[5*i+1] = map_y[i];
        pat_vbo_data[5*i+2] = config.ui.pattern_width;
        pat_vbo_data[5*i+3] = config.ui.pattern_height;
        pat_vbo_data[5*i+4] = i;
    }
    glBindBuffer(GL_ARRAY_BUFFER,pat_vbo);
    glBufferData(GL_ARRAY_BUFFER,sizeof(pat_vbo_data), pat_vbo_data,GL_STATIC_DRAW);
    glGenVertexArrays(1,&pat_vao);
    glBindVertexArray(pat_vao);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(4*sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glGenVertexArrays(1,&vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glNamedBufferData(vbo, sizeof(GLfloat) * 4, NULL, GL_DYNAMIC_DRAW);
    glBindVertexArray(vao);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // Init OpenGL
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0, 0, 0, 0);
    CHECK_GL();

    // Make framebuffers
    glGenFramebuffers(1, &select_fb);
    glGenFramebuffers(1, &strip_fb);
    CHECK_GL();

    // Init select texture
    select_tex = make_texture(ww,wh);

    pattern_array = make_texture(GL_RGBA32F, config.ui.pattern_width, config.ui.pattern_height, config.ui.n_patterns);

    glBindFramebuffer(GL_FRAMEBUFFER, select_fb);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, select_tex, 0);

    // Init pattern textures

    glGenBuffers(1,&buf_spectrum_data);
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf_spectrum_data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,buf_spectrum_data);
//    glNamedBufferStorage(buf_spectrum_data, config.audio.spectrum_bins * sizeof(GLfloat),NULL, GL_MAP_WRITE_BIT|GL_MAP_PERSISTENT_BIT);
    // Spectrum data texture
//    tex_spectrum_data = make_texture(config.audio.spectrum_bins);
    glGenBuffers(1,&buf_waveform_data);
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf_spectrum_data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,buf_waveform_data);
//    glNamedBufferStorage(buf_waveform_data, config.audio.waveform_length * 4 * sizeof(GLfloat),NULL, GL_MAP_WRITE_BIT|GL_MAP_PERSISTENT_BIT);
    glGenBuffers(1,&buf_waveform_beats_data);
//    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buf_spectrum_data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,buf_waveform_beats_data);
//    glNamedBufferStorage(buf_waveform_beats_data, config.audio.waveform_length * sizeof(GLfloat), NULL,GL_MAP_WRITE_BIT|GL_MAP_PERSISTENT_BIT);

    // Waveform data texture
///    tex_waveform_data = make_texture( config.audio.waveform_length);
//    tex_waveform_beats_data = make_texture( config.audio.waveform_length);
    // Waveform UI element

    // Strip indicators
    strip_texture = make_texture ( config.pattern.master_width, config.pattern.master_height);
    glBindFramebuffer(GL_FRAMEBUFFER, strip_fb);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, strip_texture, 0);

    // Done allocating textures & FBOs, unbind and check for errors
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if((main_shader = load_program({"#lib.glsl","#lib_ui.glsl"}, "#header_ui.glsl",{"#ui_main.glsl"})) == 0) FAIL("Could not load UI main shader!\n%s", get_load_program_error().c_str());
    if((pat_shader = load_program({"#lib.glsl","#lib_ui.glsl"},"#header_ui.glsl",{"#ui_pat.glsl"})) == 0) FAIL("Could not load UI pattern shader!\n%s", get_load_program_error().c_str());
    if((crossfader_shader = load_program({"#lib.glsl","#lib_ui.glsl"},"#header_ui.glsl",{"#ui_crossfader.glsl"})) == 0) FAIL("Could not load UI crossfader shader!\n%s", get_load_program_error().c_str());
    if((waveform_shader = load_program({"#lib.glsl","#lib_ui.glsl"},"#header_ui.glsl",{"#ui_waveform.glsl"})) == 0) FAIL("Could not load UI waveform shader!\n%s", get_load_program_error().c_str());
    if((spectrum_shader = load_program({"#lib.glsl","#lib_ui.glsl"},"#header_ui.glsl",{"#ui_spectrum.glsl"})) == 0) FAIL("Could not load UI spectrum shader!\n%s", get_load_program_error().c_str());
    if((strip_shader = load_program({"#strip.v.glsl"},{},{"#lib.glsl","#lib_ui.glsl"},"#header_ui.glsl",{"#strip.f.glsl"})) == 0) FAIL("Could not load strip indicator shader!\n%s", get_load_program_error().c_str());
    if((blit_shader = load_program({"#lib.glsl","#lib_ui.glsl"},"#header_ui.glsl",{"#blit.glsl"})) == 0) FAIL("Could not load blit shader!\n%s", get_load_program_error().c_str());
    glProgramUniform2f(waveform_shader,12,1.2f,1.2f);
    glProgramUniform2f(spectrum_shader,12,1.2f,1.2f);
    glProgramUniform2f(pat_shader,12,1.2f,1.2f);
    // Stop text input
    SDL_StopTextInput();

    gl_font = embedded_renderer(config.ui.font_atlas_width,config.ui.font_atlas_height,config.ui.fontsize,config.ui.font);
//    gl_font.open_font(config.ui.alt_font);
    gl_font.set_color(1.,1.,1.,1.);
    textbox_font = embedded_renderer(config.ui.font_atlas_width,config.ui.font_atlas_height,config.ui.fontsize,config.ui.font);
//    gl_font.open_font(config.ui.alt_font);
    textbox_font.set_color(1.,1.,1.,1.);
    // Open the font
    // Init statics
    pat_entry = false;
    CHECK_GL();
}
void ui_make_context_current(SDL_GLContext ctx)
{
    SDL_GL_MakeCurrent(window,ctx);
}
SDL_GLContext ui_make_secondary_context()
{
    SDL_GL_MakeCurrent(window,context);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_PROFILE_MASK
      , SDL_GL_CONTEXT_PROFILE_CORE
        );
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_RELEASE_BEHAVIOR
      , SDL_GL_CONTEXT_RELEASE_BEHAVIOR_FLUSH
        );
    SDL_GL_SetAttribute(
        SDL_GL_SHARE_WITH_CURRENT_CONTEXT
      , GL_FALSE
        );
    SDL_GL_SetAttribute(
        SDL_GL_CONTEXT_FLAGS
      ,(int(get_loglevel()) <= int(LOGLEVEL_DEBUG)
            ? SDL_GL_CONTEXT_DEBUG_FLAG
            : 0)
       |SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG
        );
    auto extra_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window,context);
    return extra_context;

}
void ui_term() {
        // TODO glDeleteTextures...
    glDeleteProgram(blit_shader);
    glDeleteProgram(main_shader);
    glDeleteProgram(pat_shader);
    glDeleteProgram(crossfader_shader);
    glDeleteProgram(spectrum_shader);
    glDeleteProgram(waveform_shader);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    window = NULL;
    SDL_Quit();
}

static struct pattern * selected_pattern(int s) {
    for(int i=0; i<config.ui.n_patterns; i++) {
        if(map_selection[i] == s)
            return deck[map_deck[i]].patterns[map_pattern[i]].get();
    }
    return NULL;
}

static void set_slider_to(int s, float v) {
    if(s == crossfader_selection_top || s == crossfader_selection_bot) {
        crossfader.position = v;
    } else {
        if(auto  &&p = selected_pattern(s)) {
            p->intensity = v;
        }
    }
}

static void increment_slider(int s, float v) {
    if(s == crossfader_selection_top || s == crossfader_selection_bot) {
        crossfader.position = CLAMP(crossfader.position + v, 0., 1.);
    } else {
        if(auto && p = selected_pattern(s))
            p->intensity = CLAMP(p->intensity + v, 0., 1.);
    }
}

static void handle_key(SDL_KeyboardEvent * e) {
    bool shift = e->keysym.mod & KMOD_SHIFT;
    bool ctrl = e->keysym.mod & KMOD_CTRL;
    bool alt = e->keysym.mod & KMOD_ALT;
    (void) (shift & ctrl & alt);

    if(pat_entry) {
        switch(e->keysym.sym) {
            case SDLK_RETURN:
                for(int i=0; i<config.ui.n_patterns; i++) {
                    if(map_selection[i] == selected) {
                        gl_font.clear();
                        gl_font.set_dirty();
                        if(pat_entry_text[0] == ':') {
                            if (deck[map_deck[i]].load_set(pat_entry_text+1) == 0) {
                                // TODO: Load in the correct pattern names
                            }
                        } else if(deck[map_deck[i]].load_pattern( map_pattern[i], pat_entry_text) == 0) {
                            if(pat_entry_text[0] != '\0') {
                            }
                        }
                        break;
                    }
                }
                pat_entry = false;
                SDL_StopTextInput();
                break;
            case SDLK_ESCAPE:
                pat_entry = false;
                SDL_StopTextInput();
                break;
            case SDLK_BACKSPACE:
                if (pat_entry_text[0] != '\0') {
                    pat_entry_text[strlen(pat_entry_text)-1] = '\0';
                    handle_text("\0");
                }
                break;
            default:
                break;
        }
    } else {
        DEBUG("Keysym: %u '%c'", e->keysym.sym, e->keysym.sym);
        switch(e->keysym.sym) {
            case SDLK_h:
            case SDLK_LEFT:
                selected = map_left[selected];
                break;
            case SDLK_l:
            case SDLK_RIGHT:
                selected = map_right[selected];
                break;
            case SDLK_UP:
            case SDLK_k:
                if (shift) increment_slider(selected, +0.1);
                else selected = map_up[selected];
                break;
            case SDLK_DOWN:
            case SDLK_j:
                if (shift) increment_slider(selected, -0.1);
                else selected = map_down[selected];
                break;
            case SDLK_ESCAPE:
                selected = 0;
                break;
            case SDLK_DELETE:
            case SDLK_d:
                for(int i=0; i<config.ui.n_patterns; i++) {
                    if(map_selection[i] == selected) {
                        deck[map_deck[i]].unload_pattern( map_pattern[i]);
                        break;
                    }
                }
                break;
            case SDLK_BACKQUOTE:
                set_slider_to(selected, 0);
                break;
            case SDLK_1:
                set_slider_to(selected, 0.1);
                break;
            case SDLK_2:
                set_slider_to(selected, 0.2);
                break;
            case SDLK_3:
                set_slider_to(selected, 0.3);
                break;
            case SDLK_4:
                if(shift) {
                    selected = map_end[selected];
                } else {
                    set_slider_to(selected, 0.4);
                }
                break;
            case SDLK_5:
                set_slider_to(selected, 0.5);
                break;
            case SDLK_6:
                if(shift) {
                    selected = map_home[selected];
                } else {
                    set_slider_to(selected, 0.6);
                }
                break;
            case SDLK_7:
                set_slider_to(selected, 0.7);
                break;
            case SDLK_8:
                set_slider_to(selected, 0.8);
                break;
            case SDLK_9:
                set_slider_to(selected, 0.9);
                break;
            case SDLK_0:
                set_slider_to(selected, 1);
                break;
            case SDLK_SEMICOLON: if(!shift) break;
                for(int i=0; i<config.ui.n_patterns; i++) {
                    if(map_selection[i] == selected) {
                        pat_entry = true;
                        pat_entry_text[0] = '\0';
                        SDL_StartTextInput();
                    }
                }
                break;
            case SDLK_RETURN:
                for(int i=0; i<config.ui.n_patterns; i++) {
                    if(map_selection[i] == selected) {
                        if(i < 4) {
                            left_deck_selector = 0;
                        } else if(i < 8) {
                            right_deck_selector = 1;
                        } else if(i < 12) {
                            left_deck_selector = 2;
                        } else if(i < 16) {
                            right_deck_selector = 3;
                        }
                    }
                }
                break;
            case SDLK_LEFTBRACKET:
                if(left_deck_selector == 0) {
                    left_deck_selector = 2;
                } else {
                    left_deck_selector = 0;
                }
                break;
            case SDLK_RIGHTBRACKET:
                if(right_deck_selector == 1) {
                    right_deck_selector = 3;
                } else {
                    right_deck_selector = 1;
                }
                break;
            case SDLK_SPACE:
                selected = map_space[selected];
                break;
            case SDLK_TAB:
                if(shift) {
                    selected = map_stab[selected];
                } else {
                    selected = map_tab[selected];
                }
                break;
            case SDLK_HOME:
                selected = map_home[selected];
                break;
            case SDLK_END:
                selected = map_end[selected];
                break;
            case SDLK_r:
                if (shift) {
                    midi_refresh();
                    output_refresh();
                }
                break;
            case SDLK_q:
                switch(strip_indicator) {
                    case STRIPS_NONE:
                        strip_indicator = STRIPS_SOLID;
                        break;
                    case STRIPS_SOLID:
                        strip_indicator = STRIPS_COLORED;
                        break;
                    case STRIPS_COLORED:
                    default:
                        strip_indicator = STRIPS_NONE;
                        break;
                }
                break;
            default:
                break;
        }
    }
}

static void blit(float x, float y, float w, float h) {
    bind_vao_fill_vbo(x, y, w, h);
    glDrawArrays(GL_POINTS, 0, 1);
}

static void ui_render(bool select) {
    GLint location;
    // Render strip indicators
    if(!select) {
    glDisable(GL_BLEND);
    switch(strip_indicator) {
        case STRIPS_SOLID:
        case STRIPS_COLORED:
            glBindFramebuffer(GL_FRAMEBUFFER, strip_fb);
            glUseProgram(strip_shader);
            glProgramUniform2f(strip_shader, 0, 1., 1.);//config.pattern.master_width, config.pattern.master_height);

            location = glGetUniformLocation(strip_shader, "iTexture");
            glProgramUniform1i(strip_shader, location, 0);
            location = glGetUniformLocation(strip_shader, "iIndicator");
            glProgramUniform1i(strip_shader,location, strip_indicator);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, crossfader.tex_output);

            glClear(GL_COLOR_BUFFER_BIT);
            if(!strip_vbo || !strip_vbo) {
                glGenBuffers(1,&strip_vbo);
                glBindBuffer(GL_ARRAY_BUFFER, strip_vbo);
//                glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4, NULL, GL_STATIC_DRAW);
                auto vvec = std::vector<GLfloat>{};
                auto vertex2d = [&vvec](auto x, auto y) { vvec.push_back((x + 1)/2); vvec.push_back((y+1)/2);};
                for(auto d = output_device_head; d ; d = d->next) {
                    bool first = true;
                    double x;
                    double y;
                    for(auto v = d->vertex_head; v ; v = v->next) {
                        if(!first) {
                            auto dx = v->x - x;
                            auto dy = v->y - y;
                            auto dl = hypot(dx, dy);
                            dx = config.ui.strip_thickness * dx / dl;
                            dy = config.ui.strip_thickness * dy / dl;
                            vertex2d(x + dy, y - dx);
                            vertex2d(v->x + dy, v->y - dx);
                            vertex2d(x - dy, y + dx);
                            vertex2d(v->x + dy, v->y - dx);
                            vertex2d(v->x - dy, v->y + dx);
                            vertex2d(x - dy, y + dx);
                        } else {
                            first = false;
                        }
                        x = v->x;
                        y = v->y;
                    }
                }
                glNamedBufferData(strip_vbo, vvec.size() * sizeof(vvec[0]), &vvec[0], GL_STATIC_DRAW);
                strip_vbo_size = vvec.size();
                glGenVertexArrays(1,&strip_vao);
                glBindVertexArray(strip_vao);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0);
                glEnableVertexAttribArray(0);
            }

            glBindBuffer(GL_ARRAY_BUFFER, strip_vbo);
            glBindVertexArray(strip_vao);
            glDrawArrays(GL_TRIANGLES, 0, strip_vbo_size / 2);
            break;
        default:
        case STRIPS_NONE:
            break;
    }
    }

//    // Render the patterns

    // Render the crossfader


    auto sw = 0;
    auto sh = 0;
    auto vw = 0;
    auto vh = 0;

    glEnable(GL_BLEND);

    // Render to screen (or select fb)
    if(select) {
        glBindFramebuffer(GL_FRAMEBUFFER, select_fb);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    glViewport(0, 0, ww, wh);

    CHECK_GL();
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(main_shader);
    glProgramUniform2f(main_shader, 0, ww, wh);

    glUniform2f(3, ww, wh);
    location = glGetUniformLocation(main_shader, "iSelection");
    glUniform1i(location, select);
    location = glGetUniformLocation(main_shader, "iSelector");
    glUniform3i(location, left_deck_selector,right_deck_selector,selected);
    CHECK_GL();

    fill(ww, wh);
    CHECK_GL();
    if(!select) {
        analyze_render(buf_spectrum_data, buf_waveform_data, buf_waveform_beats_data);
        // Render the spectrum
        sw = config.ui.spectrum_width;
        sh = config.ui.spectrum_height;
        glUseProgram(spectrum_shader);
        location = glGetUniformLocation(spectrum_shader, "iBins");
       glUniform1i(location, config.audio.spectrum_bins);
//        location = glGetUniformLocation(spectrum_shader, "iSpectrum");
//        glUniform1i(location, 0);
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_1D, tex_spectrum_data);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, buf_spectrum_data);
        glProgramUniform2f(spectrum_shader, 0, ww, wh);
        blit(config.ui.spectrum_x, config.ui.spectrum_y, sw, sh);
        // Render the waveform

        vw = config.ui.waveform_width;
        vh = config.ui.waveform_height;
        glUseProgram(waveform_shader);
        glProgramUniform2f(waveform_shader, 0, ww, wh);
        location = glGetUniformLocation(waveform_shader, "iLength");
        glUniform1i(location, config.audio.waveform_length);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, buf_waveform_data);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, buf_waveform_beats_data);
//        location = glGetUniformLocation(waveform_shader, "iWaveform");
///        glUniform1i(location, 0);
//        location = glGetUniformLocation(waveform_shader, "iBeats");
//        glUniform1i(location, 1);
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_1D, tex_waveform_data);
//        glActiveTexture(GL_TEXTURE1);
//        glBindTexture(GL_TEXTURE_1D, tex_waveform_beats_data);

        blit(config.ui.waveform_x, config.ui.waveform_y, vw, vh);
    }
    auto cw = config.ui.crossfader_width;
    auto ch = config.ui.crossfader_height;
    glUseProgram(crossfader_shader);
    location = glGetUniformLocation(crossfader_shader, "iSelection");
    glUniform1i(location, select);
    location = glGetUniformLocation(main_shader, "iSelector");
    glUniform3i(location, left_deck_selector,right_deck_selector,selected);
    location = glGetUniformLocation(crossfader_shader, "iTexture");
    glUniform1i(location, 0);
    location = glGetUniformLocation(crossfader_shader, "iStrips");
    glUniform1i(location, 1);
    location = glGetUniformLocation(crossfader_shader, "iIntensity");
    glUniform1f(location, crossfader.position);
    location = glGetUniformLocation(crossfader_shader, "iIndicator");
    glUniform1i(location, strip_indicator);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, crossfader.tex_output);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, strip_texture);

    glProgramUniform2f(crossfader_shader, 0, ww, wh);
    glProgramUniform2f(crossfader_shader, 12, 1.2,1.2);
    blit(config.ui.crossfader_x, config.ui.crossfader_y, cw, ch);

    int pw = config.ui.pattern_width;
    int ph = config.ui.pattern_height;
    glUseProgram(pat_shader);
    location = glGetUniformLocation(main_shader, "iSelector");
    glUniform3i(location, left_deck_selector,right_deck_selector,selected);
    location = glGetUniformLocation(pat_shader, "iSelection");
    glUniform1i(location, select);

    location = glGetUniformLocation(pat_shader, "iTexture");
    glUniform1i(location, 0);
    location = glGetUniformLocation(pat_shader, "iName");
    glUniform1i(location, 1);
    GLint pattern_intensity = glGetUniformLocation(pat_shader, "iIntensity");
    glProgramUniform2f(pat_shader, 0, ww, wh);
    {
        for(int i = 0; i < config.ui.n_patterns; i++) {
            auto &p = deck[map_deck[i]].patterns[map_pattern[i]];
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, p ? p->tex_output : 0);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);
            glVertexAttribI1i(2, i);
            glUniform1f(pattern_intensity, p ? p->intensity : 0);
            glUniform1i(location, select);

            blit(map_x[i],map_y[i],pw, ph);
        }
    }

    for(int i = 0; i < config.ui.n_patterns; i++) {
        if(auto &pat = deck[map_deck[i]].patterns[map_pattern[i]]) {
            if(pat->name.size() && gl_font.get_dirty()) {
                gl_font.print(map_x[i] + config.ui.pattern_name_x, map_y[i] + config.ui.pattern_height -config.ui.pattern_name_y, pat->name);
            }
        }
    }
    if(!select) {
        if(pat_entry) {
            for(int i = 0; i < config.ui.n_patterns; i++) {
                if(map_selection[i] == selected) {
                    if(gl_font.get_dirty()) {
                        gl_font.m_scale *= 2;
                        gl_font.active_font(config.ui.alt_font);
                        gl_font.print(map_x[i], map_y[i] + config.ui.pattern_height - gl_font.height(), pat_entry_text);
                        gl_font.active_font(config.ui.font);
                        gl_font.m_scale /= 2;
                    }
                    CHECK_GL();
                    break;
                }
            }
        }
        gl_font.render(ww, wh);
    }
    CHECK_GL();
}

struct rgba {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

static struct rgba test_hit(int x, int y) {
    struct rgba data;

    glBindFramebuffer(GL_FRAMEBUFFER, select_fb);
    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &data);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return data;
}

static void handle_mouse_move() {
    switch(ma) {
        case MOUSE_NONE:
            break;
        case MOUSE_DRAG_INTENSITY: {
            if(auto &p = deck[map_deck[mp]].patterns[map_pattern[mp]]) {
                p->intensity = mci + (mx - mcx) * config.ui.intensity_gain_x + (my - mcy) * config.ui.intensity_gain_y;
                if(p->intensity > 1) p->intensity = 1;
                if(p->intensity < 0) p->intensity = 0;
            }
            break;
        }
        case MOUSE_DRAG_CROSSFADER:
            crossfader.position = mci + (mx - mcx) * config.ui.crossfader_gain_x + (my - mcy) * config.ui.crossfader_gain_y;
            if(crossfader.position > 1) crossfader.position = 1;
            if(crossfader.position < 0) crossfader.position = 0;
            break;
    }
}

auto mouse_down = false;
static void handle_mouse_up() {
    ma = MOUSE_NONE;
    mouse_down = false;
}

static void handle_text(const char * text) {
    if(pat_entry) {
        if(strlen(pat_entry_text) + strlen(text) < sizeof(pat_entry_text)) {
            strcat(pat_entry_text, text);
        }
        gl_font.clear();
        gl_font.set_dirty();
    }
}
static void handle_mouse_down() {
    struct rgba hit;
    hit = test_hit(mx, wh - my);
    switch(hit.r) {
        case HIT_NOTHING:
            selected = 0;
            break;
        case HIT_PATTERN:
            if(hit.g < config.ui.n_patterns) selected = map_selection[hit.g];
            break;
        case HIT_INTENSITY:
            if(hit.g < config.ui.n_patterns) {
                if(auto &p = deck[map_deck[hit.g]].patterns[map_pattern[hit.g]]) {
                    ma = MOUSE_DRAG_INTENSITY;
                    mp = hit.g;
                    mcx = mx;
                    mcy = my;
                    mci = p->intensity;
                }
            }
            break;
        case HIT_CROSSFADER:
            selected = crossfader_selection_top;
            break;
        case HIT_CROSSFADER_POSITION:
            ma = MOUSE_DRAG_CROSSFADER;
            mcx = mx;
            mcy = my;
            mci = crossfader.position;
            break;
    }
}
static double seconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_nsec * 1e-9 + ts.tv_sec;
}
void ui_run() {
        SDL_Event e;

        quit = false;
        double fps_sample = seconds();
        int    fps_frames = 0;

        while(!quit) {

            while(SDL_PollEvent(&e) != 0) {
                if (midi_command_event != (Uint32) -1 &&
                    e.type == midi_command_event) {
                    midi_event * me = static_cast<midi_event*>(e.user.data1);
                    switch (me->type) {
                    case MIDI_EVENT_SLIDER: {
                        set_slider_to(me->slider.index, me->slider.value);
                        break;
                    }
                    case MIDI_EVENT_KEY: {
                        SDL_KeyboardEvent fakekeyev;
                        memset(&fakekeyev, 0, sizeof fakekeyev);
                        fakekeyev.type = SDL_KEYDOWN;
                        fakekeyev.state = SDL_PRESSED;
                        fakekeyev.keysym.sym = me->key.keycode[0];
                        handle_key(&fakekeyev);
                        break;
                        }
                    }
                    free(e.user.data1);
                    free(e.user.data2);
                    continue;
                }
                switch(e.type) {
                    case SDL_QUIT:
                        quit = true;
                        break;
                    case SDL_KEYDOWN:
                        handle_key(&e.key);
                        break;
                    case SDL_MOUSEMOTION:
                        mx = e.motion.x;
                        my = e.motion.y;
                        handle_mouse_move();
                        break;
                    case SDL_MOUSEBUTTONDOWN:
                        mx = e.button.x;
                        my = e.button.y;
                        switch(e.button.button) {
                            case SDL_BUTTON_LEFT:
                                if(!mouse_down) {
                                    ui_render(true);
                                    handle_mouse_down();
                                    mouse_down = true;
                                }
                                break;
                        }
                        break;
                    case SDL_MOUSEBUTTONUP:
                        mx = e.button.x;
                        my = e.button.y;
                        switch(e.button.button) {
                            case SDL_BUTTON_LEFT:
                                handle_mouse_up();
                                break;
                        }
                        break;
                    case SDL_TEXTINPUT:
                        handle_text(e.text.text);
                        break;
                }
            }
            if(mouse_down) {
                handle_mouse_down();
                mouse_down = false;
            }
            for(auto & d : deck)
                d.render();
            crossfader_render(&crossfader, deck[left_deck_selector].tex_output, deck[right_deck_selector].tex_output);
            render_readback(&render);
            ui_render(false);

            SDL_GL_SwapWindow(window);
            fps_frames++;
            double new_sample = seconds();
            if(new_sample - fps_sample > 1.0) {
                double fps = fps_frames / (new_sample - fps_sample);
                fps_sample = new_sample;
                fps_frames = 0;
                INFO("Display frame rate is %F.2 FPS\n",fps);
            }

            double cur_t = SDL_GetTicks();
            double dt = cur_t - l_t;
            if(dt > 0)
                current_time += dt / 1000;
            l_t = cur_t;
        }
}

