#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdarg.h>
#include "codegen.h"

#if defined(_WIN64) || defined(_WIN32)
#include <direct.h>
#endif

static void traverse_program(FILE* fp, Program* prgram);
static void traverse_stmt(FILE* fp, Stmt* stmt);
static void traverse_return(FILE* fp, StmtReturn* stmtReturn);

static void write_asm_with_indent(FILE* fp, char* fmt, ...);
static void write_prologue(FILE* fp);
static void write_epilogue(FILE* fp);

#define BUILD_DIR   "build"

int generate_binary(char *filename, Program *program, BuildTargetType target)
{
    FILE *fp;
    struct stat st;
    int result;
    char filepath[1024];

	DPRINT(stdout, "%s:%d in...\n", __FUNCTION__, __LINE__);

    result = stat(BUILD_DIR, &st);
    if (result != 0) {
#if defined(_WIN64) || defined(_WIN32)
        // for windows
        mkdir(BUILD_DIR);
#else
#endif
    }
    sprintf(filepath, "%s/%s", BUILD_DIR, filename);
    fp = fopen(filepath, "w");
    if (fp == NULL) {
        fprintf(stderr, "file open failed:[%s].\n", filepath);
        exit(1);
    }

    fprintf(fp, "section .text\n");
    traverse_program(fp, program);

    fclose(fp);

	DPRINT(stdout, "%s:%d out...\n", __FUNCTION__, __LINE__);
    return 0;
}

static void traverse_program(FILE* fp, Program* program)
{
    int i, j;
    Func* func;
    FuncBody* body;
    Vector* stmts;
    Stmt* stmt;

    for (i = 0; i < program->funcs->size; i++) {
        func = (Func *)program->funcs->data[i];
        body = func->body;
        fprintf(fp, "global %s\n", func->decl->name);
        fprintf(fp, "\n");
        fprintf(fp, "%s:\n", func->decl->name);
        write_prologue(fp);
        stmts = body->scope->stmts;
        for (j = 0; j < stmts->size; j++) {
            stmt = (Stmt *)stmts->data[i];
            traverse_stmt(fp, stmt);
        }
        write_epilogue(fp);
    }
    return;
}

static void traverse_stmt(FILE* fp, Stmt* stmt)
{
    switch (stmt->type) {
    case STMT_TYPE_RETURN:
        traverse_return(fp, (StmtReturn *)stmt->ast);
        break;
    default:
        break;
    }
    return;
}

static void write_prologue(FILE* fp)
{
    write_asm_with_indent(fp, "push rbp");
    write_asm_with_indent(fp, "mov rbp, rsp");
    return;
}

static void write_epilogue(FILE* fp)
{
    write_asm_with_indent(fp, "pop rbp");
    write_asm_with_indent(fp, "ret");
    return;
}

static void write_asm_with_indent(FILE* fp, char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(fp, "\t");
    vfprintf(fp, fmt, args);
    fprintf(fp, "\n");
    va_end(args);
    return;
}

static void traverse_return(FILE *fp, StmtReturn *stmtReturn) {
    Integer* integer;
    switch (stmtReturn->t->type) {
    case T_INTEGER:
        integer = (Integer *)(stmtReturn->val);
        write_asm_with_indent(fp, "mov eax, %d", integer->val);
    default:
        break;
    }
}
