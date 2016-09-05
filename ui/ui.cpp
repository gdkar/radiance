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

GLuint tex_array;
int    tex_array_layers;
static GLuint select_fb;
static GLuint strip_fb;
static GLuint select_tex;
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
static GLuint ssbo_layers;
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

struct render_class {
    struct reservation {
        reservation() = default;
        reservation(render_class *cls)
        : clsp(cls)
        {}
        void update(float x, float y, float w, float h, int pid = 0)
        {
            if(data.size() >= 5 ) {
                if(x!=data[0].f || y != data[1].f || w != data[2].f || h != data[3].f || data[4].i != pid) {
                    off = -1;
                    data=std::vector<generic>{x,y,w,h,pid,0,0,0};
                    dirty = true;
                    clsp->vbo_dirty = true;
                }
            }else{
                    off = -1;
                    data=std::vector<generic>{x,y,w,h,pid,0,0,0};
                    dirty = true;
                    clsp->vbo_dirty = true;
            }
        }
        void draw()
        {
            if(off < 0)
                return;
            clsp->draw(off);
        }
        render_class *clsp;
        off_t off{};
        std::vector<generic> data{};
        bool dirty{false};
    };
    std::list<reservation> reservations{};
    render_class()
    {}
    bool create()
    {
        CHECK_GL();
        if(!vao){
            glGenVertexArrays(1,&vao);
            CHECK_GL();
        }
        if(!vbo) {
            glGenBuffers(1,&vbo);
            CHECK_GL();
        }
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindVertexArray(vao);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(generic), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(generic), (void*)(2*sizeof(generic)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(generic), (void*)(4*sizeof(generic)));
        glEnableVertexAttribArray(2);

        CHECK_GL();
        return true;
    }
    reservation *reserve(float x, float y, float w, float h, int pid = 0)
    {
        reservations.emplace_back(this);
        auto &res = reservations.back();
        res.update(x,y,w,h,pid);
        return &res;
    }
    void prepare()
    {
        if(vbo_dirty) {
            data.clear();
            vbo_dirty = false;
            for(auto & res : reservations) {
//                if(res.dirty){
                    res.off = data.size() / 8;
                    std::copy(res.data.begin(),res.data.end(),std::back_inserter(data));
                    res.dirty = false;
                    vbo_dirty = true;
//                }
            }
            if(vbo_dirty){
                glBindBuffer(GL_ARRAY_BUFFER,vbo);
                glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * data.size(), data.data(), GL_DYNAMIC_DRAW);
                vbo_dirty = false;
                vbo_size = sizeof(GLfloat)*data.size();
            }
        }
    }
    void bind()
    {
        glBindVertexArray(vao);
    }
    void draw(off_t off)
    {
        prepare();
        glDrawArrays(GL_POINTS, off, 1);
    }
   ~render_class()
    {
        glDeleteVertexArrays(1,&vao);
        glDeleteBuffers(1,&vbo);
    }
    GLuint vao{};
    GLuint vbo{};
    std::vector<GLfloat> data{};
    size_t vbo_size{};
    bool   vbo_dirty{true};
};

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
static int snap_states[19] = {0};
render_class rclass{};
std::vector<render_class::reservation*> pattern_res{};
render_class::reservation* crossfader_res{};
render_class::reservation* spectrum_res{};
render_class::reservation* waveform_res{};
render_class::reservation* main_res{};
render_class::reservation* strips_res{};
// Font
embedded_renderer gl_font{};
embedded_renderer textbox_font{};
static const SDL_Color font_color = {255, 255, 255, 255};

