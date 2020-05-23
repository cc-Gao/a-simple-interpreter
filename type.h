#pragma once

#define MAX_TEXT_LINE 81
#define MAX_ID_LENGTH 32
#define MAX_SIZE_TOKEN_VALUE 64
#define MAX_SIZE_LEXER_BUF 1000
#define NUM_KEYWORD 8
#define NUM_OPERATOR 13

#define MAX_ENV_STACK_SIZE 300




/****************************************
*										*
*			token类型的设定				*
*										*
****************************************/
//token的类型设定
typedef enum identifier {
	NON,
	END,
	SEMI,			//分号
	COMMA,			//逗号
	COLON,			//冒号
	//keyword
	K_IF,
	K_ELIF,
	K_ELSE,
	K_FOR,
	K_BREAK,
	K_FUNC,
	K_RETURN,
	K_ASSIGN,
	//scope
	LS_BRACE,
	RS_BRACE,
	LM_BRACE,
	RM_BRACE,
	LB_BRACE,
	RB_BRACE,
	//typedef
	T_FUNC,
	T_INT,
	T_FLOAT,
	T_STRING,
	//variable type
	V_INT,
	V_FLOAT,
	V_STRING,
	//constant type
	C_INT,
	C_FLOAT,
	C_STRING,
	//array type
	T_INT_ARRAY,
	T_FLOAT_ARRAY,
	T_STRING_ARRAY,

	V_INT_ARRAY,
	V_FLOAT_ARRAY,
	V_STRING_ARRAY,

	C_INT_ARRAY,
	C_FLOAT_ARRAY,
	C_STRING_ARRAY,

	T_ASSIGN,
	ID,
	//operator
	O_PLUS,
	O_MINUS,
	O_MUL,
	O_F_DIV,
	O_I_DIV,
	O_REM,			//取余数
	O_GREAT,
	O_LESS,
	O_GREAT_EQUAL,
	O_LESS_EQUAL,
	O_EQUAL,
	O_NOT_EQUAL,
	O_POWER,		//幂
	O_BIT_AND,
	O_BIT_OR,
	O_BIT_XNOR,
	O_BIT_NOR,
	O_BIT_LEFT,
	O_BIT_RIGHT,
	O_LOGIC_AND,
	O_LOGIC_OR,
	O_LOGIC_NOR,
	O_ASSIGN,
	O_PLUS_ASSIGN,
	O_MINUS_ASSIGN,
	O_MUL_ASSIGN,
	O_F_DIV_ASSIGN,
	O_I_DIV_ASSIGN,
	O_REM_ASSIGN,
}dian_en_token_id;

//token流
typedef struct token
{
	dian_en_token_id type;
	char value[MAX_SIZE_TOKEN_VALUE];
}dian_st_token;

/****************************************
*										*
*			env类型的设定					*
*										*
****************************************/
typedef struct variable_pointer
{
	dian_en_token_id T_type;
	void* pointer;
}memory_block;


typedef struct function_node
{
	char id[MAX_ID_LENGTH];
	struct ASTnode* fun_def_pos;
	struct function_node* next_func;
}dian_env_func_node;


typedef struct function_env_table
{
	dian_env_func_node* Dict[26];
}dian_func_table;


typedef struct variable_node
{
	char id[MAX_ID_LENGTH];
	dian_en_token_id T_type;
	int length;
	memory_block* value;
	struct variable_node* next_var;
}dian_env_var_node;


typedef struct variable_env_table
{
	dian_env_var_node* Dict[26];
}dian_var_table;


typedef struct program_env
{
	struct function_env_table* func_table;
	struct variable_env_table* var_table;
}dian_env;


typedef struct env_table_stack
{
	dian_env* base;
	dian_env* top;
	int size;
}env_table_stack;




/****************************************
*										*
*			节点的部分定义					*
*										*
****************************************/

