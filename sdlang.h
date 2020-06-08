#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifndef SDLANG_PARSE_BUFFERSIZE
# define SDLANG_PARSE_BUFFERSIZE 1024
#endif

#ifndef SDLANG_PARSE_STACKSIZE
# define SDLANG_PARSE_STACKSIZE 32
#endif

#ifndef SDLANG_MEMMOVE
# include <string.h>
# define SDLANG_MEMMOVE(d, s, n) memmove(d, s, n)
#endif

#define SDLANG_NODE_MAXNAMELEN 48
#define SDLANG_ATTR_MAXNAMELEN 48

#ifdef __cplusplus
extern "C" {
#endif

/*#
    ## types
#*/

/*#
    ### sdlang_token_type_t
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
    SDLANG_TOKEN_UINT32,
    SDLANG_TOKEN_UINT64,

    SDLANG_TOKEN_TRUE,
    SDLANG_TOKEN_FALSE,
    SDLANG_TOKEN_NULL
};

/*#
    ### sdlang_error_t
#*/
enum sdlang_error_t
{
    SDLANG_PARSE_OK = 0,
    SDLANG_PARSE_ERROR,
    SDLANG_PARSE_ERROR_STACK_OVERFLOW,
    SDLANG_PARSE_ERROR_BUFFER_TOO_SMALL
};

/*#
    ### sdlang_token_t

    Token description as identified by the parser.

    The `string` variable points at the first and after the last `char` of
    the character sequence in relation to the token type. For a node or
    attribute, it's the node/attribute name. For value types, it's the actual
    value.

    For some token types, like `SDLANG_TOKEN_NODE_END`, `string` pointers may
    be set to `NULL`.

    !!! WARNING
        `string.from` and `string.to` point to buffer content which may not be
        valid anymore after subsequent parsing operations. You'll need to copy
        the string content if you want to access it later.
#*/
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
    ### sdlang_functions_t

    User function table.

    Node and attribute names are null-terminated. Any other string values are
    not - the caller needs to ensure to not read past `len` characters.
    
    Any node names, attribute names and character buffers addressed by `ptr`
    are only valid during callbacks, and can/will be overwritten by the parser
    afterwards.

    If a function pointer is set to `NULL`, any corresponding callback will be
    discarded.

    !!! WARNING
        `node_name` and `attr_name` are used internally as a temporary storage
        for node and attribute names. Consider their content read-only during,
        and undefined after execution of the parser.
#*/
struct sdlang_functions_t
{
    void (*block_begin)(const char* node, void* user);
    void (*block_end)(void* user);
    void (*value_i32)(const char* node, const char* attr, int32_t value, void* user);
    void (*value_i64)(const char* node, const char* attr, int64_t value, void* user);
    void (*value_i128)(const char* node, const char* attr, int64_t hi, uint64_t lo, void* user);
    void (*value_f32)(const char* node, const char* attr, float value, void* user);
    void (*value_f64)(const char* node, const char* attr, double value, void* user);
    void (*value_string)(const char* node, const char* attr, const char* ptr, int len, void* user);
    void (*value_base64)(const char* node, const char* attr, const char* ptr, int len, void* user);
    void (*value_u32)(const char* node, const char* attr, uint32_t value, void* user);
    void (*value_u64)(const char* node, const char* attr, uint64_t value, void* user);
    void (*value_bool)(const char* node, const char* attr, bool value, void* user);
    void (*value_null)(const char* node, const char* attr, void* user);

    /* local node/attribute name buffers */
    char node_name[SDLANG_NODE_MAXNAMELEN];
    char attr_name[SDLANG_ATTR_MAXNAMELEN];
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

        sdlang_emit_token(token, user);
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
    ### sdlang_set_emit_functions

    ~~~ C
    void sdlang_set_emit_functions(struct sdlang_functions_t* emit_functions);
    ~~~

    Populates the user-callback function table. Pass NULL to clear and not
    generate any more callbacks.

    This is the higher level user interface. The parser translates low level
    tokens to calls into this function table.

    Most functions take a `node` and `attribute` parameter. For anonymous
    nodes, the `node` parameter is empty. For node values, the `attributes`
    parameter is empty.

    There are no explicit callbacks to begin or end a node. These events can
    be implied by a change to the `node` parameter from one callback to the
    next.
#*/
extern void sdlang_set_emit_functions(struct sdlang_functions_t* emit_functions);

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