// Pat entry
float       pat_entry_x{};
float       pat_entry_y{};
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
/*static void bind_vao_fill_vbo(float x, float y, float w, float h)
{
    GLfloat vertices[] = { x, y, w, h };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    void *_vbo = glMapBufferRange(GL_ARRAY_BUFFER, 0, sizeof(vertices), GL_MAP_INVALIDATE_BUFFER_BIT|GL_MAP_WRITE_BIT);
    memcpy(_vbo,vertices,sizeof(vertices));
    glUnmapBuffer(GL_ARRAY_BUFFER);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glBindVertexArray(vao);
}*/
/*static void fill(float w, float h) {
    bind_vao_fill_vbo(0., 0., w, h);
    glDrawArrays(GL_POINTS, 0, 1);
}*/

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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
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
    if(int(get_loglevel()) <= int(LOGLEVEL_DEBUG)) {
        glDebugMessageCallback(debug_callback,nullptr);
        glEnable(GL_DEBUG_OUTPUT);
        CHECK_GL();
    }
    rclass.create();
    for(auto i = 0; i < 16; ++i) {
        pattern_res.push_back(rclass.reserve(float(map_x[i]),float(map_y[i]),config.ui.pattern_width,config.ui.pattern_height,i));
    }
    crossfader_res = rclass.reserve(config.ui.crossfader_x,config.ui.crossfader_y,config.ui.crossfader_width,config.ui.crossfader_height);
    spectrum_res = rclass.reserve(config.ui.spectrum_x,config.ui.spectrum_y,config.ui.spectrum_width,config.ui.spectrum_height);
    waveform_res = rclass.reserve(config.ui.waveform_x,config.ui.waveform_y,config.ui.waveform_width,config.ui.waveform_height);
    strips_res = rclass.reserve(0,0,config.pattern.master_width,config.pattern.master_height);
    main_res = rclass.reserve(0,0,config.ui.window_width,config.ui.window_height);

    rclass.prepare();
    glGenBuffers(1,&vbo);
    glGenVertexArrays(1,&vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glNamedBufferData(vbo, sizeof(GLfloat) * 4, NULL, GL_DYNAMIC_DRAW);
    glBindVertexArray(vao);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

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

    tex_array = make_texture(GL_RGBA32F, config.pattern.master_width, config.pattern.master_height, 65);

    glBindFramebuffer(GL_FRAMEBUFFER, select_fb);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, select_tex, 0);

    // Init pattern textures

    glGenBuffers(1,&buf_spectrum_data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,buf_spectrum_data);
    glGenBuffers(1,&ssbo_layers);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,ssbo_layers);
    glBufferData(GL_SHADER_STORAGE_BUFFER,64*sizeof(int), NULL, GL_DYNAMIC_DRAW);
    glGenBuffers(1,&buf_waveform_data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,buf_waveform_data);
    glGenBuffers(1,&buf_waveform_beats_data);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,buf_waveform_beats_data);

    // Strip indicators
    strip_texture = make_texture ( config.pattern.master_width, config.pattern.master_height);
    glBindFramebuffer(GL_FRAMEBUFFER, strip_fb);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, strip_texture, 0);

    // Done allocating textures & FBOs, unbind and check for errors
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if((main_shader = load_program(
        {"#vertex_framed.glsl"},{"#geometry_flat.glsl"},
        {"#lib.glsl","#lib_ui.glsl"}, "#header_ui.glsl",{"#ui_main.glsl"})) == 0) FAIL("Could not load UI main shader!\n%s", get_load_program_error().c_str());
    glProgramUniform2f(main_shader, 0, config.ui.window_width, config.ui.window_height);
    if((pat_shader = load_program(
        {"#vertex_framed.glsl"},{"#geometry_flat.glsl"},
        {"#lib.glsl","#lib_ui.glsl"},"#header_ui.glsl",{"#ui_pat.glsl"})) == 0) FAIL("Could not load UI pattern shader!\n%s", get_load_program_error().c_str());
    if((crossfader_shader = load_program(
        {"#vertex_framed.glsl"},{"#geometry_flat.glsl"},
        {"#lib.glsl","#lib_ui.glsl"},"#header_ui.glsl",{"#ui_crossfader.glsl"})) == 0) FAIL("Could not load UI crossfader shader!\n%s", get_load_program_error().c_str());
    if((waveform_shader = load_program(
        {"#vertex_framed.glsl"},{"#geometry_flat.glsl"},
        {"#lib.glsl","#lib_ui.glsl"},"#header_ui.glsl",{"#ui_waveform.glsl"})) == 0) FAIL("Could not load UI waveform shader!\n%s", get_load_program_error().c_str());
    if((spectrum_shader = load_program(
        {"#vertex_framed.glsl"},{"#geometry_flat.glsl"},{"#lib.glsl","#lib_ui.glsl"},"#header_ui.glsl",{"#ui_spectrum.glsl"})) == 0) FAIL("Could not load UI spectrum shader!\n%s", get_load_program_error().c_str());
    if((strip_shader = load_program({"#strip.v.glsl"},{},{"#lib.glsl","#lib_ui.glsl"},"#header_ui.glsl",{"#strip.f.glsl"})) == 0) FAIL("Could not load strip indicator shader!\n%s", get_load_program_error().c_str());
    glProgramUniform2f(strip_shader, 0, 1., 1.);//config.pattern.master_width, config.pattern.master_height);
    glProgramUniform2f(spectrum_shader, 0, ww, wh);
    glProgramUniform2f(waveform_shader, 0, ww, wh);
    glProgramUniform2f(spectrum_shader,12,1.2f,1.2f);
    glProgramUniform2f(waveform_shader,12,1.2f,1.2f);
    glProgramUniform2f(crossfader_shader, 0, ww, wh);
    glProgramUniform2f(pat_shader, 0, ww, wh);


    auto location = glGetUniformLocation(strip_shader, "iTexture");
    glProgramUniform1i(strip_shader, location, 0);
    location = glGetUniformLocation(pat_shader, "iTexture");
    glProgramUniform1i(pat_shader,location, 0);
    location = glGetUniformLocation(crossfader_shader, "iTexture");
    glProgramUniform1i(crossfader_shader,location, 0);
    location = glGetUniformLocation(crossfader_shader, "iStrips");
    glProgramUniform1i(crossfader_shader,location, 1);
    location = glGetUniformLocation(waveform_shader, "iLength");
    glProgramUniform1i(waveform_shader,location, config.audio.waveform_length);

    if((blit_shader = load_program({"#lib.glsl","#lib_ui.glsl"},"#header_ui.glsl",{"#blit.glsl"})) == 0) FAIL("Could not load blit shader!\n%s", get_load_program_error().c_str());
    glProgramUniform2f(blit_shader,0,ww,wh);
    glProgramUniform2f(pat_shader,12,1.2f,1.2f);
    glProgramUniform2f(crossfader_shader,12,1.2f,1.2f);
    // Stop text input
    SDL_StopTextInput();

    gl_font = embedded_renderer(config.ui.font_atlas_width,config.ui.font_atlas_height,config.ui.fontsize,config.ui.font);
    gl_font.set_color(1.,1.,1.,1.);
    textbox_font = embedded_renderer(config.ui.font_atlas_width,config.ui.font_atlas_height,config.ui.fontsize,config.ui.font);
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
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
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

