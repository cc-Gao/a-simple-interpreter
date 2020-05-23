#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "env.h"
#include "lexer.h"
#include "lib.h"
#include "parser.h"
#include "eval.h"



/*
typedef union variable_value
{
	int int_number;
	float float_number;
	int* int_array;
	float* float_array;
	char* string;
	char** string_array;
}dian_env_var_value;

typedef struct variable_node
{
	char id[MAX_ID_LENGTH];
	dian_en_token_id type;
	dian_env_var_value value;
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
*/


dian_env g_env;
env_table_stack g_env_stack;

void push_variable_env(dian_env func_env)
{
	if (g_env_stack.top - g_env_stack.base == g_env_stack.size)
	{
		printf("stack overflow\n");
		exit(-1);
	}
	g_env_stack.top++;
	*g_env_stack.top = func_env;
}

void pop_variable_env(dian_env* func_env)
{
	if (g_env_stack.top == g_env_stack.base)
	{
		printf("stack empty\n");
		exit(-1);
	}
	*func_env = *g_env_stack.top;
	g_env_stack.top--;
}

dian_env* get_top_env_stack()
{
	if (g_env_stack.top == g_env_stack.base)
	{
		return NULL;
	}
	return g_env_stack.top;
}


int get_first_char(char id[])
{
	char c = id[0];
	if (c == '$')
	{
		return 0;
	}
	else
	{
		return c - 'a' + 1;
	}
}

//control不为1时在整个变量表中寻找变量
dian_env_var_node* get_variable_node(char id[], dian_env* extra_env,int control)
{	
	dian_env* global_env = &g_env;
	dian_env* local_env = NULL;

	if (extra_env != NULL)
	{
		local_env = extra_env;
	}
	int number = get_first_char(id);
	dian_env_var_node* tmp = NULL;
	if (local_env != NULL)
	{
		tmp = local_env->var_table->Dict[number];

		while (tmp != NULL)
		{
			int ret = strcmp(tmp->id, id);
			if (ret == 0)
			{
				return tmp;
			}
			tmp = tmp->next_var;
		}
	}
	if (extra_env == NULL || control == 0)
	{
		tmp = global_env->var_table->Dict[number];

		while (tmp != NULL)
		{
			int ret = strcmp(tmp->id, id);
			if (ret == 0)
			{
				return tmp;
			}
			tmp = tmp->next_var;
		}
	}
	return NULL;
}


//注册前先确认该变量是否存在(若存在局部环境表，则使用局部环境表)
int register_variable_node(char id[], dian_en_token_id T_type, int length, dian_env* extra_env)
{
	int number = get_first_char(id);
	dian_env* env = &g_env;
	if (extra_env != NULL)
	{
		env = extra_env;
	}
	dian_env_var_node* tmp = env->var_table->Dict[number];
	if (tmp == NULL)
	{
		env->var_table->Dict[number] = malloc(sizeof(dian_env_var_node));
		tmp = env->var_table->Dict[number];
	}
	else
	{
		while (tmp->next_var != NULL)
		{
			tmp = tmp->next_var;
		}
		tmp->next_var = malloc(sizeof(dian_env_var_node));
		tmp = tmp->next_var;
	}
	strcpy(tmp->id, id);
	if (T_type == T_INT_ARRAY || T_type == T_FLOAT_ARRAY || T_type == T_STRING_ARRAY)
	{
		tmp->length = length;
		tmp->T_type = T_type;
		tmp->value = malloc(sizeof(memory_block) * length);
		memory_block* p = tmp->value;
		for (int i = 0; i < length; i++)
		{

			if (T_type == T_INT_ARRAY)
			{
				(p + i)->T_type = T_INT;
				(p + i)->pointer = malloc(sizeof(int));
				*(int*)((p + i)->pointer) = 0;
			}
			else if (T_type == T_FLOAT_ARRAY)
			{
				(p + i)->T_type = T_FLOAT;
				(p + i)->pointer = malloc(sizeof(float));
				*(float*)((p + i)->pointer) = (float)0;
			}
			else
			{
				(p + i)->T_type = T_STRING;
				(p + i)->pointer = malloc(sizeof(char) * MAX_SIZE_TOKEN_VALUE);
				memset((p + i)->pointer, 0, sizeof(char) * MAX_SIZE_TOKEN_VALUE);
				strcpy((char*)(p + i)->pointer, "");
			}
		}
	}
	else if (T_type == T_INT || T_type == T_FLOAT || T_type == T_STRING)
	{
		tmp->length = 1;
		tmp->T_type = T_type;
		tmp->value = malloc(sizeof(memory_block));
		memory_block* p = tmp->value;
		if (T_type == T_INT)
		{
			p->T_type = T_INT;
			p->pointer = malloc(sizeof(int));
			*(int*)(p->pointer) = 0;
		}
		else if (T_type == T_FLOAT)
		{
			p->T_type = T_FLOAT;
			p->pointer = malloc(sizeof(float));
			*(float*)(p->pointer) = (float)0;
		}
		else
		{
			p->T_type = T_STRING;
			p->pointer = (char*)malloc(sizeof(char) * MAX_SIZE_TOKEN_VALUE);
			memset(p->pointer, 0, sizeof(char) * MAX_SIZE_TOKEN_VALUE);
			strcpy(p->pointer, "");
		}
	}
	else
	{
		printf("error variable type\n");
		exit(-1);
	}
	return 0;
}


