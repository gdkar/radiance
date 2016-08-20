#pragma once

#include "util/common.h"
#ifdef __cplusplus
extern "C" {
#endif
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _ERR_STRINGIFY2(x) #x
#define _ERR_STRINGIFY(x) _ERR_STRINGIFY2(x)

typedef enum loglevel {
    LOGLEVEL_ALL = 0,
    LOGLEVEL_DEBUG,
    LOGLEVEL_INFO,
    LOGLEVEL_WARN,
    LOGLEVEL_ERROR,
} loglevel;

loglevel get_loglevel(void);
loglevel set_loglevel(loglevel level);
#define LOGLIMIT(LVL, msg, ...) ({          \
    static unsigned long _ntimes = 0;       \
    static unsigned long _limit = 4;        \
    if (_ntimes == 2 *_limit)  _limit *= 2; \
    if (_ntimes++ <= _limit)                \
        LVL("[%8lu] " msg, _ntimes, ## __VA_ARGS__); \
})

#define _ERR_MSG(severity, msg, ...) ({if (get_loglevel() <= LOGLEVEL_ ## severity) { fprintf(stderr, "[%-5s] [%s:%d:%s] " msg "\n", _ERR_STRINGIFY(severity), __FILE__, __LINE__, __func__, ## __VA_ARGS__);fflush(stderr); } })

#define FAIL(...) ({ERROR(__VA_ARGS__); exit(EXIT_FAILURE);})
#define ERROR(...) _ERR_MSG(ERROR, ## __VA_ARGS__)
#define WARN(...)  _ERR_MSG(WARN,  ## __VA_ARGS__)
#define INFO(...)  _ERR_MSG(INFO,  ## __VA_ARGS__)
#define DEBUG(...) _ERR_MSG(DEBUG, ## __VA_ARGS__)
#define MEMFAIL() PFAIL("Could not allocate memory")

#define PFAIL(...) ({ERROR(__VA_ARGS__); exit(EXIT_FAILURE);})
#define PERROR(msg, ...) _ERR_MSG(ERROR,"[%s] " msg, strerror(errno), ## __VA_ARGS__)
#define CHECK_GL() do {if(auto e = glGetError()) FAIL("OpenGL Error: %d, \"%s\"\n", int(e), gluErrorString(e));}while(false)
//#define CHECK_GL() do{}while(0)

#ifdef __cplusplus
#include <iostream>
#include <cstdio>
#include <sstream>
#include <string>
#include <fstream>
inline std::string to_string(loglevel lvl)
{
    switch(lvl) {
        case LOGLEVEL_ALL: return "ALL";
        case LOGLEVEL_DEBUG: return "DEBUG";
        case LOGLEVEL_INFO: return "INFO";
        case LOGLEVEL_WARN: return "WARN";
        case LOGLEVEL_ERROR: return "ERROR";
        default: return "INVALID LOGLEVEL";
    };
}
inline std::ostream &operator <<(std::ostream &ost, loglevel lvl)
{
    return ost << to_string(lvl);
}
#endif

/*
#include <execinfo.h>
#define BACKTRACE() ({ \
    INFO("Backtrace:"); \
    void * _buffer[100]; \
    int _nptrs = backtrace(_buffer, 100); \
    backtrace_symbols_fd(_buffer, _nptrs, fileno(stderr)); \
})
*/
#ifdef __cplusplus
}
#endif
