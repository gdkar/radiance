#pragma once
#include "util/common.h"
#include "util/semaphore.hpp"

struct render {
    size_t pixel_count;
    GLuint vbo;
    GLuint pbo;
    GLuint ssbo;
    GLuint prog;
    GLuint fb;
    GLsync fence;
    semaphore sem{};
    GLfloat * pixels;
    SDL_mutex * mutex;
};

void render_init(struct render * render, GLint texture);
void render_readback(struct render * render);
void render_term(struct render * render);

bool render_freeze(struct render * render);
void render_thaw(struct render * render);
SDL_Color render_sample(struct render * render, float x, float y);
