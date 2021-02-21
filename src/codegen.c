#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <string.h>

#include "codegen.h"
#include "type.h"

#if defined(_WIN64) || defined(_WIN32)
#include <direct.h>
#endif

// ============================================================================
// static variables
// ============================================================================
static char s_code[CODE_LEN_MAX];
static const Type c_types[] =
{
    {   0,              0,    "void"        },
    {   1,   sizeof(char),      "char"      },
    {   2,   sizeof(short),     "short"     },
    {   3,   sizeof(int),       "int"       },
    {   4,   sizeof(long),      "long"      },
    {   5,   sizeof(float),     "float"     },
    {   6,   sizeof(double),    "double"    },
};

static void initialize_data_type_size(Vector *types);
static int get_data_type_size(Type* ty, Vector* dataTypes);

static void traverse_program(FILE* fp, Vector *dataTypes, Program* prgram);
static void traverse_stmt(FILE* fp, Vector *dataTypes, Stmt* stmt);
static void traverse_return(FILE* fp, StmtReturn* stmtReturn);

static void write_asm_with_indent(FILE* fp, char* fmt, ...);
static void write_prologue(FILE* fp);
static void write_epilogue(FILE* fp);
static void write_function_args(FILE* fp, Vector *dataTypes, Func* func);

#define BUILD_DIR   "build"

int generate_binary(char *filename, Vector* types, Program *program, BuildTargetType target)
{
    FILE *fp;
    struct stat st;
    int result;
    char filepath[1024];

	DPRINT(stdout, "%s:%d in...\n", __FUNCTION__, __LINE__);

    initialize_data_type_size(types);

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
    traverse_program(fp, types, program);

    fclose(fp);

	DPRINT(stdout, "%s:%d out...\n", __FUNCTION__, __LINE__);
    return 0;
}

static void initialize_data_type_size(Vector* types)
{
    int i, j;
    Type* ty;
    Type* c_type;
    for (i = 0; i < types->size; i++) {
        ty = (Type *)types->data[i];
        for (j = 0; j < C_TYPES_LEN; j++) {
            if (strcmp(ty->name, c_types[j].name) == 0) {
                ty->size = c_types[j].size;
            }
        }
    }
    return;
}

static void traverse_program(FILE* fp, Vector *dataTypes, Program* program)
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
        write_function_args(fp, dataTypes, func);
        stmts = body->scope->stmts;
        for (j = 0; j < stmts->size; j++) {
            stmt = (Stmt *)stmts->data[i];
            traverse_stmt(fp, dataTypes, stmt);
        }
        write_epilogue(fp);
    }
    return;
}

static void traverse_stmt(FILE* fp, Vector *dataTypes, Stmt* stmt)
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

static void write_function_args(FILE* fp, Vector *dataTypes, Func* func)
{
    int i;
    int alloc_stack_size = 0;
    Variable* arg;
    for (i = 0; i < func->decl->args->size; i++) {
        arg = func->decl->args->data[i];
        alloc_stack_size  += get_data_type_size(arg->ty, dataTypes);
    }

    // alloc statck
    if (0 < alloc_stack_size) {
        write_asm_with_indent(fp, "sub esp %d", alloc_stack_size);
    }
    return;
}

static int get_data_type_size(Type *ty, Vector* dataTypes)
{
    int i;
    Type* tmp;
    for (i = 0; i < dataTypes->size; i++) {
        tmp = dataTypes->data[i];
        if (strcmp(ty->name, tmp->name) == 0) {
            return tmp->size;
        }
    }

    return 0;
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
