#include "util/common.h"

#include "pattern/deck.h"
#include "util/glsl.h"
#include "util/err.h"
#include "util/config.h"
#include "util/string.h"
#include "util/ini.h"

deck::deck() = default;

void deck::init(GLuint _ta, int _l)
{
    tex_array = _ta;
    layer     = _l;

    patterns.resize(config.deck.n_patterns);

    tex_input = make_texture( config.pattern.master_width, config.pattern.master_height);
    glGenFramebuffers(1, &fb_input);
//    glNamedFramebufferTexture(fb_input, GL_COLOR_ATTACHMENT0, tex_input, 0);
    glClearTexImage(tex_input, 0, GL_RGBA,GL_FLOAT,nullptr);
    CHECK_GL();
}

void deck::term()
{
    if(tex_input)
        glDeleteTextures(1, &tex_input);
    if(fb_input)
        glDeleteFramebuffers(1, &fb_input);
    if(tex_output)
        glDeleteTextures(1, &tex_output);
    tex_input = 0;
    fb_input = 0;
    tex_output = 0;
    patterns.clear();
}
deck::~deck()
{
    term();
}
int deck::load_pattern(int slot, const char * prefix, float intensity)
{
    assert(slot >= 0 && slot < config.deck.n_patterns);
    if(intensity < 0) {
        if(patterns[slot])
            intensity = patterns[slot]->intensity;
        else
            intensity = 0;
    }
    if(prefix[0] == '\0') {
        if(patterns[slot]) {
            prefix = patterns[slot]->name.c_str();
        } else {
            return -1;
        }
    }
    try {
        auto p = std::make_unique<pattern>(prefix, tex_array, layer + (slot*4), slot + (layer/4));
        p->intensity = intensity;
        patterns[slot].swap(p);
    }catch(const std::exception &e) {
        ERROR("failed to load pattern: %s",e.what());
    }
    return 0;
}
void deck::unload_pattern(int slot)
{
    assert(slot >= 0 && slot < config.deck.n_patterns);
    patterns[slot].reset();
}

struct deck_ini_data {
    struct deck * deck;
    const char * name;
    bool found;
};

static int deck_ini_handler(void * user, const char * section, const char * name, const char * value)
{
    struct deck_ini_data * data = static_cast<deck_ini_data*>(user);

    INFO("%s %s %s", section, name, value);
    if (data->found) return 1;
    if (strcmp(section, "decks") ) return 1;
    if (strcmp(name, data->name) ) return 1;
    for(auto i = 0; i < config.deck.n_patterns;++i){
        data->deck->unload_pattern(i);
    }
    data->found = true;
    auto val = std::string{value};
    auto slot = 0;
    while (slot < config.deck.n_patterns) {
        if(val.empty())
            break;
        auto seppos = val.find(' ');
        if(seppos == 0)
            break;
        auto entry = val.substr(0,seppos);
        val = val.substr(seppos+1);
        if(entry[0] == '_' && entry.size() == 1){
            slot++;
            continue;
        }
        seppos = entry.find(':');
        auto name = entry.substr(0,seppos);
        auto intensity = 0.f;
        std::istringstream{entry.substr(seppos+1)} >> intensity;
        if(data->deck->load_pattern(slot++, name.c_str(),intensity) < 0)
            return 0;
    }
    return 1;
}
int deck::load_set(const char * name)
{
    deck_ini_data data = {
        .deck = this, .name = name, .found = false
    };
    auto rc = ini_parse(params.paths.decks_config, deck_ini_handler, &data);
    if(rc)
        return rc;
    if (!data.found) {
        ERROR("No deck set named '%s'", name);
        return -1;
    }
    return 0;
}
int deck::save(const char *name)
{
    std::ofstream ofs(params.paths.decks_config,std::ios_base::app);
    ofs << name << "=";
    for(auto i = 0; i < config.deck.n_patterns;++i){
        if(patterns[i])
            ofs << " " << patterns[i]->name << ":" << patterns[i]->intensity;
        else
            ofs << " _";
    }
    ofs << std::endl;
    return 0;
}
void deck::render() {
    tex_output = tex_input;
    out_layer  = 64;//in_layer;
//    glClearTexSubImage(tex_array, 0, 0, 0, out_layer, config.pattern.master_width,config.pattern.master_height,1,GL_RGBA,GL_FLOAT,nullptr);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb_input);
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_array,0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex_array);

    glDisable(GL_BLEND);
    glViewport(0,0,config.pattern.master_width,config.pattern.master_height);
    for(auto &pat : patterns) {
        if(pat) {
            pat->render(tex_output, out_layer);
            out_layer = pat->out_layer;
        }
    }
    next_slot = 0;
}
void deck::render_one() {
    if(!next_slot) {
        tex_output = tex_input;
        out_layer  = 64;//in_layer;
        glClearTexSubImage(tex_array, 0, 0, 0, out_layer, config.pattern.master_width,config.pattern.master_height,1,GL_RGBA,GL_FLOAT,nullptr);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb_input);
        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex_array,0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, tex_array);
        glDisable(GL_BLEND);
        glViewport(0,0,config.pattern.master_width,config.pattern.master_height);
    }
    if(auto &pat = patterns[next_slot]) {
        pat->render(tex_output,out_layer);
        out_layer = pat->out_layer;
    }
    next_slot = (next_slot+1) % 4;
}
