#include "sdlang.h"

#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>

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
    const int len = (const int)(token->string.to - token->string.from);

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
    case SDLANG_TOKEN_UINT32:
        fprintf(stdout, "- value(hex32): %.*s", len, text);
        is_attribute = false;
        break;
    case SDLANG_TOKEN_UINT64:
        fprintf(stdout, "- value(hex64): %.*s", len, text);
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

static void begin_block(const char* node, void* user)
{
    fprintf(stdout, "node '%s', {\n", node);
}

static void end_block(void* user)
{
    fprintf(stdout, "}\n");
}

static void emit_value_i32(const char* node, const char* attr, int32_t value, void* user)
{
    fprintf(stdout, "node '%s', attr '%s', i32=%d\n", node, attr, value);
}

static void emit_value_i64(const char* node, const char* attr, int64_t value, void* user)
{
    fprintf(stdout, "node '%s', attr '%s', i64=%"PRId64"\n", node, attr, value);
}

static void emit_value_i128(const char* node, const char* attr, int64_t hi, uint64_t lo, void* user)
{
    fprintf(stdout, "node '%s', attr '%s', i128=[hi %"PRId64", lo=%"PRIu64"]\n", node, attr, hi, lo);
}

static void emit_value_f32(const char* node, const char* attr, float value, void* user)
{
    fprintf(stdout, "node '%s', attr '%s', i32=%f\n", node, attr, value);
}

static void emit_value_f64(const char* node, const char* attr, double value, void* user)
{
    fprintf(stdout, "node '%s', attr '%s', f64=%f\n", node, attr, value);
}

static void emit_value_string(const char* node, const char* attr, const char* value, int len, void* user)
{
    fprintf(stdout, "node '%s', attr '%s', string '%.*s'\n", node, attr, len, value);
}

static void emit_value_base64(const char* node, const char* attr, const char* value, int len, void* user)
{
    fprintf(stdout, "node '%s', attr '%s', base64 [%.*s]\n", node, attr, len, value);
}

static void emit_value_u32(const char* node, const char* attr, uint32_t value, void* user)
{
    fprintf(stdout, "node '%s', attr '%s', u32=%u\n", node, attr, value);
}

static void emit_value_u64(const char* node, const char* attr, uint64_t value, void* user)
{
    fprintf(stdout, "node '%s', attr '%s', u64=%"PRIu64"\n", node, attr, value);
}

static void emit_value_bool(const char* node, const char* attr, bool value, void* user)
{
    fprintf(stdout, "node '%s', attr '%s', bool=%s\n", node, attr, value ? "true" : "false");
}

static void emit_value_null(const char* node, const char* attr, void* user)
{
    fprintf(stdout, "node '%s', attr '%s', null\n", node, attr);
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
    int mode = 0;

    if (argc > 1)
    {
        if (argc > 2)
        {
            if (strcmp(argv[1], "-t") == 0)
            {
                mode = 1;
            }
        }

        file = fopen(argv[argc - 1], "rb");

        if (file == NULL)
        {
            fprintf(stderr, "failed to open: %s\n", argv[argc - 1]);
            return 1;
        }
    }

    if (mode == 0)
    {
        sdlang_set_emit_functions(&(struct sdlang_functions_t) {
            .block_begin = begin_block,
            .block_end = end_block,
            .value_i32 = emit_value_i32,
            .value_i64 = emit_value_i64,
            .value_i128 = emit_value_i128,
            .value_f32 = emit_value_f32,
            .value_f64 = emit_value_f64,
            .value_string = emit_value_string,
            .value_base64 = emit_value_base64,
            .value_u32 = emit_value_u32,
            .value_u64 = emit_value_u64,
            .value_bool = emit_value_bool,
            .value_null = emit_value_null
        });
    }
    else
    {
        sdlang_set_emit_token(emit_token);
    }

    sdlang_set_report_error(report_error);

    const int result = sdlang_parse(read, file);

    if (file != NULL)
    {
        fclose(file);
    }

    return result;
}