//节点node的类型设定
typedef enum AST_node_type
{
	binop,
	unaryop,
	assignop,
	variable,
	number,
	string,
	array_node,
	array_node_slice,
	array_addr,
	func_addr,
	func_call,
	def_assign,
	def_var,
	def_func,
	node_if,
	node_for,
	expression_list,
	statement,
	statement_list,
	break_node,
	return_node,
	empty,
}AST_en_node_type;

//节点中的联合指针
typedef union AST_node_pointer {
	struct AST_node_binop* p_binop;
	struct AST_node_unaryop* p_unaryop;
	struct AST_Node_assign* p_assignop;
	struct AST_node_array* p_array;
	struct AST_node_variable* p_variable;
	struct AST_node_number* p_number;
	struct AST_node_string* p_string;
	struct AST_node_addr* p_addr;
	struct AST_node_call_funtion* p_func_call;
	struct AST_node_def_function* p_def_func;
	struct AST_node_def_variable* p_def_var;
	struct AST_node_def_assign* p_def_assign;
	struct AST_node_if_statement* p_if;
	struct AST_node_for_statement* p_for;
	struct AST_node_expression_list* p_expression_list;
	struct AST_node_statement* p_statement;
	struct AST_node_statement_list* p_statement_list;
	struct AST_node_return_node* p_return;
	struct AST_node_break_node* p_break;
}dian_un_node_pointer;

//AST的node结构
typedef struct ASTnode
{
	AST_en_node_type type;
	union AST_node_pointer property;				//节点实际存储的信息
	struct ASTnode* (*eval)(void* arg);
}ASTnode;


typedef struct AST_node_addr
{
	dian_env_var_node* array_addr;
	dian_env_func_node* func_addr;
}ASTnode_addr;

/****************************************
*										*
*			运算节点类型对应结构			*
*										*
****************************************/
//双目运算符节点
typedef struct AST_node_binop
{
	ASTnode* left;
	ASTnode* right;
	dian_en_token_id op;
}ASTnode_binop;

//单目运算符节点
typedef struct AST_node_unaryop
{
	ASTnode* child;
	dian_en_token_id op;
}ASTnode_unaryop;

//赋值运算符节点
typedef struct AST_Node_assign
{
	ASTnode* left;
	ASTnode* right;
}ASTnode_assign;

//数值常量节点
typedef struct AST_node_number
{
	dian_en_token_id type;
	char value[32];
}ASTnode_number;

//字符串常量节点
typedef struct AST_node_string
{
	char value[64];
}ASTnode_string;

//变量节点
typedef struct AST_node_variable
{
	char variable_id[MAX_ID_LENGTH];
}ASTnode_variable;

//数组节点
typedef struct AST_node_array
{
	char array_id[MAX_ID_LENGTH];
	ASTnode* expression_start;
	ASTnode* expression_end;
}ASTnode_array;

//assign绑定语句节点
typedef struct AST_node_def_assign
{
	char variable_id[MAX_ID_LENGTH];
	ASTnode* value_expression;
}ASTnode_def_assign;

//变量声明节点
typedef struct AST_node_def_variable
{
	dian_en_token_id type;
	ASTnode* expression;
	ASTnode* assign;
}ASTnode_def_var;


//函数参数列表
typedef struct AST_node_function_parameter
{
	dian_en_token_id type;
	char id[MAX_ID_LENGTH];
	void* extra_message;
	struct AST_node_function_parameter* next_param;
}ASTnode_func_param;


//函数返回列表
typedef struct AST_node_function_typelist
{
	dian_en_token_id type;
	struct AST_node_function_typelist* next_type;
}ASTnode_func_typelist;

//函数指针定义
typedef struct AST_node_function_point
{
	ASTnode_func_typelist* param_list;
	ASTnode_func_typelist* return_list;
}ASTnode_func_point;

//函数声明节点
typedef struct AST_node_def_function
{
	char name[MAX_ID_LENGTH];
	ASTnode_func_param* param_list;
	ASTnode_func_typelist* ret_list;
	ASTnode* exec_statement;
}ASTnode_def_func;


