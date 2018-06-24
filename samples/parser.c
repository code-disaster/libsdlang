#include "sdlang.h"

#include <stdio.h>
#include <stdbool.h>

static FILE* file = NULL;

static bool is_node = false;
static bool is_attribute = false;
static int depth = 0;

static size_t read(void* ptr, size_t size, size_t nmemb)
{
    return fread(ptr, size, nmemb, file ? file : stdin);
}

static void emit_token(const struct sdlang_token_t* token)
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

int main(int argc, char* argv[])
{
    if (argc > 1)
    {
        file = fopen(argv[1], "rb");
        
        if (file == NULL)
        {
            fprintf(stderr, "failed to open: %s\n", argv[1]);
            return 1;
        }
    }

    sdlang_emit_token = emit_token;

    const int result = sdlang_parse(read);

    if (file != NULL)
    {
        fclose(file);
    }

    return result;
}
