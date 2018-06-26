#include "sdlang.h"

#include <stdio.h>

static void emit_token(const struct sdlang_token_t* token, void* user)
{
    (void)user;
    fprintf(stdout, "[%2d] type=%d, value=", token->line, token->type);
    if (token->string.from < token->string.to)
    {
        fwrite(token->string.from, 1, token->string.to - token->string.from, stdout);
    }
    else
    {
        fprintf(stdout, "n/a");
    }
    fprintf(stdout, "\n");
}

void (*sdlang_emit_token)(const struct sdlang_token_t*, void*) = emit_token;

static void emit(enum sdlang_token_type_t type, const char* ts,
                 const char* te, int line, void* user)
{
    switch (type)
    {
    case SDLANG_TOKEN_ATTRIBUTE:
        /* strip trailing '=' */
        --te;
        break;

    case SDLANG_TOKEN_INT32:
    case SDLANG_TOKEN_INT64:
    case SDLANG_TOKEN_INT128:
    case SDLANG_TOKEN_FLOAT32:
    case SDLANG_TOKEN_FLOAT64:
        /* strip leading '+' */
        if (*ts == '+')
        {
            ++ts;
        }
        break;

    case SDLANG_TOKEN_STRING:
    case SDLANG_TOKEN_BASE64:
        /* strip delimiters */
        ++ts;
        --te;
        break;

    default:
        break;
    }

    const struct sdlang_token_t token = {
        .type = type,
        .string = {
            .from = ts,
            .to = te
        },
        .line = line
    };

    (*sdlang_emit_token)(&token, user);
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

void (*sdlang_report_error)(enum sdlang_error_t error, int line) = report_error;

static void check_stack_size(char** p, char* pe, int top, int line)
{
    if (top == SDLANG_PARSE_STACKSIZE - 1)
    {
        /*
            trick the FSM into completing the current iteration,
            then the main loop checks for the stack pointer
        */
        *p = pe - 1;
    }
}

#include "sdlang.inl"
