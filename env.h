#pragma once





extern dian_env g_env;
extern env_table_stack g_env_stack;


void dian_init_env(dian_env* env);

void dian_init_parser(dian_st_parser* parser, dian_st_lexer* lexer);
void dian_init_lexer(char* buf, dian_st_lexer* lexer);

void push_variable_env(dian_env func_env);
void pop_variable_env(dian_env* func_env);
dian_env* get_top_env_stack();



int get_first_char(char id[]);

dian_env_var_node* get_variable_node(char id[], dian_env* extra_env, int control);

//注册前先确认该变量是否存在
int register_variable_node(char id[], dian_en_token_id T_type, int length, dian_env* extra_env);

//修改前先确认该变量是否存在
int modify_variable_node(char id[], char value[], dian_en_token_id T_type, int place, dian_env* extra_env);

ASTnode* get_variable_value(char id[], int pos, dian_env* extra_env);

dian_env_func_node* get_func_node(char id[], dian_env* extra_env);

int register_func_node(char id[], ASTnode* function, dian_env* extra_env);