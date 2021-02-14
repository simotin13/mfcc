#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include <assert.h>

#include "gpcc.h"
#include "vector.h"
#include "lex.h"
#include "ast.h"
#include "parser.h"
#include "codegen.h"

// ============================================================================
// struct define
// ============================================================================

// ============================================================================
// enum define
// ============================================================================

// ============================================================================
// prototype functions
// ============================================================================
static int read_file(char *filepath, unsigned int *len);

// ============================================================================
// global variable
// ============================================================================
static char s_code[CODE_LEN_MAX];

// ============================================================================
// main
// ============================================================================
int main(int argc, char **argv) {
    int result;
    unsigned int len = 0;
    Vector *tokens = vec_new();
    Program* program = program_new();

    if (argc < 2) {
        fprintf(stderr, "no input files.\n");
        exit(1);
    }

    result = read_file(argv[1], &len);
    if (result < 0) {
        fprintf(stderr, "read source file failed filepath:[%s]\n", argv[1]);
        exit(1);
    }

    result = tokenize(s_code, len, tokens);
    if (result < 0) {
        fprintf(stderr, "scan_code failed.\n");
        exit(1);
    }

    // ====================================================
    // DEBUG dump tokens
    // ====================================================
    {
        int i = 0;
        Token *token;
        fprintf(stdout, "************ dump tokens start ************\n");
        fprintf(stdout, "tokens size:[%d]\n", tokens->size);
        for (i = 0; i < tokens->size; i++) {
            token = (Token *)tokens->data[i];
            fprintf(stdout, "%d:, type:[%d], val:[%s]\n", i, token->type, token->val);
        }
        fprintf(stdout, "************ dump token end ************\n");
    }
#if 1
    result = parse_tokens(tokens, program);
    if (result < 0) {
        fprintf(stderr, "parse_tokens failed.\n");
        exit(1);
    }

    result = generate_binary("./build/tmp.S", program, TARGET_RISC_V);
    if (result < 0) {
        fprintf(stderr, "generate_binary failed.\n");
        exit(1);
    }
#endif

    return 0;
}

static int read_file(char *filepath, unsigned int *len)
{
    FILE *fp;
    char c;
    unsigned int pos = 0; 
   

    fp = fopen(filepath, "r");
    if (fp == NULL) {
        fprintf(stderr, "file open failed:[%s].\n", filepath);
        exit(1);
    }

    pos = 0;
    while(1) {
        c = fgetc(fp);
        if (c == EOF) {
            break;
        }
        s_code[pos] = c;
        pos++;
        if (CODE_LEN_MAX < pos) {
            fprintf(stderr, "code size too big...\n");
            fclose(fp);
            exit(1);
        }
    }

    *len = pos;
    fclose(fp);

    DPRINT(stdout, s_code);

    return 0;
}
