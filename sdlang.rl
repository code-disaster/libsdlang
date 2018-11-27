/*******************************************************************************
 * This file is generated by Ragel. Do not edit!
 *   $ ragel -L -G2 -o sdlang.inl sdlang.rl
 ******************************************************************************/

%%{

    machine sdlang;

    prepush {
        check_stack_size(&p, pe, top, curline);
    }

    # line breaks, with line counter

    newline = '\r'? '\n' @{curline += 1;};
    newline_wrap = '\\' newline;
    any_count_line = any | newline;

    # quoted strings
    #   + optionally use ' to delimit single-quoted strings

    single_quote_char = [^'`\\] | newline | ( '\\' . any_count_line );
    single_quote_string = ['`]{1} . single_quote_char* . ['`]{1};

    double_quote_char = [^"\\] | newline | ( '\\' any_count_line );
    double_quote_string = '"' . double_quote_char* . '"';

    any_string = single_quote_string | double_quote_string;

    # boolean and null values

    kw_true = 'true' | 'on';
    kw_false = 'false' | 'off';
    kw_null = 'null';
    keywords = kw_true | kw_false | kw_null;

    # literals (node names)

    alnum_literal = alnum | [\.\:\-_$];
    literal = '_'? . alpha . alnum_literal* - keywords;

    # comments

    one_line_comment = ('//' | '--' | '#') [^\n]* newline;
    c_comment := any_count_line* :>> '*/' @{fgoto block;};

    # numeric values
    #   + supports floating point e-notation
    #   + explicitely captures trailing '+'/'-'
    #   + 32/64 bit hexadecimal numbers

    float_fract = digit* '.' digit+ | digit+ '.';
    float_exp = [eE] [+\-]? digit+;

    float64 = [+\-]? (float_fract float_exp? | digit+ float_exp);
    float32 = float64 [fF];

    int32 = [+\-]? ('0' | [1-9] [0-9]*);
    int64 = int32 [lL];
    int128 = int32 [bB] [dD];

    hex32 = '0' [xX] [0-9a-fA-F]{1,8};
    hex64 = '0' [xX] [0-9a-fA-F]{9,16};

    # base64

    base64_char = alnum | [+/=];
    base64_string = '[' . base64_char* . ']';

    # attributes

    attribute = literal '=';

    block := |*

        attribute {emit(SDLANG_TOKEN_ATTRIBUTE, ts, te, curline, user);};

        literal {emit(SDLANG_TOKEN_NODE, ts, te, curline, user);};

        any_string {emit(SDLANG_TOKEN_STRING, ts, te, curline, user);};

        float32 {emit(SDLANG_TOKEN_FLOAT32, ts, te, curline, user);};
        float64 {emit(SDLANG_TOKEN_FLOAT64, ts, te, curline, user);};

        int64 {emit(SDLANG_TOKEN_INT64, ts, te, curline, user);};
        int128 {emit(SDLANG_TOKEN_INT128, ts, te, curline, user);};
        int32 {emit(SDLANG_TOKEN_INT32, ts, te, curline, user);};

        hex64 {emit(SDLANG_TOKEN_UINT64, ts, te, curline, user);};
        hex32 {emit(SDLANG_TOKEN_UINT32, ts, te, curline, user);};

        kw_true {emit(SDLANG_TOKEN_TRUE, ts, te, curline, user);};
        kw_false {emit(SDLANG_TOKEN_FALSE, ts, te, curline, user);};

        kw_null {emit(SDLANG_TOKEN_NULL, ts, te, curline, user);};

        base64_string {emit(SDLANG_TOKEN_BASE64, ts, te, curline, user);};
# data/time formats
# skip empty lines
# lazy token_end

        ';' {emit(SDLANG_TOKEN_NODE_END, ts, te, curline, user);};

        '{' {
            emit(SDLANG_TOKEN_BLOCK, ts, te, curline, user);
            fcall block;
        };

        '}' {
            emit(SDLANG_TOKEN_BLOCK_END, ts, te, curline, user);
            fret;
        };

        one_line_comment {emit(SDLANG_TOKEN_NODE_END, NULL, NULL, curline, user);};
        '/*' {fgoto c_comment;};

        newline_wrap; # wrapping lines do not end node
        newline {emit(SDLANG_TOKEN_NODE_END, NULL, NULL, curline, user);};

        [ \t];

    *|;

    main := |*
        # Just enter the top-level block.
        any {fhold; fcall block;};
    *|;

}%%

%% write data nofinal;

int sdlang_parse(size_t (*stream)(void* ptr, size_t size, void* user), void* user)
{
    char buf[SDLANG_PARSE_BUFFERSIZE];
    int cs, act, have = 0, curline = 1;
    int stack[SDLANG_PARSE_STACKSIZE], top = 0;
    char *ts, *te = 0;
    int done = 0, err = SDLANG_PARSE_OK;

    %% write init;

    while (!done)
    {
        char *p = buf + have, *pe, *eof = 0;
        int len, space = SDLANG_PARSE_BUFFERSIZE - have;

        if (space == 0)
        {
            err = SDLANG_PARSE_ERROR_BUFFER_TOO_SMALL;
            break;
        }

        len = (int)stream(p, space, user);
        pe = p + len;

        if (len < space)
        {
            eof = pe;
            done = 1;
        }

        %% write exec;

        if (cs == sdlang_error)
        {
            err = SDLANG_PARSE_ERROR;
            break;
        }

        if (top == SDLANG_PARSE_STACKSIZE)
        {
            err = SDLANG_PARSE_ERROR_STACK_OVERFLOW;
            break;
        }

        if (ts == 0)
        {
            have = 0;
        }
        else
        {
            have = (int)(pe - ts);
            SDLANG_MEMMOVE(buf, ts, have);
            te = buf + (te - ts);
            ts = buf;
        }
    }

    if (err != SDLANG_PARSE_OK)
    {
        (*sdlang_user_report_error)(err, curline);
    }

    return err;
}
