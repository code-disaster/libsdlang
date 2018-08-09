#include "sdlang.h"

#include <stdlib.h>

static struct sdlang_functions_t* sdlang_user_emit_functions = NULL;

void sdlang_set_emit_functions(struct sdlang_functions_t* emit_functions)
{
    if (emit_functions != NULL)
    {
        emit_functions->node_name[0] = '\0';
        emit_functions->attr_name[0] = '\0';
    }

    sdlang_user_emit_functions = emit_functions;
}

void sdlang_emit_token(const struct sdlang_token_t* token, void* user)
{
    struct sdlang_functions_t* vtbl = sdlang_user_emit_functions;

    if (vtbl == NULL)
    {
        return;
    }

    const char* value = token->string.from;
    const int len = (const int)(token->string.to - value);

#define size_minus_one(dst) \
    ((int)sizeof(vtbl->dst) - 1)

#define safe_copy_value(dst) \
    strncpy(vtbl->dst, value, len < size_minus_one(dst) ? len : size_minus_one(dst)); \
    vtbl->dst[len < size_minus_one(dst) ? len : size_minus_one(dst)] = '\0'

#define safe_call(fn, params) \
    if (vtbl->fn != NULL) vtbl->fn params

#define safe_emit_value(fn, ...) \
    if (vtbl->value_##fn != NULL) vtbl->value_##fn(vtbl->node_name, vtbl->attr_name, __VA_ARGS__); \
    vtbl->attr_name[0] = '\0';

    switch (token->type)
    {
    case SDLANG_TOKEN_NODE:
        {
            safe_copy_value(node_name);
        }
        break;

    case SDLANG_TOKEN_NODE_END:
        {
            vtbl->node_name[0] = '\0';
        }
        break;

    case SDLANG_TOKEN_BLOCK:
        {
            safe_call(block_begin, (vtbl->node_name, user));
        }
        break;

    case SDLANG_TOKEN_BLOCK_END:
        {
            safe_call(block_end, (user));
        }
        break;

    case SDLANG_TOKEN_ATTRIBUTE:
        {
            /* store attribute name */
            safe_copy_value(attr_name);
        }
        break;

    case SDLANG_TOKEN_INT32:
        {
            const long l = strtol(value, NULL, 10);
            safe_emit_value(i32, l, user);
        }
        break;

    case SDLANG_TOKEN_INT64:
        {
            const long long ll = strtoll(value, NULL, 10);
            safe_emit_value(i64, ll, user);
        }
        break;

    case SDLANG_TOKEN_INT128:
        {
            int64_t hi = 0;
            uint64_t lo = 0;

            const char *s = value, *e = &value[len];
            int64_t sign = 1;

            if (*s == '-')
            {
                sign = -sign;
                ++s;
            }

            while (s != e)
            {
                const uint64_t digit = *s - '0';

                if (lo * 10 + digit < lo)
                {
                    hi = (hi << 4) | (lo >> 48);
                    lo >>= 4;
                }

                lo = lo * 10 + digit;

                ++s;
            }

            hi = sign * hi;

            safe_emit_value(i128, hi, lo, user);
        }
        break;

    case SDLANG_TOKEN_FLOAT32:
        {
            const float f = strtof(value, NULL);
            safe_emit_value(f32, f, user);
        }
        break;

    case SDLANG_TOKEN_FLOAT64:
        {
            const double d = strtod(value, NULL);
            safe_emit_value(f64, d, user);
        }
        break;

    case SDLANG_TOKEN_STRING:
        {
            safe_emit_value(string, value, len, user);
        }
        break;

    case SDLANG_TOKEN_BASE64:
        {
            safe_emit_value(base64, value, len, user);
        }
        break;

    case SDLANG_TOKEN_UINT32:
        {
            const unsigned long ul = strtoul(value, NULL, 16);
            safe_emit_value(u32, ul, user);
        }
        break;

    case SDLANG_TOKEN_UINT64:
        {
            const unsigned long long ull = strtoull(value, NULL, 16);
            safe_emit_value(u64, ull, user);
        }
        break;

    case SDLANG_TOKEN_TRUE:
        {
            safe_emit_value(bool, true, user);
        }
        break;

    case SDLANG_TOKEN_FALSE:
        {
            safe_emit_value(bool, false, user);
        }
        break;

    case SDLANG_TOKEN_NULL:
        {
            safe_emit_value(null, user);
        }
        break;

    default:
        break;
    }
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
    case SDLANG_TOKEN_FLOAT32:
    case SDLANG_TOKEN_FLOAT64:
        /* strip leading '+' */
        if (*ts == '+')
        {
            ++ts;
        }
        break;

    case SDLANG_TOKEN_INT128:
        /* strip leading '+' and suffix */
        if (*ts == '+')
        {
            ++ts;
        }
        te -= 2;
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
