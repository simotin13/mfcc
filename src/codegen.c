#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "codegen.h"

static void traverse_program(FILE* fp, Program* prgram);
static void traverse_stmt(FILE* fp, Stmt* stmt);
static void traverse_return(FILE* fp, StmtReturn* stmtReturn);

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

    fprintf(fp, ".text\n");
    traverse_program(fp, program);
    fprintf(fp, ".text\n");

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
        fprintf(fp, "%s:\n", func->decl->name);
        stmts = body->scope->stmts;
        for (j = 0; j < stmts->size; j++) {
            stmt = (Stmt *)stmts->data[i];
            traverse_stmt(fp, stmt);
        }
    }
    return;
}

static void traverse_stmt(FILE* fp, Stmt* stmt)
{
    switch (stmt->type) {
    case STMT_TYPE_RETURN:
        traverse_return(fp, (StmtReturn *)stmt);
        break;
    default:
        break;
    }
    return;
}

static void traverse_return(FILE *fp, StmtReturn *stmtReturn) {
    fprintf(fp, "ret ");
    Integer* integer;
    switch (stmtReturn->t->type) {
    case T_INTEGER:
        integer = (Integer *)(stmtReturn->val);
        fprintf(fp, "%d\n", integer->val);
    default:
        break;
    }
}