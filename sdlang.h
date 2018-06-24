#pragma once

#include <stdint.h>

#ifndef SDLANG_PARSE_BUFFERSIZE
# define SDLANG_PARSE_BUFFERSIZE 128
#endif

#ifndef SDLANG_PARSE_STACKSIZE
# define SDLANG_PARSE_STACKSIZE 32
#endif

#ifndef SDLANG_MEMMOVE
# include <string.h>
# define SDLANG_MEMMOVE(d, s, n) memmove(d, s, n)
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum sdlang_token_type_t
{
    SDLANG_TOKEN_NODE,
    SDLANG_TOKEN_NODE_END,
    SDLANG_TOKEN_BLOCK,
    SDLANG_TOKEN_BLOCK_END,
    SDLANG_TOKEN_ATTRIBUTE,

    SDLANG_TOKEN_INT32,
    SDLANG_TOKEN_INT64,
    SDLANG_TOKEN_INT128,
    SDLANG_TOKEN_FLOAT32,
    SDLANG_TOKEN_FLOAT64,
    SDLANG_TOKEN_STRING,
    SDLANG_TOKEN_BASE64,

    SDLANG_TOKEN_TRUE,
    SDLANG_TOKEN_FALSE,
    SDLANG_TOKEN_NULL
};

enum sdlang_error_t
{
    SDLANG_ERROR_PARSE_ERROR,
    SDLANG_ERROR_PARSE_STACK_ERROR,
    SDLANG_ERROR_OUT_OF_BUFFER
};

struct sdlang_token_t
{
    enum sdlang_token_type_t type;

    struct
    {
        const char* from;
        const char* to;
    } string;

    int line;
};

extern void (*sdlang_emit_token)(const struct sdlang_token_t* token);

extern void (*sdlang_report_error)(enum sdlang_error_t error, int line);

extern int sdlang_parse(size_t (*stream)(void* ptr, size_t size, size_t nmemb));

#ifdef __cplusplus
}
#endif
