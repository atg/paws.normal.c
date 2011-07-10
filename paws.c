#include <stdio>
#include <deque>

typedef uchar uint8_t;

#define list_t(T) std::deque<T>*
#define list_create(T) new std::deque<T>
#define list_push(l, x) ((l)->push_back(x))
#define list_free(l) delete l;

///////// PARSE TREE /////////
// https://gist.github.com/86a8d5e776f09b4f9dfd

typedef enum {
    EXPR_SCOPE, // { expr ('\n' expr)* '}'
    EXPR_JUXTA, // foo bar
    EXPR_NAME, // aka "terminals". 
} expr_kind_t;


struct expr;

struct expr_scope {
    list_t(struct expr) items;
};

struct expr_juxta {
    list_t(struct expr) items;
};

struct expr_name {
    const uchar* name;
};

typedef struct expr {
    expr_kind_t kind;
    union {
        struct expr_scope scope;
        struct expr_juxta juxta;
        struct expr_name name;
    } inner;
} *expr_t;



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

static token_t lex_token(const uchar **p, _Bool lookahead) {
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
static token_t consume_token(const uchar **p) {
    return lex_token(p, true);
}
static token_t lookahead_token(const uchar **p) {
    return lex_token(p, false);
}

ast_t parse(const uchar *input) {
    parse_scope(&input, false);
}
static expr_t parse_expr(const uchar **p) {
    token_t tok;
    
    // If we parsed none, then error
    // If we parsed one, then this isn't a juxta but a normal expression
    // If we parsed more than one, then make this into a juxta chain
    
    expr_t first = NULL;
    expr_t juxta = NULL;
    
    while (1) {
        tok = lookahead_token(p);
        
        expr_t expr = NULL;
        if (tok.kind == '{') {
            expr = parse_scope(p, TRUE);
        }
        else if (tok.kind == '(') {
            expr = parse_paren_expr(p);
        }
        else if (tok.kind == NAME) {
            expr = parse_name(p);
        }
        else {
            token_free(tok);
            break;
        }
        
        if (!first) {
            first = expr;
        }
        else if (!juxta) {
            juxta = calloc(sizeof(struct expr), 1);
            juxta->kind = EXPR_JUXTA;
            juxta->juxta.items = list_create(expr_t);
            list_push(juxta->juxta.items, first);
        }
        else {
            list_push(juxta->juxta.items, first);
        }
        
        token_free(tok);
    }
    
    if (!first)
        return first;
    return juxta;
}
static expr_t parse_paren_expr(const uchar **p) {
    token_t tok;
    
    // Consume '('
    tok = consume_token(p);
    if (tok.kind != '(')
        error("Expected (");
    
    expr_t expr = parse_expr(p);
    
    // Consume ')'
    tok = consume_token(p);
    if (tok.kind != ')')
        error("Expected )");
    
    return expr;
}
static expr_t parse_name(const uchar **p) {
    
    // Consume NAME
    token_t tok = consume_token(p);
    if (tok.kind != NAME)
        error("Expected name");
    
    expr_t expr = calloc(sizeof(struct expr), 1);
    expr->kind = EXPR_NAME;
    expr->name.name = tok.name;
    
    return expr;
}
static expr_t parse_scope(const uchar **p, _Bool expectBraces) {
    token_t tok;
    
    expr_t expr = calloc(sizeof(struct expr), 1);
    expr->kind = EXPR_SCOPE;
    expr->scope.items = list_create(expr_t);
    
    if (expectBraces) {
        tok = consume_token(p);
        if (tok.kind != '{')
            error("Expected {");
    }
    
    while (1) {
        expr_t expr = parse_expr(p);
        
        list_push(expr->scope.items, expr);
        
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
    
    return expr;
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