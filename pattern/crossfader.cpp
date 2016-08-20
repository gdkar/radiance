#include "util/common.h"
#include "pattern/crossfader.h"
#include "util/glsl.h"
#include "util/string.h"
#include "util/err.h"
#include "util/config.h"
#include <string.h>
#include "ui/ui.h"
static GLuint vao = 0;
static GLuint vbo = 0;

void crossfader_init(struct crossfader * crossfader) {
    bool new_buffers = false;
    if(!vao) {
        glGenVertexArrays(1,&vao);
        new_buffers = true;
    }
    if(!vbo)  {
        glGenBuffers(1,&vbo);
        new_buffers = true;
        float w = config.pattern.master_width,h =  config.pattern.master_height;
        float x = 0.f, y = 0.f;
        GLfloat vertices[] = {
            x, y, w, h
        };
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glNamedBufferData(vbo, sizeof(vertices), vertices, GL_STATIC_DRAW);

    }
    if(new_buffers){
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBindVertexArray(vao);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2*sizeof(GLfloat)));
        glEnableVertexAttribArray(1);
    }

    memset(crossfader, 0, sizeof *crossfader);

    crossfader->position = 0.5;

    crossfader->shader = load_program({"#lib.glsl"},"#header_ui.glsl",{"#crossfader.glsl"});
    if(crossfader->shader == 0) FAIL("Unable to load crossfader shader:\n%s", get_load_program_error().c_str());
    auto h = crossfader->shader;
    glUseProgram(crossfader->shader);
    glUniform2f(0,  config.pattern.master_width, config.pattern.master_height);
    auto loc = crossfader->loc.iIntensity  = glGetUniformLocation(crossfader->shader, "iIntensity");
    glUniform1f(loc, crossfader->position);
    loc = crossfader->loc.iFrameLeft = glGetUniformLocation(crossfader->shader, "iFrameLeft");
    glProgramUniform1i(h,loc, 0);
    loc = crossfader->loc.iFrameRight = glGetUniformLocation(crossfader->shader, "iFrameRight");
    glProgramUniform1i(h,loc, 1);
    loc = crossfader->loc.iLeftOnTop = glGetUniformLocation(crossfader->shader, "iLeftOnTop");
    glProgramUniform1i(h, loc, crossfader->left_on_top);
    CHECK_GL();

    // Render targets
    glGenFramebuffers(1, &crossfader->fb);
    glGenTextures(1, &crossfader->tex_output);
    crossfader->tex_output = make_texture( GL_RGBA32F, config.pattern.master_width, config.pattern.master_height);
    glBindFramebuffer(GL_FRAMEBUFFER, crossfader->fb);
    glNamedFramebufferTexture(crossfader->fb, GL_COLOR_ATTACHMENT0,crossfader->tex_output, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    CHECK_GL();
}

void crossfader_term(struct crossfader * crossfader) {
    glDeleteTextures(1, &crossfader->tex_output);
    glDeleteFramebuffers(1, &crossfader->fb);
    glDeleteProgram(crossfader->shader);
    CHECK_GL();

    memset(crossfader, 0, sizeof *crossfader);
}

void crossfader_render(struct crossfader * crossfader, GLuint left, GLuint right)
{
    CHECK_GL();
    glViewport(0, 0, config.pattern.master_width, config.pattern.master_height);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, crossfader->fb);
    glUseProgram(crossfader->shader);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    GLuint texs[] = { left, right};
    glBindTextures(0, 2, texs);
    CHECK_GL();
    auto &loc = crossfader->loc;
    glProgramUniform1f(crossfader->shader,loc.iIntensity, crossfader->position);
    glProgramUniform1i(crossfader->shader, loc.iLeftOnTop, crossfader->left_on_top);
    CHECK_GL();

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glDrawArrays(GL_POINTS, 0, 1);
    CHECK_GL();

    if(crossfader->position == 1.) {
        crossfader->left_on_top = true;
    } else if(crossfader->position == 0.) {
        crossfader->left_on_top = false;
    }
}
