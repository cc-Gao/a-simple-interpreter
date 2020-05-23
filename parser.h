#pragma once

extern dian_st_parser g_parser;
extern int g_def_fun_limit;


int parser_callback_eat(void* arg, dian_en_token_id type);
ASTnode* parser_program_AST(void* arg);

ASTnode* parser_callback_expression(void* arg);
ASTnode* parser_callback_term0(void* arg);
ASTnode* parser_callback_term1(void* arg);
ASTnode* parser_callback_term2(void* arg);
ASTnode* parser_callback_term3(void* arg);
ASTnode* parser_callback_term4(void* arg);
ASTnode* parser_callback_term5(void* arg);
ASTnode* parser_callback_term6(void* arg);

ASTnode* create_expression_list(ASTnode* expression_list, ASTnode* expression);
ASTnode* create_statement_list(ASTnode* statement);
ASTnode_func_point* parser_function_pointer(dian_st_parser* parser);
ASTnode* parser_callback_statement_list(void* arg);

ASTnode* parser_callback_statement(void* arg);

ASTnode* parser_callback_empty_statement(void* arg);

ASTnode* parser_callback_simple_statement(void* arg);

ASTnode* parser_callback_delaration_statement(void* arg);

ASTnode* parser_callback_function_delaration_statement(void* arg);

ASTnode* parser_callback_compound_statement(void* arg);

ASTnode* parser_callback_if_statement(void* arg);

ASTnode* parser_callback_for_statement(void* arg);

ASTnode* parser_callback_break_statement(void* arg);

ASTnode* parser_callback_return_statement(void* arg);

ASTnode* parser_callback_assign_statement(void* arg);







