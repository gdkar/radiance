#pragma once

#include "util/common.h"

#define MAX_INTEGRAL 1024

struct pattern {
    std::vector<GLuint> shader{};
    std::string name{};
    double intensity{};
    double intensity_integral{};

    std::vector<GLuint> tex;
    GLuint fb{};
    GLuint tex_output{};
    pattern( const std::string &prefix);
    virtual ~pattern();
    void render(GLuint input_tex);
};