static float get_slider(int s) {
    if(s == crossfader_selection_top || s == crossfader_selection_bot)
        return crossfader.position;
    if(auto p = selected_pattern(s))
        return p->intensity;
    return 0;
}

static void set_slider_to(int s, float v, int snap = 0) {
    if(snap && snap_states[s] != snap){
        if(std::abs(get_slider(s) - v) > params.ui.snap_threshold)
            return;
    }
    if(s == crossfader_selection_top || s == crossfader_selection_bot) {
        crossfader.position = v;
    } else {
        if(auto  &&p = selected_pattern(s)) {
            p->intensity = v;
        }
    }
    snap_states[s] = snap;
}

static void increment_slider(int s, float v) {
    set_slider_to(s, v + get_slider(s), 0);
}

static void handle_key(SDL_KeyboardEvent * e) {
    bool shift = e->keysym.mod & KMOD_SHIFT;
    bool ctrl = e->keysym.mod & KMOD_CTRL;
    bool alt = e->keysym.mod & KMOD_ALT;
    (void) (shift & ctrl & alt);

    if(pat_entry) {
        switch(e->keysym.sym) {
            case SDLK_RETURN:{
                auto entry = std::string{pat_entry_text};
                auto seppos = entry.find(' ');
                auto pref = entry.substr(0,seppos);
                auto suff = entry.substr(seppos + 1);
                if(pref == "w" || pref == "wa") {
                    for(int i=0; i<config.ui.n_patterns; i++) {
                        if(map_selection[i] == selected) {
                            deck[map_deck[i]].save(suff.c_str());
                            break;
                        }
                    }
                }else if(pref == "e" || pref == "edit" || pref == ":") {
                    for(int i=0; i<config.ui.n_patterns; i++) {
                        if(map_selection[i] == selected) {
                            gl_font.clear();
                            gl_font.set_dirty();
                            if (deck[map_deck[i]].load_set(suff.c_str()) == 0) {
                                for(int j = 0; j < config.ui.n_patterns; ++j) {
                                    if(map_deck[j] == map_deck[selected]) {
                                        snap_states[j] = 0.f;
                                    }
                                }
                                // TODO: Load in the correct pattern names
                            } else if(deck[map_deck[i]].load_pattern( map_pattern[i], suff.c_str(),-1) == 0) {
                                if(pat_entry_text[0] != '\0') {
                                }
                                snap_states[selected] = 0;
                            }
                            break;
                        }
                    }
                }
                pat_entry = false;
                SDL_StopTextInput();
                break;
            }
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
                if(selected >= 1 && selected < 17) {
                    deck[map_deck[selected-1]].unload_pattern( map_pattern[selected-1]);
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
            case SDLK_SEMICOLON: if(!shift) break; {
                if(selected >= 1 && selected < 17){
                    pat_entry = true;
                    pat_entry_x = map_x[selected-1];
                    pat_entry_y = map_y[selected-1];
                    pat_entry_text[0] = '\0';
                    textbox_font.clear();
                    textbox_font.set_dirty();
                    SDL_StartTextInput();
                }
            }
            case SDLK_RETURN:{
                int i = selected-1;
                if(i < 4) {
                    left_deck_selector = 0;
                } else if(i < 8) {
                    right_deck_selector = 1;
                } else if(i < 12) {
                    left_deck_selector = 2;
                } else if(i < 16) {
                    right_deck_selector = 3;
                }
                break;
        }
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
                params_refresh();
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
static void ui_render(bool select) {
    // Render strip indicators
    if(!select) {
        switch(strip_indicator) {
            case STRIPS_SOLID:
            case STRIPS_COLORED: {
                glDisable(GL_BLEND);
                glBindFramebuffer(GL_FRAMEBUFFER, strip_fb);
                glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                glUseProgram(strip_shader);
                auto location = glGetUniformLocation(strip_shader, "iIndicator");
                glProgramUniform1i(strip_shader,location, strip_indicator);
                glBindTextures(0, 1, &crossfader.tex_output);

                if(!strip_vbo || !strip_vbo) {
                    glGenBuffers(1,&strip_vbo);
                    glBindBuffer(GL_ARRAY_BUFFER, strip_vbo);
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
            }
            default:
            case STRIPS_NONE:
                break;
        }
    }

    if(!select) {
        analyze_render(buf_spectrum_data, buf_waveform_data, buf_waveform_beats_data);
    }
    // Render to screen (or select fb)
    if(select) {
        glBindFramebuffer(GL_FRAMEBUFFER, select_fb);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    glViewport(0, 0, ww, wh);
    glUseProgram(main_shader);
    auto location = glGetUniformLocation(main_shader, "iSelection");
    glUniform1i(location, select);
    location = glGetUniformLocation(main_shader, "iSelector");
    glUniform3i(location, left_deck_selector,right_deck_selector,selected);
    rclass.prepare();
    rclass.bind();
    main_res->draw();
    glEnable(GL_BLEND);
    if(!select) {
          GLuint buffers[] = {  buf_waveform_data,buf_waveform_beats_data, buf_spectrum_data};
          glBindBuffersBase(GL_SHADER_STORAGE_BUFFER, 1, 3, buffers);

          glUseProgram(spectrum_shader);
          spectrum_res->draw();
          glUseProgram(waveform_shader);
          waveform_res->draw();
    }
    glUseProgram(crossfader_shader);
    location = glGetUniformLocation(crossfader_shader, "iSelection");
    glUniform1i(location, select);
    location = glGetUniformLocation(main_shader, "iSelector");
    glUniform3i(location, left_deck_selector,right_deck_selector,selected);
    location = glGetUniformLocation(crossfader_shader, "iIntensity");
    glUniform1f(location, crossfader.position);
    location = glGetUniformLocation(crossfader_shader, "iIndicator");
    glUniform1i(location, strip_indicator);
    {
        GLuint texs[] = { crossfader.tex_output,strip_texture};
        glBindTextures(0, 2, texs);
    }
    crossfader_res->draw();
    glUseProgram(pat_shader);
    location = glGetUniformLocation(pat_shader, "iSelector");
    glProgramUniform3i(pat_shader,location, left_deck_selector,right_deck_selector,selected);
    location = glGetUniformLocation(pat_shader, "iSelection");
    glProgramUniform1i(pat_shader,location, select);
    {
        GLuint texs[] = { tex_array };
        glBindTextures(0, 1, texs);
        struct item{int layer;float intensity;};
        std::vector<item> items(16);
        for(auto i = 0; i < config.ui.n_patterns;++i){
            auto &p = deck[map_deck[i]].patterns[map_pattern[i]];
            if(p) {
                items[i].layer = p->out_layer;
                items[i].intensity = p->intensity;
            }else{
                items[i].layer = 64;
            }
        }
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, ssbo_layers);
        glBufferData(GL_SHADER_STORAGE_BUFFER, items.size() * sizeof(items[0]), items.data(),GL_DYNAMIC_DRAW);
    }
    rclass.bind();
    glDrawArrays(GL_POINTS,0,16);
/*    {
        for(int i = 0; i < config.ui.n_patterns; i++) {
            pattern_res[i]->draw();
        }
    }*/
    if(!select) {
        if(gl_font.get_dirty()){
            for(int i = 0; i < config.ui.n_patterns; i++) {
                if(auto &pat = deck[map_deck[i]].patterns[map_pattern[i]]) {
                    if(pat->name.size()) {
                        gl_font.print(map_x[i] + config.ui.pattern_name_x, map_y[i] + config.ui.pattern_height -config.ui.pattern_name_y, pat->name);
                    }
                }
            }
        }
        gl_font.render(ww, wh);
        if(pat_entry){
            textbox_font.clear();
            textbox_font.print(config.ui.pat_entry_x + pat_entry_x,config.ui.pat_entry_y + pat_entry_y,std::string{config.ui.pat_entry_prompt} + pat_entry_text);
            textbox_font.render(ww,wh);
        }
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

    glBindFramebuffer(GL_READ_FRAMEBUFFER, select_fb);
    glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &data);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
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
        textbox_font.clear();
        textbox_font.set_dirty();
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
                        set_slider_to(me->slider.index, me->slider.value,me->snap);
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
            for(auto & d : deck) d.render();
            crossfader_render(&crossfader, deck[left_deck_selector].out_layer, deck[right_deck_selector].out_layer);
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