//修改前先确认该变量是否存在	T_Type为希望修改的数据类型(千万注意之前先检查数字是否为float和int，对应修改T_type)
int modify_variable_node(char id[], char value[], dian_en_token_id T_type, int place, dian_env* extra_env)
{
	dian_env_var_node* tmp = get_variable_node(id, extra_env,0);
	if (tmp == NULL)
	{
		printf("variable not existed\n");
		exit(-1);
	}
	if (tmp->T_type == T_ASSIGN)
	{
		printf("assign variable can not be modify\n");
		exit(-1);
	}

	//number变量允许修改类型
	if (T_type == T_FLOAT || T_type == T_INT)
	{
		if (tmp->T_type != T_FLOAT && tmp->T_type != T_INT)
		{
			printf("can not modify a non-number variable to number variable\n");
			exit(-1);
		}
		if (T_type == T_FLOAT)
		{
			*(float*)tmp->value->pointer = atof(value);
			tmp->T_type = T_FLOAT;
			tmp->value->T_type = T_FLOAT;
		}
		if (T_type == T_INT)
		{
			*(int*)tmp->value->pointer = atoi(value);
			tmp->T_type = T_INT;
			tmp->value->T_type = T_INT;
		}
	}
	if (T_type == T_INT_ARRAY || T_type == T_FLOAT_ARRAY)
	{
		if (tmp->T_type != T_INT_ARRAY && tmp->T_type != T_FLOAT_ARRAY)
		{
			printf("error assign to a non-array variable\n");
			exit(-1);
		}
		int n = place;
		if (T_type == T_FLOAT_ARRAY)
		{
			if (n < 0)
			{
				int length = tmp->length;
				memory_block* p = tmp->value;
				for (int i = 0; i < length; i++)
				{
					*(float*)(p + i)->pointer = atof(value);
					(p + i)->T_type = T_FLOAT;
				}
			}
			else
			{
				memory_block* p = tmp->value;
				*(float*)(p + n)->pointer = atof(value);
				(p + n)->T_type = T_FLOAT;
			}
		}
		if (T_type == T_INT_ARRAY)
		{
			if (n < 0)
			{
				int length = tmp->length;
				memory_block* p = tmp->value;
				for (int i = 0; i < length; i++)
				{
					*(int*)(p + i)->pointer = atoi(value);
					(p + i)->T_type = T_INT;
				}
			}
			else
			{
				memory_block* p = tmp->value;
				*(int*)(p + n)->pointer = atoi(value);
				(p + n)->T_type = T_INT;
			}
		}
	}
	if (T_type == T_STRING)
	{
		if (tmp->T_type != T_STRING)
		{
			printf("can not assign a string to a non-string avilable\n");
			exit(-1);
		}
		memset(tmp->value->pointer, 0, MAX_SIZE_TOKEN_VALUE);
		strcpy((char*)(tmp->value->pointer), value);
	}
	if (T_type == T_STRING_ARRAY)
	{
		int n = place;
		if (tmp->T_type != T_STRING_ARRAY)
		{
			printf("the variable is not a string array\n");
			exit(-1);
		}
		int length = tmp->length;
		memory_block* p = tmp->value;
		if (n < 0)
		{
			for (int i = 0; i < length; i++)
			{
				memset((char*)(p + i)->pointer, 0, MAX_SIZE_TOKEN_VALUE);
				strcpy((char*)((p + i)->pointer), value);
			}

		}
		else
		{
			memset((char*)(p + n)->pointer, 0, MAX_SIZE_TOKEN_VALUE);
			strcpy((char*)((p + n)->pointer), value);
		}
	}

	return 0;
}


