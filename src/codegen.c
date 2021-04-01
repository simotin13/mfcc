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
static void initialize_data_type_size(Vector *types);
static int get_data_type_size(Type* ty, Vector* dataTypes);

static int traverse_program(FILE* fp, Vector *dataTypes, Program* prgram);
static int traverse_stmt(FILE* fp, FuncBody *funcBody, Vector *dataTypes, Stmt* stmt);
static int traverse_return(FILE* fp, FuncBody *funcBody, ReturnStmt* stmtReturn);

static int traverse_node(FILE *fp , AstNode *node);
static int travarse_term(FILE* fp, Term* term);
static int travarse_binary(FILE* fp, AstBinary* binary);

static void write_asm_with_indent(FILE* fp, char* fmt, ...);
static void write_prologue(FILE* fp);
static void write_epilogue(FILE* fp, int frameSize);
static void write_local_vars(FILE* fp, Vector* dataTypes, Vector* vars, int *allocSize);
static void write_initial_value(FILE* fp, Vector* dataTypes, Vector* vars, Variable* var);

static int get_rbp_offset_var(Vector* vars, Variable* var, int* offset);

#define BUILD_DIR   "build"

int generate_binary(char *filename, Vector* types, Program *program, BuildTargetType target)
{
    FILE *fp;

	DPRINT(stdout, "%s:%d in...\n", __FUNCTION__, __LINE__);

    initialize_data_type_size(types);
    fp = fopen(filename, "w");
    if (fp == NULL) {
        fprintf(stderr, "file open failed:[%s].\n", filename);
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

static int traverse_program(FILE* fp, Vector *dataTypes, Program* program)
{
    int i, j;
    Func* func;
    FuncBody* funcBody;
    Vector* stmts;
    Stmt* stmt;
    int allocSize;

    for (i = 0; i < program->funcs->size; i++) {
        func = (Func *)program->funcs->data[i];
        funcBody = func->body;
        fprintf(fp, "global %s\n", func->decl->name);
        fprintf(fp, "\n");
        fprintf(fp, "%s:\n", func->decl->name);
        write_prologue(fp);
        write_local_vars(fp, dataTypes, func->body->scope->vars, &allocSize);
        stmts = funcBody->scope->stmts;
        for (j = 0; j < stmts->size; j++) {
            stmt = (Stmt *)stmts->data[i];
            traverse_stmt(fp, funcBody, dataTypes, stmt);
        }
        write_epilogue(fp, allocSize);
    }
    return 0;
}

static int traverse_stmt(FILE* fp, FuncBody *funcBody, Vector *dataTypes, Stmt* stmt)
{
    ReturnStmt* returnStmt;
    AssignStmt* assignStmt;
    switch (stmt->type) {
    case STMT_RETURN:
        returnStmt = (ReturnStmt*)stmt->ast;
        traverse_return(fp, funcBody, returnStmt);
        break;
    case STMT_ASSIGN:
        assignStmt = (AssignStmt*)stmt->ast;
        // TODO
        break;
    default:
        break;
    }
    return 0;
}

static void write_prologue(FILE* fp)
{
    write_asm_with_indent(fp, "push rbp");
    write_asm_with_indent(fp, "mov rbp, rsp");
    return;
}

static void write_epilogue(FILE* fp, int frameSize)
{
    if (0 < frameSize) {
        write_asm_with_indent(fp, "add rsp, 0x%d", frameSize);
    }
    write_asm_with_indent(fp, "mov rsp, rbp");
    write_asm_with_indent(fp, "pop rbp");
    write_asm_with_indent(fp, "ret");
    return;
}

#if 0
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
        write_asm_with_indent(fp, "sub rsp %d", alloc_stack_size);
    }

    // variable initialize
    for (i = 0; i < func->decl->args->size; i++) {
        arg = func->decl->args->data[i];
        write_initial_value(fp, dataTypes, func->body->scope->vars, arg);
    }
    return;
}
#endif

static void write_local_vars(FILE* fp, Vector* dataTypes, Vector* vars, int *allocSize)
{
    int i;
    Variable* var;

    *allocSize = 0;

    for (i = 0; i < vars->size; i++) {
        var = vars->data[i];
        *allocSize += get_data_type_size(var->ty, dataTypes);
    }

    // alloc statck
    if (0 < *allocSize) {
        write_asm_with_indent(fp, "sub rsp, %d", *allocSize);
    }

    // variable initialize
    for (i = 0; i < vars->size; i++) {
        var = vars->data[i];
        if (var->initialAssign == NULL) {
            continue;
        }
        write_initial_value(fp, dataTypes, vars, var);
    }
    return;
}

static void write_initial_value(FILE *fp, Vector *dataTypes, Vector *vars, Variable* var)
{
    int offset;
    // TODO types 
    if (strcmp(var->ty->name, "int") == 0) {
        get_rbp_offset_var(vars, var, &offset);
        // TODO generate expression
#if 0
        astInt = (Integer *)var->iVal;
        write_asm_with_indent(fp, "mov DWORD [rbp - 0x%d], 0x%d", offset, astInt->val);
#endif
    }
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

static int traverse_return(FILE *fp, FuncBody* body, ReturnStmt *stmtReturn) {
    int ret;
    // generate exp
    AstNode* node = stmtReturn->node;
    ret = traverse_node(fp, node);
    write_asm_with_indent(fp, "pop rax");
    return ret;
}

static int traverse_node(FILE *fp, AstNode *node)
{
    Term* term;
    AstBinary* bin;
    switch (node->type) {
    case NODE_TERM:
        term = node->entry;
        travarse_term(fp, term);
        break;
    case NODE_BINARY:
        bin = node->entry;
        travarse_binary(fp, bin);
        break;
    }
    return 0;
}

static int travarse_term(FILE* fp, Term* term)
{
    Integer* integer;
    switch (term->termType) {
    case TermLiteral:
        integer = (Integer *)term->ast;
        write_asm_with_indent(fp ,"mov rax, 0x%02X", integer->val);
        write_asm_with_indent(fp, "push rax");
        break;
    case TermVariable:
        // TODO
        break;
    default:
        return -1;
    }

    return 0;
}

static int travarse_binary(FILE *fp, AstBinary* binary)
{
    traverse_node(fp, binary->lhs);
    traverse_node(fp, binary->rhs);

    switch (binary->op)
    {
    case OPERATION_ADD:
        // TODO calc reg
        write_asm_with_indent(fp, "pop rbx");
        write_asm_with_indent(fp, "pop rax");
        write_asm_with_indent(fp, "add rax, rbx");
        write_asm_with_indent(fp, "push rax");
        break;
    case OPERATION_SUB:
        break;
    case OPERATION_MUL:
        write_asm_with_indent(fp, "pop rbx");
        write_asm_with_indent(fp, "pop rax");
        write_asm_with_indent(fp, "imul rax, rbx");
        write_asm_with_indent(fp, "push rax");
        break;
    case OPERATION_DIV:
        break;

    default:
        break;
    }
    return 0;
}

static int get_rbp_offset_var(Vector* vars, Variable* var, int *offset)
{
    int i;
    Variable* tmp;
    *offset = 0;
    for (i = 0; i < vars->size; i++) {
        tmp = vars->data[i];
        *offset -= tmp->ty->size;
        if (strcmp(tmp->name, var->name) == 0) {
            return i;
        }
    }

    return -1;
}
