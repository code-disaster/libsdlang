#include "sdlang.h"

void sdlang_emit_token(const struct sdlang_token_t* token, void* user)
{
    (void)token;
    (void)user;
}

static void (*sdlang_user_emit_token)(const struct sdlang_token_t*, void*) = sdlang_emit_token;

void sdlang_set_emit_token(void (*emit_token)(const struct sdlang_token_t* token, void* user))
{
    sdlang_user_emit_token = emit_token != NULL ? emit_token : sdlang_emit_token;
}

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

    (*sdlang_user_emit_token)(&token, user);
}

static void sdlang_report_error(enum sdlang_error_t error, int line)
{
    (void)error;
    (void)line;
}

static void (*sdlang_user_report_error)(enum sdlang_error_t error, int line) = sdlang_report_error;

void sdlang_set_report_error(void (*report_error)(enum sdlang_error_t error, int line))
{
    sdlang_user_report_error = report_error != NULL ? report_error : sdlang_report_error;
}

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