//获取前先确认该变量存在
ASTnode* get_variable_value(char id[], int pos, dian_env* extra_env)
{
	dian_env_var_node* tmp = get_variable_node(id, extra_env,0);
	if (tmp == NULL)
	{
		printf("this id is not existed\n");
		exit(-1);
	}
	ASTnode* node = malloc(sizeof(ASTnode));
	if (tmp->T_type == T_ASSIGN)
	{
		ASTnode* tmp_node = (ASTnode*)(tmp->value->pointer);
		node = tmp_node->eval(tmp_node);
		return node;
	}
	if (tmp->T_type == T_INT_ARRAY || tmp->T_type == T_FLOAT_ARRAY)
	{
		memory_block* p = tmp->value;
		int n = pos;
		if (n < 0)
		{
			printf("error pos for a array\n");
			exit(-1);
		}
		if ((p + n)->T_type == T_INT)
		{
			int int_number = *(int*)((p + n)->pointer);
			node->type = number;
			node->eval = cb_eval_number;
			node->property.p_number = malloc(sizeof(ASTnode_number));
			node->property.p_number->type = C_INT;
			selflib_itoa(int_number, node->property.p_number->value);
			return node;
		}
		else
		{
			float float_number = *(float*)((p + n)->pointer);
			node->type = number;
			node->eval = cb_eval_number;
			node->property.p_number = malloc(sizeof(ASTnode_number));
			node->property.p_number->type = C_FLOAT;
			selflib_ftoa(float_number, node->property.p_number->value);
			return node;
		}

	}
	if (tmp->T_type == T_INT || tmp->T_type == T_FLOAT)
	{
		if (tmp->T_type == T_INT)
		{
			int int_number = *(int*)(tmp->value->pointer);
			node->type = number;
			node->eval = cb_eval_number;
			node->property.p_number = malloc(sizeof(ASTnode_number));
			node->property.p_number->type = C_INT;
			selflib_itoa(int_number, node->property.p_number->value);
			return node;
		}
		if (tmp->T_type == T_FLOAT)
		{
			float float_number = *(float*)(tmp->value->pointer);
			node->type = number;
			node->eval = cb_eval_number;
			node->property.p_number = malloc(sizeof(ASTnode_number));
			node->property.p_number->type = C_FLOAT;
			selflib_ftoa(float_number, node->property.p_number->value);
			return node;
		}
	}
	if (tmp->T_type == T_STRING)
	{
		node->type = string;
		node->eval = cb_eval_string;
		node->property.p_string = malloc(sizeof(ASTnode_string));
		memset(node->property.p_string->value, 0, sizeof(ASTnode_string));
		strcpy(node->property.p_string->value, (char*)tmp->value->pointer);
		return node;
	}
	if (tmp->T_type == T_STRING_ARRAY)
	{
		int n = pos;
		if (n < 0)
		{
			printf("error pos for a array\n");
			exit(-1);
		}
		memory_block* p = tmp->value;
		node->type = string;
		node->eval = cb_eval_string;
		node->property.p_string = malloc(sizeof(ASTnode_string));
		memset(node->property.p_string->value, 0, sizeof(ASTnode_string));
		strcpy(node->property.p_string->value, (char*)(p + n)->pointer);
		return node;
	}
	else
	{
		printf("unknown variable type\n");
		exit(-1);
	}
	return NULL;
}








//函数环境的结构
/*
typedef struct function_node
{
	char id[MAX_ID_LENGTH];
	ASTnode* fun_def_pos;
	struct function_node* next_func;
}dian_env_func_node;


typedef struct function_env_table
{
	dian_env_func_node* Dict[26];
}dian_func_table;
*/




dian_env_func_node* get_func_node(char id[], dian_env* extra_env)
{
	dian_env* global_env = &g_env;
	dian_env* local_env = NULL;
	if (extra_env != NULL)
	{
		local_env = extra_env;
	}
	int number = get_first_char(id);
	dian_env_func_node* tmp = NULL;
	if (local_env != NULL)
	{
		tmp = local_env->func_table->Dict[number];
		while (tmp != NULL)
		{
			int ret = strcmp(tmp->id, id);
			if (ret == 0)
			{
				return tmp;
			}
			tmp = tmp->next_func;
		}
	}

	tmp = global_env->func_table->Dict[number];
	while (tmp != NULL)
	{
		int ret = strcmp(tmp->id, id);
		if (ret == 0)
		{
			return tmp;
		}
		tmp = tmp->next_func;
	}

	return NULL;
}


//
int register_func_node(char id[], ASTnode* function, dian_env* extra_env)
{

	int number = get_first_char(id);
	dian_env* env = extra_env;
	if (extra_env == NULL)
	{
		env = &g_env;
	}

	dian_env_func_node* tmp = env->func_table->Dict[number];
	if (tmp == NULL)
	{
		env->func_table->Dict[number] = malloc(sizeof(dian_env_func_node));
		tmp = env->func_table->Dict[number];
		tmp->fun_def_pos = function;
		strcpy(tmp->id, id);
		tmp->next_func = NULL;
	}
	else
	{
		while (tmp->next_func != NULL)
		{
			tmp = tmp->next_func;
		}
		tmp->next_func = malloc(sizeof(dian_env_func_node));
		tmp = tmp->next_func;
		tmp->fun_def_pos = function;
		strcpy(tmp->id, id);
		tmp->next_func = NULL;
	}
	return 1;
}