#include "util/common.h"
#include "ui/ui.h"
#include "pattern/pattern.h"
#include "time/timebase.h"
#include "util/glsl.h"
#include "util/string.h"
#include "sys/stat.h"
#include "util/err.h"
#include "util/config.h"
#include "main.h"
#include <stdexcept>
#include <exception>
#include <system_error>


static GLuint vao = 0;
pattern::pattern(const std::string&prefix, GLuint tex_array, int tex_layer)
: tex_array(tex_array)
, tex_layer(tex_layer)
{
    bool new_buffers = false;
    if(!vao) {
        glGenVertexArrays(1,&vao);
        new_buffers = true;
    }
        float w = config.pattern.master_width,h =  config.pattern.master_height;
        float x = 0.f, y = 0.f;

    if(new_buffers){
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(vao);
        glVertexAttrib2f(0,x, y);
        glVertexAttrib2f(1,w, h);
        glVertexAttribI1i(2, 0);
    }

    intensity = 0;
    intensity_integral = 0;
    name = prefix;

    int n = 0;
    for(;;) {
        struct stat statbuf;
        auto filename = std::string{config.pattern.dir} + prefix + "." + std::to_string(n) + ".glsl";
        int rc = stat(filename.c_str(), &statbuf);

        if (rc != 0 || S_ISDIR(statbuf.st_mode))
            break;
        n++;
    }
    if(n == 0) {
        ERROR("Could not find any shaders for %s", prefix.c_str());
        throw std::system_error(EINVAL,std::system_category(),"failed to load shader.");
    }
    shader.clear();
    shader.reserve(n);
    tex.resize(n + 1);

    auto success = true;
    for(auto i = 0; i < n; i++) {
        auto filename = std::string{config.pattern.dir} + prefix + "." + std::to_string(i) + ".glsl";
        auto h = load_program({"#lib.glsl"},"#header.glsl",{filename.c_str()});
        if(!h) {
            success = false;
        }
        if (h == 0) {
            fprintf(stderr, "%s", get_load_program_error().c_str());
            WARN("Unable to load shader %s", filename.c_str());
            success = false;
        } else {
            glProgramUniform2f(h, 0,  config.pattern.master_width, config.pattern.master_height);
            GLint tex_assignments[3] = { 1, 2, 3};
            glProgramUniform1iv(h, 7, 3, tex_assignments);
            shader.push_back(h);
            DEBUG("Loaded shader #%d", i);
        }
    }
    if(!success) {
        ERROR("Failed to load some shaders.");
        throw std::system_error(EINVAL,std::system_category(),"failed to load shader.");
    }
    CHECK_GL();

    // Render targets
    glGenFramebuffers(1, &fb);
    CHECK_GL();
    glGenTextures(tex.size(), &tex[0]);
    CHECK_GL();
    layers.resize(tex.size());
    std::iota(layers.begin(),layers.end(),tex_layer);
    for(auto i = 0ul; i < tex.size();++i) {
        auto &t = tex[i];
//        t = make_view(GL_TEXTURE_2D,tex_array,GL_RGBA32F, layers[i],1);
        t = make_texture( config.pattern.master_width, config.pattern.master_height);
        glClearTexImage(t, 0, GL_RGBA, GL_FLOAT, nullptr);
        CHECK_GL();
    }
    CHECK_GL();

    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    CHECK_GL();

}

pattern::~pattern()
{
    for(auto sh : shader) {
        glDeleteProgram(sh);
    }
    shader.clear();
    glDeleteTextures(tex.size(), &tex[0]);
    tex.clear();
    glDeleteFramebuffers(1, &fb);
    CHECK_GL();
    fb = 0;
}

void pattern::render(GLuint input_tex) {

//    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBindVertexArray(vao);
    glViewport(0, 0, config.pattern.master_width, config.pattern.master_height);

    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    CHECK_GL();

    intensity_integral = std::fmod(intensity_integral + intensity / config.ui.fps, MAX_INTEGRAL);
    glBindTextures(1,tex.size() - 1, tex.data());
    glDisable(GL_BLEND);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, input_tex);
    CHECK_GL();
    for (auto i = int(shader.size()) - 1; i >= 0; --i) {
        glUseProgram(shader[i]);

        glActiveTexture(GL_TEXTURE1 + i);
        glBindTexture(GL_TEXTURE_2D, tex[i]);
        CHECK_GL();
        glFramebufferTexture(
            GL_FRAMEBUFFER
          , GL_COLOR_ATTACHMENT0
          , tex.back()
          , 0);

        CHECK_GL();
        glUniform1f(1, time_master.beat_frac + time_master.beat_index);
        glUniform4f(2, audio_low, audio_mid, audio_hi, audio_level);
        glUniform1f(3, intensity);
        glUniform1f(4, intensity_integral);
        glUniform1f(5, config.ui.fps);


        glDrawArrays(GL_POINTS, 0, 1);
        glTextureBarrier();
        CHECK_GL();
        std::swap(tex.back(), tex[i]);
        std::swap(layers.back(),layers[i]);
    }
    CHECK_GL();
    tex_output = tex.back();
    out_layer  = layers.back();
}
