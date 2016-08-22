#pragma once

#include "util/common.h"

#define MAX_INTEGRAL 1024

struct pattern {
    std::vector<GLuint> shader{};
    std::string name{};
    double intensity{};
    double intensity_integral{};

    GLuint tex_array;
    int    tex_layer;
    int    pid;
    std::vector<GLuint> tex;
    std::vector<int>    layers;
    GLuint tex_output{};
    int    out_layer{};
    pattern( const std::string &prefix, GLuint tex_array, int layer, int pid);
    virtual ~pattern();
    void render(GLuint input_tex, int input_layer);
};
