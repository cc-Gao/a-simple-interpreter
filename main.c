#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "lexer.h"
#include "lib.h"
#include "parser.h"
#include "eval.h"
#include "env.h"



dian_st_lexer g_lexer;
dian_st_parser g_parser;
dian_st_token* gp_peek_token = NULL;



int g_def_fun_limit = 0;

void dian_init_parser(dian_st_parser* parser,dian_st_lexer* lexer)
{
	parser->lexer = lexer;
	parser->arg = parser;
	parser->p_current_token = NULL;
	parser->parser_program_AST = parser_program_AST;
	parser->cb_eat = parser_callback_eat;
	parser->cb_assign_statement = parser_callback_assign_statement;
	parser->cb_break_statement = parser_callback_break_statement;
	parser->cb_return_statement = parser_callback_return_statement;
	parser->cb_func_def_statement = parser_callback_function_delaration_statement;
	parser->cb_for_statement = parser_callback_for_statement;
	parser->cb_if_statement = parser_callback_if_statement;
	parser->cb_compound_statement = parser_callback_compound_statement;
	parser->cb_statement = parser_callback_statement;
	parser->cb_simple_statement = parser_callback_simple_statement;
	parser->cb_delaration_statement = parser_callback_delaration_statement;
	parser->cb_statement_list = parser_callback_statement_list;
	parser->cb_empty_statement = parser_callback_empty_statement;
	parser->cb_expression = parser_callback_expression;
	parser->cb_term0 = parser_callback_term0;
	parser->cb_term1 = parser_callback_term1;
	parser->cb_term2 = parser_callback_term2;
	parser->cb_term3 = parser_callback_term3;
	parser->cb_term4 = parser_callback_term4;
	parser->cb_term5 = parser_callback_term5;
	parser->cb_term6 = parser_callback_term6;
	return;
}

void dian_init_lexer(char *buf, dian_st_lexer* lexer)
{
	strcpy(lexer->text, buf);
	lexer->pos = 0;
	lexer->arg = lexer;

	lexer->cb_comment = lexer_callback_comment;
	lexer->cb_skip_next = lexer_callback_skip_next;
	lexer->cb_get_peek_token = lexer_callback_peek_next_token;
	lexer->cb_get_next_token = lexer_callback_get_next_token;
	lexer->cb_identifier = lexer_callback_identifier;
	lexer->cb_number = lexer_callback_number;
	lexer->cb_operator = lexer_callback_operator;
	lexer->cb_string = lexer_callback_string;
	return;
}

void dian_init_env(dian_env* env)
{
	env->func_table = malloc(sizeof(dian_func_table));
	memset(env->func_table, 0, sizeof(dian_func_table));
	env->var_table = malloc(sizeof(dian_var_table));
	memset(env->var_table, 0, sizeof(dian_var_table));

}

void dian_init_env_stack(env_table_stack* env_stack_point)
{
	env_stack_point->base = malloc(sizeof(dian_env) * MAX_ENV_STACK_SIZE);
	env_stack_point->top = env_stack_point->base;
	env_stack_point->size = MAX_ENV_STACK_SIZE;
}

int main(int argc, char* argv[])
{
	

	

	if (argc < 2)
	{
		printf("usage: dian [filename] [< input.txt] [> output.txt]\n");
		exit(1);
	}



	FILE* fp = fopen(argv[1], "rb");
	if (fp == NULL)
	{
		printf("file open error\n");
		exit(-1);
	}
	static char code[1000];
	fread(code, 1000, 1, fp);



	dian_init_lexer(code,&g_lexer);
	dian_init_parser(&g_parser,&g_lexer);
	dian_init_env(&g_env);
	dian_init_env_stack(&g_env_stack);
	g_parser.p_current_token = g_lexer.cb_get_next_token(&g_lexer);
	ASTnode* node = g_parser.parser_program_AST(&g_parser);
	node->eval(node);



	return 0;


}
