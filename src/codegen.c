#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

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
static int traverse_stmt(FILE* fp, Vector *globalVars, Func *func, Vector *dataTypes, Stmt* stmt);
static int traverse_return(FILE* fp, Vector* globalVars, Func *func, ReturnStmt* stmtReturn);
static int traverse_node(FILE *fp, Vector* globalVars, Func *func, AstNode *node);
static int travarse_term(FILE* fp, Vector *globalVars, Func *func, Term* term);
static int travarse_binary(FILE* fp, Vector* globalVars, Func* func, AstBinary* binary);
static int traverse_assign(FILE* fp, Vector* globalVars, Func* body, AssignStmt* assignStmt);
static int traverse_func_call(FILE* fp, Vector* globalVars, Func* body, AstFuncCall* stmtFuncCall);
static void write_asm_with_indent(FILE* fp, char* fmt, ...);
static void write_prologue(FILE* fp);
static void write_epilogue(FILE* fp, int frameSize);
static void write_local_vars(FILE* fp, Vector* dataTypes, Vector* vars, int *allocSize);
static void write_initial_value(FILE* fp, Vector* dataTypes, Vector* vars, Variable* var);

static int get_local_var_rbp_offset(Vector* vars, Variable* var, int* offset);
static int get_arg_var_rbp_offset(Vector* vars, Variable* var, int* offset);

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
    Vector* stmts;
    Stmt* stmt;
    int allocSize;

    // write function label
    for (i = 0; i < program->funcs->size; i++) {
        func = (Func*)program->funcs->data[i];
        fprintf(fp, "global %s\n", func->decl->name);
    }
    fprintf(fp, "\n");

    for (i = 0; i < program->funcs->size; i++) {
        func = (Func *)program->funcs->data[i];
        fprintf(fp, "%s:\n", func->decl->name);
        write_prologue(fp);
        write_local_vars(fp, dataTypes, func->body->scope->vars, &allocSize);
        stmts = func->body->scope->stmts;
        for (j = 0; j < stmts->size; j++) {
            stmt = (Stmt *)stmts->data[j];
            traverse_stmt(fp, program->vars, func, dataTypes, stmt);
        }
        write_epilogue(fp, allocSize);
        fprintf(fp, "\n");
    }
    return 0;
}

