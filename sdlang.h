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

/*#
    ## types
#*/

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
    SDLANG_PARSE_OK = 0,
    SDLANG_PARSE_ERROR,
    SDLANG_PARSE_ERROR_STACK_OVERFLOW,
    SDLANG_PARSE_ERROR_BUFFER_TOO_SMALL
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

/*#
    ## functions
#*/

/*#

    ### sdlang_emit_token

    ~~~ C
    void sdlang_emit_token(const struct sdlang_token_t* token, void* user);
    ~~~

    Injects a token into the emitter stream.

    Under normal conditions, calling this function isn't required. The main
    reason to have it exposed is to allow pass-through calls for custom
    callbacks:

    ~~~ C
    void emit_token(const struct sdlang_token_t* token, void* user)
    {
        if (context_wants_capture_token(user))
        {
            // process token
        }
        else
        {
            sdlang_emit_token(token, user);
        }
    }

    // ...

    sdlang_set_emit_token(emit_token, user);
    ~~~

#*/
extern void sdlang_emit_token(const struct sdlang_token_t* token, void* user);

/*#

    ### sdlang_set_emit_token

    ~~~ C
    void sdlang_set_emit_token(void (*emit_token)(const struct sdlang_token_t* token, void* user));
    ~~~

    Captures the token function. Pass NULL to set the default function.

    This is the callback used by the parser at the lowest level. By capturing
    this function, any APIs at a higher level are effectively cut off. You can
    forward the call to `sdlang_emit_token()` to prevent this.

#*/
extern void sdlang_set_emit_token(void (*emit_token)(const struct sdlang_token_t* token, void* user));

/*#

    ### sdlang_set_report_error

    ~~~ C
    void sdlang_set_report_error(void (*report_error)(enum sdlang_error_t error, int line));
    ~~~

    Sets the error report function. Pass NULL to set the default function,
    which is an empty implementation.

    See __samples/parser.c__ for an example.

#*/
extern void sdlang_set_report_error(void (*report_error)(enum sdlang_error_t error, int line));

/*#

    ### sdlang_parse

    ~~~ C
    int sdlang_parse(size_t (*stream)(void* ptr, size_t size, void* user), void* user);
    ~~~

    Parses a SDLang document from an input stream.

    The first argument is a stream function which writes up to `size` bytes to
    memory at `ptr`, and returns the number of bytes effectively written.

    The `user` parameter is a user-defined context pointer forwarded to most
    callbacks, and can be `NULL`.

    ~~~ C
    size_t stream_function(void* ptr, size_t size, void* user)
    {
        FILE* file = get_file_handle_from_user_context(user);
        return fread(ptr, 1, size, file);
    }
    ~~~

    Returns 0 on success. If an error occured, a positive number is returned,
    which can be mapped to `sdlang_error_t`.

#*/
extern int sdlang_parse(size_t (*stream)(void* ptr, size_t size, void* user), void* user);

#ifdef __cplusplus
}
#endif
