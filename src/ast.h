#ifndef _AST_H_
#define _AST_H_

#include "lex.h"
#include "type.h"

// ============================================================================
// struct define
// ============================================================================
typedef enum {
    ExpOperation,
    ExpTerm,
} ExpType;

typedef enum {
    TermLiteral,
    TermArgVariable,
    TermLocalVariable,
    TermGlobalVariable,
} TermType;

// ============================================================================
// struct define
// ============================================================================
typedef struct {
    Vector *vars;
    Vector *funcs;
} Program;

typedef struct {
    StorageClass class;
    char name[256];
    Type* retType;
    Vector* args;
} FuncDecl;

typedef struct {
    Vector* vars;
    Vector* stmts;
} Scope;

typedef struct {
    Scope* scope;
} FuncBody;

typedef struct {
    FuncDecl *decl;
    FuncBody *body;
} Func;

typedef enum {
    STMT_RETURN,
    STMT_ASSIGN,
    STMT_FUNC_CALL
} StatementType;

typedef struct {
    int val;
} Integer;

typedef struct {
    TermType termType;
    Type* ty;
    void* ast;
} Term;

typedef struct {
    char fancName[256];
    Vector *args;
} AstFuncCall;

typedef enum {
    NODE_TERM,
    NODE_BINARY,
    NODE_FUNC_CALL,
} NodeType;

typedef enum {
    OPERATION_ADD,
    OPERATION_SUB,
    OPERATION_MUL,
    OPERATION_DIV,
} OperationType;

typedef struct {
    NodeType type;
    void* entry;
} AstNode;

typedef struct {
    OperationType op;
    AstNode* lhs;
    AstNode* rhs;
} AstBinary;

typedef struct {
    ExpType ty;
    void* exp;
} ExpEntry;

typedef struct {
    StatementType type;
    void* ast;
} Stmt;

typedef struct {
    AstNode* node;
} ReturnStmt;

typedef struct {
    AstFuncCall* funcCall;
} FuncCallStmt;

typedef struct {
    StorageClass class;
    int pointer_level;
    Type *ty;
    char name[256];
    AstNode *initialAssign;
} Variable;

typedef struct {
    Variable* var;
    AstNode* node;
} AssignStmt;

extern Program* program_new();
extern Stmt *stmt_new(StatementType type, void* ast);
extern Func* func_new(FuncDecl* decl, FuncBody* body);
extern Scope* scope_new(void);
extern void scope_add_stmt(Scope* scope, Stmt* stmt);
extern ReturnStmt* stmt_new_stmt_return(AstNode* node);
extern FuncCallStmt* func_call_stmt_new(AstFuncCall* funcCall);
extern AssignStmt* assign_stmt_new(Variable *var, AstNode* node);
AstNode* node_new(NodeType type, void* entry);
extern AstBinary* ast_binary_new(OperationType op, AstNode *lhs, AstNode *rhs);
extern Term* term_new(TermType type, Type* ty, void* ast);
extern Variable* variable_new(char* name, StorageClass class, Type* ty, int pointer_level);
extern AstFuncCall* fanc_call_new(char* funcName, Vector* args);

#endif // _AST_H_