static int traverse_stmt(FILE* fp, Vector *globalVars, Func *func, Vector *dataTypes, Stmt* stmt)
{
    ReturnStmt* returnStmt;
    AssignStmt* assignStmt;
    FuncCallStmt* funcCallStmt;
    switch (stmt->type) {
    case STMT_RETURN:
        returnStmt = (ReturnStmt *)stmt->ast;
        traverse_return(fp, globalVars, func, returnStmt);
        break;
    case STMT_ASSIGN:
        assignStmt = (AssignStmt *)stmt->ast;
        traverse_assign(fp, globalVars, func, assignStmt);
        // TODO
        break;
    case STMT_FUNC_CALL:
        funcCallStmt = (FuncCallStmt *)stmt->ast;
        traverse_func_call(fp, globalVars, func, funcCallStmt->funcCall);
        break;
    default:
        // not implemented
        assert(0);
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
        get_local_var_rbp_offset(vars, var, &offset);
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

static int traverse_func_call(FILE* fp, Vector* globalVars, Func* func, AstFuncCall* funcCall) {
    int i;
    Vector *args = funcCall->args;
    AstNode* node;
    Term* term;
    Integer* integer;
    for (i = args->size - 1; 0 <= i; i--) {
        node = args->data[i];
        switch (node->type) {
        case NODE_TERM:
            term = node->entry;
            if (term->termType == TermLiteral) {
                integer = (Integer*)term->ast;
                write_asm_with_indent(fp, "push %d", integer->val);
            }
            break;
        default:
            assert(0);
            break;
        }
    }
    write_asm_with_indent(fp, "call %s", funcCall->fancName);
    return 0;
}
static int traverse_return(FILE *fp, Vector* globalVars, Func* func, ReturnStmt *stmtReturn) {
    int ret = 0;
    // generate exp
    AstNode* node = stmtReturn->node;
    traverse_node(fp, globalVars, func, node);
    write_asm_with_indent(fp, "pop rax");
    return ret;
}
static int traverse_assign(FILE* fp, Vector *globalVars, Func* func, AssignStmt* assignStmt) {
    AstNode* node;
    Variable* var;
    int offset;
    int ret;
    node = (AstNode *)assignStmt->node;
    var = assignStmt->var;
    if (strcmp(var->ty->name, "int") == 0) {
        ret = traverse_node(fp, globalVars, func, node);

        // TODO Type
        // local var is negative offset
        ret = get_local_var_rbp_offset(func->body->scope->vars, var, &offset);
        if (ret != 0) {
            return -1;
        }
        write_asm_with_indent(fp, "mov [rbp - 0x%X], rax", offset);
        //TOOD
       // write_asm_with_indent(fp, "mov DWORD [rbp - 0x%d], 0x%d", offset, astInt->val);
    }

    return 0;
}

static int traverse_node(FILE *fp, Vector* glovalVars, Func* func, AstNode *node)
{
    Term* term;
    AstBinary* bin;
    switch (node->type) {
    case NODE_TERM:
        term = node->entry;
        travarse_term(fp, glovalVars, func, term);
        break;
    case NODE_BINARY:
        bin = node->entry;
        travarse_binary(fp, glovalVars, func, bin);
        break;
    case NODE_FUNC_CALL:
        traverse_func_call(fp, glovalVars, func, (AstFuncCall*)node->entry);
        break;
    }
    return 0;
}

static int travarse_term(FILE* fp, Vector *glovalVars, Func *func, Term* term)
{
    int ret;
    int offset;
    Integer* integer;
    Variable* var;
    
    // TODO data type check
    switch (term->termType) {
    case TermLiteral:
        integer = (Integer *)term->ast;
        write_asm_with_indent(fp ,"mov rax, 0x%02X", integer->val);
        write_asm_with_indent(fp, "push rax");
        break;
    case TermArgVariable:
        var = (Variable *)term->ast;
        ret = get_arg_var_rbp_offset(func->decl->args, var, &offset);
        if (ret < 0) {
            assert(0);
            return -1;
        }
        write_asm_with_indent(fp, "mov rax, [rbp + 0x%X]", offset);
        write_asm_with_indent(fp, "push rax");
        break;
    case TermLocalVariable:
        var = (Variable*)term->ast;
        ret = get_local_var_rbp_offset(func->body->scope->vars, var, &offset);
        if (ret < 0) {
            assert(0);
            return -1;
        }
        write_asm_with_indent(fp, "mov rax, [rbp - 0x%X]", offset);
        write_asm_with_indent(fp, "push rax");
        break;
    case TermGlobalVariable:
        assert(0);
        break;
    default:
        return -1;
    }

    return 0;
}

static int travarse_binary(FILE *fp, Vector *globalVars, Func *func, AstBinary* binary)
{
    traverse_node(fp, globalVars, func, binary->lhs);
    traverse_node(fp, globalVars, func, binary->rhs);

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

static int get_local_var_rbp_offset(Vector* vars, Variable* var, int *offset)
{
    int i;
    Variable* tmp;
    for (i = 0; i < vars->size; i++) {
        tmp = vars->data[i];
        *offset = tmp->ty->size;
        if (strcmp(tmp->name, var->name) == 0) {
            return i;
        }
    }

    return -1;
}

static int get_arg_var_rbp_offset(Vector* args, Variable* var, int* offset)
{
    int i;
    Variable* tmp;
    *offset = 8;
    for (i = args->size - 1;  0 <= i; i--) {
        tmp = args->data[i];
        *offset += tmp->ty->size * 2;
        if (strcmp(tmp->name, var->name) == 0) {
            return i;
        }
    }

    return -1;
}
