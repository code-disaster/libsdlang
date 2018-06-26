#include "sdlang.h"

#include <stdio.h>
#include <stdbool.h>

static bool is_node = false;
static bool is_attribute = false;
static int depth = 0;

static size_t read(void* ptr, size_t size, void* user)
{
    FILE* file = user;
    return fread(ptr, 1, size, file);
}

static void emit_token(const struct sdlang_token_t* token, void* user)
{
    if (token->type == SDLANG_TOKEN_NODE_END)
    {
        is_node = false;
        is_attribute = false;
        return;
    }

    if (token->type == SDLANG_TOKEN_BLOCK)
    {
        ++depth;
        return;
    }
    else if (token->type == SDLANG_TOKEN_BLOCK_END)
    {
        --depth;
        return;
    }

    for (int d = 0; d < depth; d++)
    {
        fprintf(stdout, "    ");
    }

    const char* text = token->string.from;
    const int len = token->string.to - token->string.from;

    switch (token->type)
    {
    case SDLANG_TOKEN_NODE:
        fprintf(stdout, "%.*s", len, text);
        is_node = true;
        break;
    case SDLANG_TOKEN_BLOCK:
        ++depth;
        break;
    case SDLANG_TOKEN_BLOCK_END:
        --depth;
        break;
    case SDLANG_TOKEN_ATTRIBUTE:
        fprintf(stdout, "- attr: %.*s", len, text);
        is_attribute = true;
        break;
    case SDLANG_TOKEN_INT32:
        fprintf(stdout, "- value(i32): %.*s", len, text);
        is_attribute = false;
        break;
    case SDLANG_TOKEN_INT64:
        fprintf(stdout, "- value(i64): %.*s", len, text);
        is_attribute = false;
        break;
    case SDLANG_TOKEN_INT128:
        fprintf(stdout, "- value(i128): %.*s", len, text);
        is_attribute = false;
        break;
    case SDLANG_TOKEN_FLOAT32:
        fprintf(stdout, "- value(f32): %.*s", len, text);
        is_attribute = false;
        break;
    case SDLANG_TOKEN_FLOAT64:
        fprintf(stdout, "- value(f64): %.*s", len, text);
        is_attribute = false;
        break;
    case SDLANG_TOKEN_STRING:
        fprintf(stdout, "- value(str): %.*s", len, text);
        is_attribute = false;
        break;
    case SDLANG_TOKEN_BASE64:
        fprintf(stdout, "- value(base64): %.*s", len, text);
        is_attribute = false;
        break;
    case SDLANG_TOKEN_TRUE:
        fprintf(stdout, "- value: true");
        is_attribute = false;
        break;
    case SDLANG_TOKEN_FALSE:
        fprintf(stdout, "- value: false");
        is_attribute = false;
        break;
    case SDLANG_TOKEN_NULL:
        fprintf(stdout, "- value: null");
        is_attribute = false;
        break;

    case SDLANG_TOKEN_NODE_END:
    default:
        break;
    }

    fprintf(stdout, "\n");
}

static void report_error(enum sdlang_error_t error, int line)
{
    switch (error)
    {
    case SDLANG_PARSE_ERROR:
        fprintf(stderr, "parse error at line %d\n", line);
        break;
    case SDLANG_PARSE_ERROR_STACK_OVERFLOW:
        fprintf(stderr, "parse stack overflow at line %d\n", line);
        break;
    case SDLANG_PARSE_ERROR_BUFFER_TOO_SMALL:
        fprintf(stderr, "out of buffer memory at line %d\n", line);
        break;
    default:
        fprintf(stderr, "unknown error [%d] at line %d\n", error, line);
        break;
    }
}

int main(int argc, char* argv[])
{
    FILE* file = stdout;

    if (argc > 1)
    {
        file = fopen(argv[1], "rb");
        
        if (file == NULL)
        {
            fprintf(stderr, "failed to open: %s\n", argv[1]);
            return 1;
        }
    }

    sdlang_set_emit_token(emit_token);
    sdlang_set_report_error(report_error);

    const int result = sdlang_parse(read, file);

    if (file != NULL)
    {
        fclose(file);
    }

    return result;
}
