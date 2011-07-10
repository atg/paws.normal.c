#import <stdio.h>

typedef uchar uint8_t;

///////// PARSE TREE /////////
// https://gist.github.com/86a8d5e776f09b4f9dfd

typedef enum {
    AST_SCOPE, // { expr ('\n' expr)* '}'
    AST_JUXTA, // foo bar
    AST_NAME, // aka "terminals". 
} expr_kind_t;

struct _expr_t;

typedef struct {
    struct _expr_t** items;
} expr_scope_t;

typedef struct {
    struct _expr_t* left;
    struct _expr_t* right;
} expr_juxta_t;

typedef struct {
    const uchar* name;
} expr_name_t;

typedef struct _expr_t {
    expr_kind_t kind;
    union {
        expr_scope_t scope;
        expr_juxta_t juxta;
        expr_name_t name;
    } inner;
} expr_t;



///////// PARSER /////////

typedef enum {
    END = 0,
    NEWLINE,
    NAME,
    L_PAREN = '(',
    R_PAREN = ')',
    L_BRACE = '{',
    R_BRACE = '}'
} token_kind_t;

typedef struct {
    token_kind_t kind;
    const uchar* name;
} token_t;

static token_t token(token_kind_t kind, const uchar* name)
{
    token_t t;
    t.kind = kind;
    t.name = name;
    return t;
}
static void token_free(token_t tok) {
    free(tok.name);
}

token_t lex_token(const uchar **p, _Bool lookahead) {
    uchar c = **p;
    
    if (c == '\0') {
        return token(END, NULL);
    }
    
    // TODO: In a real tokenizer we would want to multiple newlines
    if (c == '\n') {
        if (!lookahead)
            ++*p;
        return token(NEWLINE, NULL);
    }
    
    // TODO: In a real tokenizer we would want to ignore newlines after { or ( or the start of the program, and before } or ) or the end of the program
    if (c == '{' || c == '}' || c == '(' || c == ')') {
        if (!lookahead)
            ++*p;
        return token(c, NULL);
    }
    
    // TODO: This should really be a unicode quote/unquote character
    // but that's something we'd do with ragel
    if (c == '"') {
        // Find the length
        uchar* s = *p + 1;
        uint64_t length;
        while (1) {
            if (*s == '\0')
                error("Unexpected end of name");
            if (*s == '"')
                break;
            
            length++;
        }
        
        // Allocate
        uchar* namestr = malloc(sizeof(uchar) * length);
        
        // Copy
        memcpy(namestr, s, length);
        
        if (!lookahead)
            *p += length + 2;
        return token(NAME, namestr);
    }
    
    // Spaces are ignored
    if (c == ' ') {
        if (!lookahead)
            ++*p;
        return consume_token(p);
    }
    
    // Otherwise, it's an identifier
    // Find the length
    uchar* s = *p + 1;
    uint64_t length;
    while (1) {
        if (*s == '\0' || *s == ' '  || *s == '(' || *s == ')' || *s == '{' ||  || *s == '}')
            break;
        
        length++;
    }
    
    // Allocate
    uchar* namestr = malloc(sizeof(uchar) * length);
    
    // Copy
    memcpy(namestr, s, length);
    
    if (!lookahead)
        *p += length;
    return token(NAME, namestr);
}
token_t consume_token(const uchar **p) {
    return lex_token(p, true);
}
token_t lookahead_token(const uchar **p) {
    return lex_token(p, false);
}

ast_t parse(const uchar *input) {
    parse_scope(&input, false);
}
expr_t parse_expr(const uchar **p) {
    parse
}
expr_scope_t parse_scope(const uchar **p, _Bool expectBraces) {
    token_t tok;
    
    if (expectBraces) {
        tok = consume_token(p);
        if (tok.kind != '{') {
            error("Expected {");
        }
    }
    
    while (1) {
        tok = lookahead_token(p);
        
        if (tok.kind == '{') {
            parse_scope(p);
        }
        if (tok.kind == '(') {
            parse_paren_expr(p);
        }
        if (tok.kind == '(') {
            parse_name(p);
        }
        token_free(tok);
        
        
        tok = consume_token(p);
        if (tok.kind == NEWLINE)
            continue;
        if (expectBraces && tok.kind == '}')
            break;
        if (!expectBraces && tok.kind == END)
            break;
        
        if (expectBraces)
            error("Expected newline or }");
        else
            error("Expected newline or end-of-file");
    }
}




///////// RUNTIME /////////

typedef enum {
    INT,
    NAME,
    LIST
    EXPR, /* */
} object_kind_t;





typedef struct {
    object_kind_t kind;
    int64_t value;
} int_object_t;

typedef struct {
    object_kind_t kind;
    
} list_object_t;


///////// EXECUTIONS /////////

typedef struct {
    
    ast_pointer_t c;
    
} execution_t;

void run(ast_t ast) {
    
}

void stage(execution_t ex) {
    
}
void unstage(execution_t ex) {
    
}