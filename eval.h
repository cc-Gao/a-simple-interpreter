
#pragma once


ASTnode* get_expr_from_exprlist(ASTnode* exprlist, int* number);
ASTnode* eval_expression_list(ASTnode* expr_list);



int get_array_number(ASTnode* array_node);
int get_array_number_end(ASTnode* array_node);




ASTnode* cb_eval_string(void* arg);
ASTnode* cb_eval_number(void* arg);
ASTnode* cb_eval_variable(void* arg);
ASTnode* cb_eval_array(void* arg);




ASTnode* assign_variable(ASTnode* var, int pos, ASTnode* value);
ASTnode* assign_array(ASTnode* array_var, ASTnode* array_value);
ASTnode* cb_eval_assignop(void* arg);




ASTnode* cb_eval_def_var(void* arg);
ASTnode* cb_eval_def_func(void* arg);
ASTnode* cb_eval_def_assign(void* arg);



ASTnode* cb_eval_binop(void* arg);
ASTnode* cb_eval_unaryop(void* arg);





ASTnode* cb_eval_expression_list(void* arg);
ASTnode* cb_eval_statement(void* arg);
ASTnode* cb_eval_statement_list(void* arg);
ASTnode* cb_eval_if_else(void* arg);
ASTnode* cb_eval_for_else(void* arg);
ASTnode* cb_eval_break(void* arg);



void define_param(ASTnode_func_param* function_param, ASTnode* real_param);


ASTnode* cb_eval_func_call(void* arg);
ASTnode* cb_eval_return(void* arg);
ASTnode* exec_func(ASTnode* func_def_pos);



int exam_func_point();

int exam_func_return();

void explain_string(char* string);

void translate_string(char* string_trans, char return_string[]);