//表达式列表
typedef struct AST_node_expression_list
{
	ASTnode* expression;
	struct AST_node_expression_list* next_expression;
}ASTnode_expression_list;





/****************************************
*										*
*			语句节点类型对应结构			*
*										*
****************************************/

//语句类型
typedef enum AST_node_type_statement
{
	if_statement,
	for_statement,
	def_statement,
	func_def_statement,
	simple_statement,
	empty_statement,
	return_statement,
	break_statement,
	assign_statement,
	compound_statement,
}ASTnode_statement_type;

//语句节点
typedef struct AST_node_statement
{
	ASTnode_statement_type type;
	ASTnode* child;
}ASTnode_statement;

//语句组节点
typedef struct AST_node_statement_list
{
	ASTnode* statement;
	struct AST_node_statement_list* next_statement;
}ASTnode_statement_list;

//if_else节点
typedef struct AST_node_if_statement
{
	dian_en_token_id type;
	ASTnode* condition;
	ASTnode* exec_statement;
	struct AST_node_if_statement* other;
}ASTnode_if_statement;

//for节点
typedef struct AST_node_for_statement
{
	ASTnode* statement_A;
	ASTnode* statement_B;
	ASTnode* statement_C;
	ASTnode* exec_statement;
	ASTnode* else_statement;
}ASTnode_for_statement;

//函数调用节点
typedef struct AST_node_call_funtion
{
	char id[MAX_ID_LENGTH];
	ASTnode* param;
}ASTnode_func_call;


//break节点
typedef struct AST_node_break_node
{
	ASTnode* verification;
}ASTnode_break;

//return节点
typedef struct AST_node_return_node
{
	ASTnode* return_expr;
}ASTnode_return;





/****************************************
*										*
*		词法分析器与语法分析器结构			*
*										*
****************************************/

typedef struct parser
{
	void* arg;
	dian_st_token* p_current_token;
	struct lexer* lexer;

	ASTnode* (*parser_program_AST)(void* arg);
	//eat函数,检查语法并更新当前token
	int (*cb_eat)(void* arg, dian_en_token_id type);
	//expression相关回调
	ASTnode* (*cb_expression)(void* arg);
	ASTnode* (*cb_term0)(void* arg);
	ASTnode* (*cb_term1)(void* arg);
	ASTnode* (*cb_term2)(void* arg);
	ASTnode* (*cb_term3)(void* arg);
	ASTnode* (*cb_term4)(void* arg);
	ASTnode* (*cb_term5)(void* arg);
	ASTnode* (*cb_term6)(void* arg);
	//statement相关回调
	ASTnode* (*cb_func_def_statement)(void* arg);
	ASTnode* (*cb_for_statement)(void* arg);
	ASTnode* (*cb_if_statement)(void* arg);
	ASTnode* (*cb_compound_statement)(void* arg);
	ASTnode* (*cb_simple_statement)(void* arg);
	ASTnode* (*cb_delaration_statement)(void* arg);
	ASTnode* (*cb_statement)(void* arg);
	ASTnode* (*cb_statement_list)(void* arg);
	ASTnode* (*cb_empty_statement)(void* arg);
	ASTnode* (*cb_return_statement)(void* arg);
	ASTnode* (*cb_assign_statement)(void* arg);
	ASTnode* (*cb_break_statement)(void* arg);
}dian_st_parser;


typedef struct lexer
{
	char text[MAX_SIZE_LEXER_BUF];
	int pos;
	void* arg;
	void (*cb_get_peek_token)(void* self);
	dian_st_token* (*cb_get_next_token)(void* self);
	void (*cb_skip_next)(void* self);
	dian_st_token* (*cb_comment)(void* self);
	dian_st_token* (*cb_identifier)(void* self);
	dian_st_token* (*cb_number)(void* self);
	dian_st_token* (*cb_operator)(void* self);
	dian_st_token* (*cb_string)(void* self);
}dian_st_lexer;