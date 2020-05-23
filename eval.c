#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "env.h"
#include "lexer.h"
#include "lib.h"
#include "parser.h"
#include "eval.h"

/********************************************************
*       以下两个函数完成将expression_list               *
*       中每个节点进行运算，得到最终的结果              *
*                                                       *
********************************************************/

//从exprlist 中找到第 number 个非expression-list 的expression
ASTnode* get_expr_from_exprlist(ASTnode* exprlist, int* number)
{
	if (number == NULL)
	{
		return NULL;
	}
	if (exprlist == NULL)
	{
		return NULL;
	}
	if (exprlist->type != expression_list)
	{
		if (*number == 1)
			return exprlist;
		else
			return NULL;
	}
	int* p = number;
	ASTnode_expression_list* tmp = exprlist->property.p_expression_list;
	ASTnode* result = NULL;
	while (*p > 0 && tmp != NULL)
	{
		ASTnode* now_expression = tmp->expression;
		if (now_expression->type == expression_list)
		{
			now_expression = get_expr_from_exprlist(now_expression, p);
		}
		else
		{
			(*p)--;
		}
		if (*p == 0)
		{
			result = now_expression;
			return result;
		}
		tmp = tmp->next_expression;
	}
	return NULL;
}

//将expression_list 执行至每个都为number or string（如果调用的expression返回一个expression-list 会递归调用）
ASTnode* eval_expression_list(ASTnode* expr_list)
{
	if (expr_list == NULL)
	{
		return NULL;
	}
	if(expr_list->type != expression_list)
	{
		return expr_list->eval(expr_list);
	}
	ASTnode* node = malloc(sizeof(ASTnode));
	node->type = expression_list;
	node->property.p_expression_list = malloc(sizeof(ASTnode_expression_list));
	node->property.p_expression_list->expression = NULL;
	node->property.p_expression_list->next_expression = NULL;
	int n = 1;
	int control = 0;
	do
	{
		int p = n;
		ASTnode* expression = get_expr_from_exprlist(expr_list,&p);
		if (expression == NULL)
			break;
		expression = expression->eval(expression);
		if (expression->type != number && expression->type != string)
			control = 1;
		create_expression_list(node, expression);
		n++;
	} while (1);
	if (control == 1)
	{
		node = eval_expression_list(node);
	}
	return node;
}



/********************************************************
*                                                       *
*               数组部分的一些处理                      *
*                                                       *
********************************************************/


//获取数组的前pos（可能是唯一节点）
int get_array_number(ASTnode* array_node)
{
	ASTnode* tmp = array_node->property.p_array->expression_start->eval(array_node->property.p_array->expression_start);
	if (tmp->type != number)
	{
		printf("数组序号必须为一个自然数\n");
		exit(-1);
	}
	if (tmp->property.p_number->type != C_INT)
	{
		printf("数组序号必须为一个自然数\n");
		exit(-1);
	}
	int result = atoi(tmp->property.p_number->value);
	if (result < 0)
	{
		printf("数组序号必须为一个自然数\n");
		exit(-1);
	}
	return result;
}

//获取数组的后pos（无后pos返回-1）
int get_array_number_end(ASTnode* array_node)
{
	if (array_node->property.p_array->expression_end == NULL)
	{
		return -1;
	}
	ASTnode* tmp = array_node->property.p_array->expression_end->eval(array_node->property.p_array->expression_end);
	if (tmp->type != number)
	{
		printf("数组序号必须为一个自然数\n");
		exit(-1);
	}
	if (tmp->property.p_number->type != C_INT)
	{
		printf("数组序号必须为一个自然数\n");
		exit(-1);
	}
	int result = atoi(tmp->property.p_number->value);
	if (result < 0)
	{
		printf("数组序号必须为一个自然数\n");
		exit(-1);
	}
	return result;
}

/********************************************************
*                                                       *
*               基本节点的eval函数                      *
*                                                       *
********************************************************/
ASTnode* cb_eval_number(void* arg)
{
	ASTnode* self = (ASTnode*)arg;
	return self;
}

ASTnode* cb_eval_string(void* arg)
{
	ASTnode* self = (ASTnode*)arg;
	return self;
}

//该处使用了环境变量
ASTnode* cb_eval_variable(void* arg)
{
	ASTnode* self = (ASTnode*)arg;
	dian_env_var_node* judge =  get_variable_node(self->property.p_variable->variable_id, get_top_env_stack(),0);
	if (judge == NULL)
	{
		printf("variable is not existed\n");
		exit(-1);
	}
	if (judge->length != 1)
	{
		printf("incorrect use array variable\n");
		exit(-1);
	}
	return get_variable_value(self->property.p_variable->variable_id, 1, get_top_env_stack());
}

//返回值为number string 或者 expression_list（数组片）
ASTnode* cb_eval_array(void* arg)
{
	ASTnode* self = (ASTnode*)arg;

	dian_env_var_node* judge = get_variable_node(self->property.p_array->array_id, get_top_env_stack(),0);
	if (judge == NULL)
	{
		printf("variable is not existed\n");
		exit(-1);
	}
	else if (judge->T_type == T_STRING)
	{
		ASTnode* node = malloc(sizeof(ASTnode));
		node->type = string;
		node->property.p_string = malloc(sizeof(ASTnode_string));
		int start = get_array_number(self);
		int end = get_array_number_end(self);
		if (end == -1)
		{
			ASTnode* tmp = get_variable_value(self->property.p_array->array_id, 1, get_top_env_stack());
			char m = tmp->property.p_string->value[start];
			char p[2];
			p[0] = m;
			p[1] = '\0';
			strcpy(node->property.p_string->value, p);
			return node;
		}
		if (end <= start)
		{
			printf("字符串切片尾部必须大于头部\n");
			exit(-1);
		}
		ASTnode* tmp = get_variable_value(self->property.p_array->array_id, 1, get_top_env_stack());
		int length = end - start;
		for (int i = 0; i <= length; i++)
		{
			node->property.p_string->value[i] = tmp->property.p_string->value[i + start];
		}
		return node;
	}
	else {
		int start = get_array_number(self);
		int end = get_array_number_end(self);
		if (end == -1)
		{
			return get_variable_value(self->property.p_array->array_id, start, get_top_env_stack());
		}
		else
		{
			ASTnode* node = malloc(sizeof(ASTnode));
			node->type = expression_list;
			node->property.p_expression_list = malloc(sizeof(ASTnode_expression_list));
			node->property.p_expression_list->expression = NULL;
			node->property.p_expression_list->next_expression = NULL;
			if (end <= start)
			{
				printf("数组切片尾部必须大于头部\n");
				exit(-1);
			}
			else
			{
				for (int i = start; i <= end; i++)
				{
					ASTnode* result = get_variable_value(self->property.p_array->array_id, i, get_top_env_stack());
					create_expression_list(node, result);
				}
			}
			return node;
		}
	}
}

/********************************************************
*                                                       *
*                   赋值的相关处理                      *
*                                                       *
********************************************************/

//对数组整体的赋值(数组切片)
ASTnode* assign_array(ASTnode* array_var, ASTnode* value)
{
	dian_env_var_node* judge = get_variable_node(array_var->property.p_array->array_id, get_top_env_stack(),0);
	if (judge == NULL)
	{
		printf("array slice can be used to define a array\n");
		exit(-1);
	}
	int start_pos = get_array_number(array_var);
	int end_pos = get_array_number_end(array_var);
	if (start_pos >= end_pos)
	{
		printf("array slice error : start-pos must more than end-pos\n");
		exit(-1);
	}
	for (int i = start_pos; i <= end_pos; i++)
	{

		assign_variable(array_var, i, value);

	}
	return value;
}

//对变量的赋值（定义）	var为variable或arraynode，value为number或string pos小于0则表示pos根据arraynode决定（pos的值使用在数组切片中）
ASTnode* assign_variable(ASTnode* var, int pos,ASTnode* value)
{
	if (value->type == number)
	{
		if (var->type == variable)
		{
			dian_env_var_node* judge = get_variable_node(var->property.p_variable->variable_id, get_top_env_stack(),0);
			if (judge == NULL)
			{
				register_variable_node(var->property.p_variable->variable_id, \
					value->property.p_number->type - 6, 1, get_top_env_stack());
				modify_variable_node(var->property.p_variable->variable_id, \
					value->property.p_number->value, value->property.p_number->type - 6, \
					1, get_top_env_stack());
				return value;
			}
			else
			{
				modify_variable_node(var->property.p_variable->variable_id, \
					value->property.p_number->value, value->property.p_number->type - 6, \
					1, get_top_env_stack());
				return value;
			}
		}
		else if (var->type == array_node || var->type == array_node_slice)
		{
			if (var->type == array_node_slice && pos < 0)
			{
				printf("error in assign variable\n");
			}
			dian_env_var_node* judge = get_variable_node(var->property.p_array->array_id, get_top_env_stack(),0);
			if (judge == NULL)
			{
				int	length = get_array_number(var);
				register_variable_node(var->property.p_array->array_id, \
					value->property.p_number->type +3, length, get_top_env_stack());
				modify_variable_node(var->property.p_array->array_id, \
					value->property.p_number->value, value->property.p_number->type + 3, \
					-1, get_top_env_stack());
				return value;
			}
			else
			{
				int length = pos;
				if (pos < 0)
				{
					length = get_array_number(var);
				}
				modify_variable_node(var->property.p_array->array_id, \
					value->property.p_number->value, value->property.p_number->type + 3, \
					length, get_top_env_stack());
				return value;
			}
		}
		else
		{
			printf("等号左边必须为一个变量\n");
			exit(-1);
		}
	}
	if (value->type == string)
	{
		if (var->type == variable)
		{
			dian_env_var_node* judge = get_variable_node(var->property.p_variable->variable_id, get_top_env_stack(),0);
			if (judge == NULL)
			{
				register_variable_node(var->property.p_variable->variable_id, T_STRING\
					 ,1, get_top_env_stack());
				modify_variable_node(var->property.p_variable->variable_id, \
					value->property.p_string->value, T_STRING, \
					1, get_top_env_stack());
				return value;
			}
			else
			{
				modify_variable_node(var->property.p_variable->variable_id, \
					value->property.p_string->value, T_STRING, \
					1, get_top_env_stack());
			}
		}
		else if (var->type == array_node || var->type == array_node_slice)
		{
			if (var->type == array_node_slice && pos < 0)
			{
				printf("error in assign variable\n");
			}
			dian_env_var_node* judge = get_variable_node(var->property.p_array->array_id, get_top_env_stack(),0);
			if (judge == NULL)
			{
				int length = get_array_number(var);
				register_variable_node(var->property.p_array->array_id, \
					T_STRING_ARRAY, length, get_top_env_stack());
				modify_variable_node(var->property.p_variable->variable_id, \
					value->property.p_string->value, T_STRING_ARRAY, \
					- 1, get_top_env_stack());
				return value;
			}
			else
			{

				int length = pos;
				if (pos < 0)
				{
					length = get_array_number(var);
				}
				modify_variable_node(var->property.p_variable->variable_id, \
					value->property.p_string->value, T_STRING_ARRAY, \
					length, get_top_env_stack());
				return value;
			}
		}
		else
		{
			printf("等号左边必须为一个变量\n");
			exit(-1);
		}
	}
	else
	{
		printf("等式右边应当为一常量\n");
		exit(-1);
	}
	return value;
}

ASTnode* array_slice_to_expression_list(ASTnode* array_slice)
{
	dian_env_var_node* judge = get_variable_node(array_slice->property.p_array->array_id, get_top_env_stack(),0);
	if (judge == NULL)
	{
		printf("array slice can be used to define a array\n");
		exit(-1);
	}
	int start_pos = get_array_number(array_slice);
	int end_pos = get_array_number_end(array_slice);
	if (start_pos >= end_pos)
	{
		printf("array slice error : start-pos must more than end-pos\n");
		exit(-1);
	}

	ASTnode* expr_list = (ASTnode*)malloc(sizeof(ASTnode));
	expr_list->type = expression_list;
	expr_list->property.p_expression_list = (ASTnode_expression_list*)malloc(sizeof(ASTnode_expression_list));
	expr_list->property.p_expression_list->expression = NULL;
	expr_list->property.p_expression_list->next_expression = NULL;

	for (int i = start_pos; i <= end_pos; i++)
	{
		ASTnode* tmp = (ASTnode*)malloc(sizeof(ASTnode));
		tmp->type = array_node;
		tmp->eval = cb_eval_array;
		tmp->property.p_array = (ASTnode_array*)malloc(sizeof(ASTnode_array));
		strcpy(tmp->property.p_array->array_id, array_slice->property.p_array->array_id);
		ASTnode* tmp_number = (ASTnode*)malloc(sizeof(ASTnode));
		tmp_number->type = number;
		tmp_number->eval = cb_eval_number;
		tmp_number->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
		tmp_number->property.p_number->type = C_INT;
		selflib_itoa(i,tmp_number->property.p_number->value);
		tmp->property.p_array->expression_start = tmp_number;
		tmp->property.p_array->expression_end = NULL;
		create_expression_list(expr_list, tmp);
	}
	return expr_list;
}

//赋值运算（不存在则同时定义）
ASTnode* cb_eval_assignop(void* arg)
{
	ASTnode* self = (ASTnode*)arg;
	ASTnode* left = self->property.p_assignop->left;
	ASTnode* right = self->property.p_assignop->right;
	right = eval_expression_list(right);
	if (left->type == array_node || left->type == variable)
	{
		if (right->type == expression_list)
		{
			printf("assign opreator  both side number must be equal\n");
			exit(-1);
		}
		return assign_variable(left, -1, right);
	}
	if (left->type == array_node_slice)
	{
		if (right->type != expression_list)
		{
			return assign_array(left, right);
		}
		else
		{
			left = array_slice_to_expression_list(left);
		}
	}

		//此时left与right均为expression_list

	ASTnode_expression_list* tmp_expression_list = left->property.p_expression_list;
	while (tmp_expression_list != NULL)
	{
		ASTnode* now_expression = tmp_expression_list->expression;
		if (now_expression->type == array_node_slice)
		{
			tmp_expression_list->expression = array_slice_to_expression_list(now_expression);
		}
		tmp_expression_list = tmp_expression_list->next_expression;
	}

	ASTnode* right_result = eval_expression_list(right);
	ASTnode* tmp_left = NULL;
	ASTnode* tmp_right = NULL;
	ASTnode* return_list = malloc(sizeof(ASTnode));
	return_list->type = expression_list;
	return_list->eval = cb_eval_expression_list;
	return_list->property.p_expression_list = (ASTnode_expression_list*)malloc(sizeof(ASTnode_expression_list));
	return_list->property.p_expression_list->expression = NULL;
	return_list->property.p_expression_list->next_expression = NULL;

	int n = 1;
	do
	{
		int pl = n;
		int pr = n;
		tmp_left = get_expr_from_exprlist(left, &pl);
		tmp_right = get_expr_from_exprlist(right_result, &pr);
		if (tmp_right == NULL && tmp_left == NULL)
		{
			return return_list;
		}
		if (tmp_left == NULL && tmp_right != NULL)
		{
			printf("赋值表达式两边变量应相同\n");
			exit(-1);
		}
		if (tmp_right == NULL && tmp_left != NULL)
		{
			printf("赋值表达式两边变量应相同\n");
			exit(-1);
		}
		if (tmp_left->type != variable && tmp_left->type != array_node)
		{
			printf("等式左边必须是变量\n");
			exit(-1);
		}
		ASTnode* result = assign_variable(tmp_left, -1, tmp_right);
		create_expression_list(return_list, result);
		n++;
	} while (tmp_left != NULL && tmp_right != NULL);

	return return_list;

}

/********************************************************
*                                                       *
*               变量与函数定义的相关处理                *
*                                                       *
********************************************************/
//对var（可以为variable 也 可以为 array_node）进行定义，如果被重定义则报错
int def_variable(ASTnode* var, ASTnode* value, dian_en_token_id T_type)
{
	if (var->type == variable)
	{
		dian_env_var_node* judge = get_variable_node(var->property.p_variable->variable_id, get_top_env_stack(),1);
		if (judge != NULL)
		{
			printf("该变量名已被定义\n");
			printf("error in %s\n", __FUNCTION__);
			exit(-1);
		}
		if (value == NULL)
		{
			register_variable_node(var->property.p_variable->variable_id, \
				T_type, 1, get_top_env_stack());
			return 1;
		}
		else
		{
			assign_variable(var, -1,value);
		}
	}
	else if(var->type == array_node)
	{
		dian_env_var_node* judge = get_variable_node(var->property.p_array->array_id, get_top_env_stack(),1);
		if (judge != NULL)
		{
			printf("该变量名已被定义\n");
			printf("error in %s\n", __FUNCTION__);
			exit(-1);
		}
		if (value == NULL)
		{
			int length = get_array_number(var);
			register_variable_node(var->property.p_array->array_id, \
				T_type+9, length, get_top_env_stack());
			return 1;
		}
		else
		{
			assign_variable(var,-1, value);
		}
	}
	return 0;
}

//声明节点的回调
ASTnode* cb_eval_def_var(void* arg)
{
	ASTnode* self = (ASTnode*)arg;
	dian_en_token_id type = self->property.p_def_var->type;
	ASTnode* variable_list = self->property.p_def_var->expression;
	ASTnode* assign_expression = self->property.p_def_var->assign;
	if (assign_expression != NULL)
	{
		ASTnode* assign_result = NULL;
		assign_result = assign_expression->eval(assign_expression);
		if (assign_expression->type == expression_list)
		{
			assign_result = eval_expression_list(assign_expression);
		}

		if (variable_list->type != expression_list)
		{
			def_variable(variable_list, assign_result,type);
			return NULL;
		}

		int i = 1;
		do
		{
			int pl = i;
			int pr = i;
			ASTnode* tmp_left = NULL;
			ASTnode* tmp_right = NULL;
			tmp_left = get_expr_from_exprlist(variable_list, &pl);
			tmp_right = get_expr_from_exprlist(assign_result, &pr);
			if (tmp_left == NULL)
			{
				return NULL;
			}
			def_variable(tmp_left, tmp_right, type);
			i++;
		} while (1);
	}
	else
	{
		if (variable_list->type != expression_list)
		{
			def_variable(variable_list, NULL, type);
		}
		else
		{
			int i = 1;
			while (1)
			{
				int p = i;
				ASTnode* tmp_left = get_expr_from_exprlist(variable_list, &p);
				if (tmp_left == NULL)
				{
					return NULL;
				}
				def_variable(tmp_left,NULL, type);
				i++;
			}
		}
		
	}

	return NULL;
}

//函数定义
ASTnode* cb_eval_def_func(void* arg)
{
	ASTnode* self = (ASTnode*)arg;
	dian_env_func_node* judge = get_func_node(self->property.p_def_func->name, get_top_env_stack());
	if (judge != NULL)
	{
		printf("函数重定义\n");
		exit(-1);
	}
	register_func_node(self->property.p_def_func->name, self, get_top_env_stack());
	return NULL;
}


ASTnode* cb_eval_def_assign(void* arg)
{
	ASTnode* self = (ASTnode*)arg;
	dian_env_var_node* judge = get_variable_node(self->property.p_def_assign->variable_id, get_top_env_stack(),1);
	if (judge != NULL)
	{
		printf("该变量已经被定义\n");
		exit(-1);
	}
	int number = get_first_char(self->property.p_def_assign->variable_id);
	dian_env* env = get_top_env_stack();
	if (env == NULL)
	{
		env = &g_env;
	}
	dian_env_var_node* tmp = env->var_table->Dict[number];

	if (tmp == NULL)
	{
		env->var_table->Dict[number] = malloc(sizeof(dian_env_var_node));
		tmp = env->var_table->Dict[number];
		tmp->next_var = NULL;

	}
	else
	{
		while (tmp->next_var != NULL)
		{
			tmp = tmp->next_var;
		}
		tmp->next_var = malloc(sizeof(dian_env_var_node));
		tmp = tmp->next_var;
		tmp->next_var = NULL;
	}
	tmp->length = 1;
	strcpy(tmp->id, self->property.p_def_assign->variable_id);
	tmp->T_type = T_ASSIGN;
	tmp->length = 1;
	tmp->value = malloc(sizeof(memory_block));
	tmp->value->T_type = T_ASSIGN;
	tmp->value->pointer = (ASTnode*)(self->property.p_def_assign->value_expression);
	return NULL;
}


/********************************************************
*                                                       *
*               运算符的eval函数                        *
*                                                       *
********************************************************/

//二元运算符的执行函数
ASTnode* cb_eval_binop(void* arg)
{
	ASTnode* self = (ASTnode*)arg;
	ASTnode* left = self->property.p_binop->left;
	ASTnode* right = self->property.p_binop->right;
	dian_en_token_id op = self->property.p_binop->op;
	if (left == NULL || right == NULL)
	{
		printf("error in %s\n", __FUNCTION__);
		exit(-1);
	}
	if (left->eval == NULL || right->eval == NULL)
	{
		printf("error in %s\n", __FUNCTION__);
		exit(-1);
	}
	switch (op)
	{
	case O_PLUS:
		left = left->eval(left);
		if (left->type == number || left->type == string)
		{
			right = right->eval(right);
			if (right->type == left->type)
			{
				if (right->type == string)
				{
					ASTnode* node = malloc(sizeof(ASTnode));
					node->type = string;
					node->eval = cb_eval_string;
					node->property.p_string = malloc(sizeof(ASTnode_string));
					char tmp_string[64] = { 0 };
					strcat(tmp_string, left->property.p_string->value);
					strcat(tmp_string, right->property.p_string->value);
					strcpy(node->property.p_string->value, tmp_string);
					return node;
				}
				else if (right->type == number)
				{
					//获取数值
					float left_number,right_number;
					if (left->property.p_number->type == C_INT)
					{
						left_number = atoi(left->property.p_number->value);
					}
					else if(left->property.p_number->type == C_FLOAT)
					{
						left_number = atof(left->property.p_number->value);
					}
					if (right->property.p_number->type == C_INT)
					{
						right_number = atoi(right->property.p_number->value);
					}
					else if (right->property.p_number->type == C_FLOAT)
					{
						right_number = atof(right->property.p_number->value);
					}

					float result = left_number + right_number;
					ASTnode* node = malloc(sizeof(ASTnode));
					node->type = number;
					node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
					node->eval = cb_eval_number;
					if (is_float(result))
					{
						node->property.p_number->type = C_FLOAT;
						selflib_ftoa(result, node->property.p_number->value);
					}
					else
					{
						node->property.p_number->type = C_INT;
						selflib_itoa((int)result, node->property.p_number->value);
					}

					return node;
				}
			}
			else
			{
				//报错
				printf("binary operator \"+\" error right-type different with left-type\n");
				exit(-1);
			}
		}
		else
		{
			//报错
			printf("binary operator \"+\" error\n");
			printf("error in %s\n", __FUNCTION__);
			exit(-1);
		}
		break;
	case O_MINUS:
		left = left->eval(left);
		if (left->type == number)
		{
			right = right->eval(right);
			if (right->type == number)
			{
				//获取数值
				float left_number, right_number;
				if (left->property.p_number->type == C_INT)
				{
					left_number = atoi(left->property.p_number->value);
				}
				else
				{
					left_number = atof(left->property.p_number->value);
				}
				if (right->property.p_number->type == C_INT)
				{
					right_number = atoi(right->property.p_number->value);
				}
				else
				{
					right_number = atof(right->property.p_number->value);
				}

				float result = left_number - right_number;
				ASTnode* node = malloc(sizeof(ASTnode));
				node->type = number;
				node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
				node->eval = cb_eval_number;
				if (is_float(result))
				{
					node->property.p_number->type = C_FLOAT;
					selflib_ftoa(result, node->property.p_number->value);
				}
				else
				{
					node->property.p_number->type = C_INT;
					selflib_itoa((int)result, node->property.p_number->value);
				}

				return node;
			}
			else
			{
				//报错
				printf("left type different with right type\n");
			}
		}
		else
		{
			//报错
			printf("binary operator \"-\" error\n");
			exit(-1);
		}
		break;
	case O_MUL:
		left = left->eval(left);
		if (left->type == number)
		{
			right = right->eval(right);
			if (right->type == number)
			{
				//获取数值
				float left_number, right_number;
				if (left->property.p_number->type == C_INT)
				{
					left_number = atoi(left->property.p_number->value);
				}
				else
				{
					left_number = atof(left->property.p_number->value);
				}
				if (right->property.p_number->type == C_INT)
				{
					right_number = atoi(right->property.p_number->value);
				}
				else
				{
					right_number = atof(right->property.p_number->value);
				}

				float result = left_number * right_number;
				ASTnode* node = malloc(sizeof(ASTnode));
				node->type = number;
				node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
				node->eval = cb_eval_number;
				if (is_float(result))
				{
					node->property.p_number->type = C_FLOAT;
					selflib_ftoa(result, node->property.p_number->value);
				}
				else
				{
					node->property.p_number->type = C_INT;
					selflib_itoa((int)result, node->property.p_number->value);
				}

				return node;
			}
			else
			{
				//报错
				printf("binary operator\"*\"left type different with right type\n");
			}
		}
		else
		{
			//报错
			printf("binary operator \"*\" error\n");
			exit(-1);
		}
		break;
	case O_F_DIV:
		left = left->eval(left);
		if (left->type == number)
		{
			right = right->eval(right);
			if (right->type == number)
			{
				//获取数值
				float left_number, right_number;
				if (left->property.p_number->type == C_INT)
				{
					left_number = atoi(left->property.p_number->value);
				}
				else
				{
					left_number = atof(left->property.p_number->value);
				}
				if (right->property.p_number->type == C_INT)
				{
					right_number = atoi(right->property.p_number->value);
				}
				else
				{
					right_number = atof(right->property.p_number->value);
				}
				float result = left_number / right_number;
				ASTnode* node = malloc(sizeof(ASTnode));
				node->type = number;
				node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
				node->eval = cb_eval_number;
				if (is_float(result))
				{
					node->property.p_number->type = C_FLOAT;
					selflib_ftoa(result, node->property.p_number->value);
				}
				else
				{
					node->property.p_number->type = C_INT;
					selflib_itoa((int)result, node->property.p_number->value);
				}
				return node;
			}
			else
			{
				//报错
				printf("binary operator\"/\"left type different with right type\n");
			}
		}
		else
		{
			//报错
			printf("binary operator \"/\" error\n");
			exit(-1);
		}
		break;
	case O_I_DIV:
		left = left->eval(left);
		if (left->type == number)
		{
			if (left->property.p_number->type == C_INT)
			{
				right = right->eval(right);
				if (right->type == number && right->property.p_number->type == C_INT)
				{
					int left_number, right_number;
					left_number = atoi(left->property.p_number->value);
					right_number = atoi(right->property.p_number->value);
					int result = left_number / right_number;
					ASTnode* node = malloc(sizeof(ASTnode));
					node->type = number;
					node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
					node->eval = cb_eval_number;
					node->property.p_number->type = C_INT;
					selflib_itoa(result, node->property.p_number->value);
					return node;
				}
				else
				{
					printf("exact division must be int\n");
					exit(-1);
				}
			}
			else
			{
				printf("exact division must be int\n");
				exit(-1);
			}
		}
		else
		{
			//报错
			printf("binary operator \"//\" error\n");
			exit(-1);
		}
		break;
	case O_REM:
		left = left->eval(left);
		if (left->type == number)
		{
			if (left->property.p_number->type == C_INT)
			{
				right = right->eval(right);
				if (right->type == number && right->property.p_number->type == C_INT)
				{
					int left_number, right_number;
					left_number = atoi(left->property.p_number->value);
					right_number = atoi(right->property.p_number->value);
					int result = left_number % right_number;
					ASTnode* node = malloc(sizeof(ASTnode));
					node->type = number;
					node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
					node->eval = cb_eval_number;
					node->property.p_number->type = C_INT;
					selflib_itoa(result, node->property.p_number->value);
					return node;
				}
				else
				{
					printf("% must be int\n");
					exit(-1);
				}
			}
			else
			{
				printf("% must be int\n");
				exit(-1);
			}
		}
		else
		{
			//报错
			printf("binary operator \"%\" error\n");
			exit(-1);
		}
		break;
	case O_GREAT:
		left = left->eval(left);
		if (left->type == number)
		{
			right = right->eval(right);
			if (right->type == number)
			{
				//获取数值
				float left_number, right_number;
				if (left->property.p_number->type == C_INT)
				{
					left_number = atoi(left->property.p_number->value);
				}
				else
				{
					left_number = atof(left->property.p_number->value);
				}
				if (right->property.p_number->type == C_INT)
				{
					right_number = atoi(right->property.p_number->value);
				}
				else
				{
					right_number = atof(right->property.p_number->value);
				}
				int result;
				if (left_number > right_number)
				{
					result = 1;
				}
				else
				{
					result = 0;
				}
				ASTnode* node = malloc(sizeof(ASTnode));
				node->type = number;
				node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
				node->eval = cb_eval_number;
				node->property.p_number->type = C_INT;
				selflib_itoa(result, node->property.p_number->value);
				return node;
			}
			else
			{
				//报错
				printf("binary operator \">\" error\n");
				exit(-1);
			}
		}
		else
		{
			//报错
			printf("binary operator \">\" error\n");
			exit(-1);
		}
		break;
	case O_LESS:
		left = left->eval(left);
		if (left->type == number)
		{
			right = right->eval(right);
			if (right->type == number)
			{
				//获取数值
				float left_number, right_number;
				if (left->property.p_number->type == C_INT)
				{
					left_number = atoi(left->property.p_number->value);
				}
				else
				{
					left_number = atof(left->property.p_number->value);
				}
				if (right->property.p_number->type == C_INT)
				{
					right_number = atoi(right->property.p_number->value);
				}
				else
				{
					right_number = atof(right->property.p_number->value);
				}
				int result;
				if (left_number < right_number)
				{
					result = 1;
				}
				else
				{
					result = 0;
				}
				ASTnode* node = malloc(sizeof(ASTnode));
				node->type = number;
				node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
				node->eval = cb_eval_number;
				node->property.p_number->type = C_INT;
				selflib_itoa(result, node->property.p_number->value);
				return node;
			}
			else
			{
				//报错
				printf("binary operator \"<\" error\n");
				exit(-1);
			}
		}
		else
		{
			//报错
			printf("binary operator \"<\" error\n");
			exit(-1);
		}
		break;
	case O_GREAT_EQUAL:
		left = left->eval(left);
		if (left->type == number)
		{
			right = right->eval(right);
			if (right->type == number)
			{
				//获取数值
				float left_number, right_number;
				if (left->property.p_number->type == C_INT)
				{
					left_number = atoi(left->property.p_number->value);
				}
				else
				{
					left_number = atof(left->property.p_number->value);
				}
				if (right->property.p_number->type == C_INT)
				{
					right_number = atoi(right->property.p_number->value);
				}
				else
				{
					right_number = atof(right->property.p_number->value);
				}
				int result;
				if (left_number >= right_number)
				{
					result = 1;
				}
				else
				{
					result = 0;
				}
				ASTnode* node = malloc(sizeof(ASTnode));
				node->type = number;
				node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
				node->eval = cb_eval_number;
				node->property.p_number->type = C_INT;
				selflib_itoa(result, node->property.p_number->value);
				return node;
			}
			else
			{
				//报错
				printf("binary operator \">=\" error\n");
				exit(-1);
			}
		}
		else
		{
			//报错
			printf("binary operator \">=\" error\n");
			exit(-1);
		}
		break;
	case O_LESS_EQUAL:
		left = left->eval(left);
		if (left->type == number)
		{
			right = right->eval(right);
			if (right->type == number)
			{
				//获取数值
				float left_number, right_number;
				if (left->property.p_number->type == C_INT)
				{
					left_number = atoi(left->property.p_number->value);
				}
				else
				{
					left_number = atof(left->property.p_number->value);
				}
				if (right->property.p_number->type == C_INT)
				{
					right_number = atoi(right->property.p_number->value);
				}
				else
				{
					right_number = atof(right->property.p_number->value);
				}
				int result;
				if (left_number <= right_number)
				{
					result = 1;
				}
				else
				{
					result = 0;
				}
				ASTnode* node = malloc(sizeof(ASTnode));
				node->type = number;
				node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
				node->eval = cb_eval_number;
				node->property.p_number->type = C_INT;
				selflib_itoa(result, node->property.p_number->value);
				return node;
			}
			else
			{
				//报错
				printf("binary operator \"<=\" error\n");
				exit(-1);
			}
		}
		else
		{
			//报错
			printf("binary operator \"<=\" error\n");
			exit(-1);
		}
		break;
	case O_EQUAL:
		left = left->eval(left);
		if (left->type == number)
		{
			if (left->property.p_number->type == C_INT)
			{
				right = right->eval(right);
				if (right->type == number && right->property.p_number->type == C_INT)
				{
					int left_number, right_number;
					int result;
					left_number = atoi(left->property.p_number->value);
					right_number = atoi(right->property.p_number->value);
					if (left_number == right_number)
					{
						result = 1;
					}
					else
					{
						result = 0;
					}
					ASTnode* node = malloc(sizeof(ASTnode));
					node->type = number;
					node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
					node->eval = cb_eval_number;
					node->property.p_number->type = C_INT;
					selflib_itoa(result, node->property.p_number->value);
					return node;
				}
				else
				{
					printf("equal-operator must be int\n");
					exit(-1);
				}
			}
			else
			{
				printf("equal-operator must be int\n");
				exit(-1);
			}
		}
		else
		{
			//报错
			printf("binary operator \"==\" error\n");
			exit(-1);
		}
		break;
	case O_NOT_EQUAL:
		left = left->eval(left);
		if (left->type == number)
		{
			right = right->eval(right);
			if (right->type == number)
			{
				//获取数值
				float left_number, right_number;
				if (left->property.p_number->type == C_INT)
				{
					left_number = atoi(left->property.p_number->value);
				}
				else
				{
					left_number = atof(left->property.p_number->value);
				}
				if (right->property.p_number->type == C_INT)
				{
					right_number = atoi(right->property.p_number->value);
				}
				else
				{
					right_number = atof(right->property.p_number->value);
				}
				int result;
				if (left_number != right_number)
				{
					result = 1;
				}
				else
				{
					result = 0;
				}
				ASTnode* node = malloc(sizeof(ASTnode));
				node->type = number;
				node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
				node->eval = cb_eval_number;
				node->property.p_number->type = C_INT;
				selflib_itoa(result, node->property.p_number->value);
				return node;
			}
			else
			{
				//报错
				printf("binary operator \"!=\" error\n");
				exit(-1);
			}
		}
		else
		{
			//报错
			printf("binary operator \"!=\" error\n");
			exit(-1);
		}
		break;
	case O_POWER:
		left = left->eval(left);
		if (left->type == number)
		{
			right = right->eval(right);
			if (right->type == number && right->property.p_number->type == C_INT)
			{
				float left_number;
				int right_number = atoi(right->property.p_number->value);
				if (right_number < 0)
				{
					printf("power right_number must more than 0\n");
					exit(-1);
				}

				if (left->property.p_number->type == C_INT)
				{
					left_number = atoi(left->property.p_number->value);
				}	
				else
				{
					left_number = atof(left->property.p_number->value);
				}
				float result = 1;
				for (int i = 0; i < right_number; i++)
				{
					result = result * left_number;
				}
				ASTnode* node = malloc(sizeof(ASTnode));
				node->type = number;
				node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
				node->eval = cb_eval_number;
				if (is_float(result))
				{
					node->property.p_number->type = C_FLOAT;
					selflib_ftoa(result, node->property.p_number->value);
				}
				else
				{
					node->property.p_number->type = C_INT;
					selflib_itoa((int)result, node->property.p_number->value);
				}

				return node;
			}
			else
			{
				printf("power de must be int");
				exit(-1);
			}
		}
		else
		{
			printf("binart operator power error\n");
			exit(-1);
		}
		break;
	case O_BIT_AND:
		left = left->eval(left);
		if (left->type == number)
		{
			if (left->property.p_number->type == C_INT)
			{
				right = right->eval(right);
				if (right->type == number && right->property.p_number->type == C_INT)
				{
					int left_number, right_number;
					left_number = atoi(left->property.p_number->value);
					right_number = atoi(right->property.p_number->value);
					int result = left_number & right_number;
					ASTnode* node = malloc(sizeof(ASTnode));
					node->type = number;
					node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
					node->eval = cb_eval_number;
					node->property.p_number->type = C_INT;
					selflib_itoa(result, node->property.p_number->value);
					return node;
				}
				else
				{
					printf("bit-operator must be int\n");
					exit(-1);
				}
			}
			else
			{
				printf("bit-operator must be int\n");
				exit(-1);
			}
		}
		else
		{
			//报错
			printf("binary operator \"&\" error\n");
			exit(-1);
		}
		break;
	case O_BIT_OR:
		left = left->eval(left);
		if (left->type == number)
		{
			if (left->property.p_number->type == C_INT)
			{
				right = right->eval(right);
				if (right->type == number && right->property.p_number->type == C_INT)
				{
					int left_number, right_number;
					left_number = atoi(left->property.p_number->value);
					right_number = atoi(right->property.p_number->value);
					int result = left_number | right_number;
					ASTnode* node = malloc(sizeof(ASTnode));
					node->type = number;
					node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
					node->eval = cb_eval_number;
					node->property.p_number->type = C_INT;
					selflib_itoa(result, node->property.p_number->value);
					return node;
				}
				else
				{
					printf("bit-operator must be int\n");
					exit(-1);
				}
			}
			else
			{
				printf("bit-operator must be int\n");
				exit(-1);
			}
		}
		else
		{
			//报错
			printf("binary operator \"|\" error\n");
			exit(-1);
		}
		break;
	case O_BIT_XNOR:
		left = left->eval(left);
		if (left->type == number)
		{
			if (left->property.p_number->type == C_INT)
			{
				right = right->eval(right);
				if (right->type == number && right->property.p_number->type == C_INT)
				{
					int left_number, right_number;
					left_number = atoi(left->property.p_number->value);
					right_number = atoi(right->property.p_number->value);
					int result = left_number ^ right_number;
					ASTnode* node = malloc(sizeof(ASTnode));
					node->type = number;
					node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
					node->eval = cb_eval_number;
					node->property.p_number->type = C_INT;
					selflib_itoa(result, node->property.p_number->value);
					return node;
				}
				else
				{
					printf("bit-operator must be int\n");
					exit(-1);
				}
			}
			else
			{
				printf("bit-operator must be int\n");
				exit(-1);
			}
		}
		else
		{
			//报错
			printf("binary operator \"^\" error\n");
			exit(-1);
		}
		break;
	case O_BIT_LEFT:
		left = left->eval(left);
		if (left->type == number)
		{
			if (left->property.p_number->type == C_INT)
			{
				right = right->eval(right);
				if (right->type == number && right->property.p_number->type == C_INT)
				{
					int left_number, right_number;
					left_number = atoi(left->property.p_number->value);
					right_number = atoi(right->property.p_number->value);
					int result = left_number << right_number;
					ASTnode* node = malloc(sizeof(ASTnode));
					node->type = number;
					node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
					node->eval = cb_eval_number;
					node->property.p_number->type = C_INT;
					selflib_itoa(result, node->property.p_number->value);
					return node;
				}
				else
				{
					printf("bit-operator must be int\n");
					exit(-1);
				}
			}
			else
			{
				printf("bit-operator must be int\n");
				exit(-1);
			}
		}
		else
		{
			//报错
			printf("binary operator \"<<\" error\n");
			exit(-1);
		}
		break;
	case O_BIT_RIGHT:
		left = left->eval(left);
		if (left->type == number)
		{
			if (left->property.p_number->type == C_INT)
			{
				right = right->eval(right);
				if (right->type == number && right->property.p_number->type == C_INT)
				{
					int left_number, right_number;
					left_number = atoi(left->property.p_number->value);
					right_number = atoi(right->property.p_number->value);
					int result = left_number >> right_number;
					ASTnode* node = malloc(sizeof(ASTnode));
					node->type = number;
					node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
					node->eval = cb_eval_number;
					node->property.p_number->type = C_INT;
					selflib_itoa(result, node->property.p_number->value);
					return node;
				}
				else
				{
					printf("bit-operator must be int\n");
					exit(-1);
				}
			}
			else
			{
				printf("bit-operator must be int\n");
				exit(-1);
			}
		}
		else
		{
			//报错
			printf("binary operator \">>\" error\n");
			exit(-1);
		}
		break;
	case O_LOGIC_AND:
		left = left->eval(left);
		if (left->type == number)
		{
			if (left->property.p_number->type == C_INT)
			{
				right = right->eval(right);
				if (right->type == number && right->property.p_number->type == C_INT)
				{
					int left_number, right_number;
					left_number = atoi(left->property.p_number->value);
					right_number = atoi(right->property.p_number->value);
					int result = left_number && right_number;
					ASTnode* node = malloc(sizeof(ASTnode));
					node->type = number;
					node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
					node->eval = cb_eval_number;
					node->property.p_number->type = C_INT;
					selflib_itoa(result, node->property.p_number->value);
					return node;
				}
				else
				{
					printf("logic-operator must be int\n");
					exit(-1);
				}
			}
			else
			{
				printf("logic-operator must be int\n");
				exit(-1);
			}
		}
		else
		{
			//报错
			printf("binary operator \"&&\" error\n");
			exit(-1);
		}
		break;
	case O_LOGIC_OR:
		left = left->eval(left);
		if (left->type == number)
		{
			if (left->property.p_number->type == C_INT)
			{
				right = right->eval(right);
				if (right->type == number && right->property.p_number->type == C_INT)
				{
					int left_number, right_number;
					left_number = atoi(left->property.p_number->value);
					right_number = atoi(right->property.p_number->value);
					int result = left_number || right_number;
					ASTnode* node = malloc(sizeof(ASTnode));
					node->type = number;
					node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
					node->eval = cb_eval_number;
					node->property.p_number->type = C_INT;
					selflib_itoa(result, node->property.p_number->value);
					return node;
				}
				else
				{
					printf("logic-operator must be int\n");
					exit(-1);
				}
			}
			else
			{
				printf("logic-operator must be int\n");
				exit(-1);
			}
		}
		else
		{
			//报错
			printf("binary operator \"||\" error\n");
			exit(-1);
		}
		break;
	default:
		printf("undefined binary operator\n");
		exit(-1);
		break;
	}
	return NULL;
}

//一元运算符的执行函数
ASTnode* cb_eval_unaryop(void* arg)
{
	ASTnode* self = (ASTnode*)arg;
	ASTnode* child = self->property.p_unaryop->child;
	dian_en_token_id op = self->property.p_unaryop->op;

	switch (op)
	{
	case O_PLUS:
		child = child->eval(child);
		if (child->type == number)
		{
			return child;
		}
		else
		{
			printf("error used \"+\"\n");
			exit(-1);
		}
		break;
	case O_MINUS:
		child = child->eval(child);
		if (child->type == number)
		{

			if (child->property.p_number->type == C_INT)
			{
				int tmp;
				tmp = atoi(child->property.p_number->value);
				tmp = -tmp;
				selflib_itoa(tmp, child->property.p_number->value);
			}
			else
			{
				float tmp;
				tmp = atof(child->property.p_number->value);
				tmp = -tmp;
				selflib_ftoa(tmp, child->property.p_number->value);
			}
			return child;
		}
		else
		{
			printf("error used \"-\"\n");
			exit(-1);
		}
		break;
	case O_BIT_NOR:
		child = child->eval(child);
		if (child->type == number)
		{

			if (child->property.p_number->type == C_INT)
			{
				int tmp;
				tmp = atoi(child->property.p_number->value);
				tmp = ~tmp;
				selflib_itoa(tmp, child->property.p_number->value);
			}
			else
			{
				printf("~ operator must be int\n");
			}
			return child;
		}
		else
		{
			printf("error used \"~\"\n");
			exit(-1);
		}
		break;
	case O_LOGIC_NOR:
		child = child->eval(child);
		if (child->type == number)
		{

			if (child->property.p_number->type == C_INT)
			{
				int tmp;
				tmp = atoi(child->property.p_number->value);
				tmp = !tmp;
				selflib_itoa(tmp, child->property.p_number->value);
			}
			else
			{
				printf("! operator must be int\n");
			}
			return child;
		}
		else
		{
			printf("error used \"!\"\n");
			exit(-1);
		}
		break;
	default:
		printf("undefined unary operator\n");
		exit(-1);
		break;
	}

	return NULL;
}






/********************************************************
*                                                       *
*           语句执行结构方面的eval函数                  *
*                                                       *
********************************************************/
/*if_else节点结构*/
//typedef struct AST_node_if_statement
//{
//	dian_en_token_id type;
//	ASTnode* condition;
//	ASTnode* exec_statement;
//	struct AST_node_if_statement* other;
//}ASTnode_if_statement;

/*for节点结构*/
//typedef struct AST_node_for_statement
//{
//	ASTnode* statement_A;
//	ASTnode* statement_B;
//	ASTnode* statement_C;
//	ASTnode* exec_statement;
//	ASTnode* else_statement;
//}ASTnode_for_statement;

ASTnode* cb_eval_expression_list(void* arg)
{
	ASTnode* self = (ASTnode*)arg;
	return self;
}

ASTnode* cb_eval_statement(void* arg)
{
	ASTnode* self = (ASTnode*)arg;
	if (self->property.p_statement->type == empty_statement)
	{
		return NULL;
	}
	else if (self->property.p_statement->type == simple_statement)
	{
		return self->property.p_statement->child->eval(self->property.p_statement->child);
	}
	else if (self->property.p_statement->type == break_statement)
	{
		return self->property.p_statement->child->eval(self->property.p_statement->child);
	}
	else if (self->property.p_statement->type == return_statement)
	{
		return self->property.p_statement->child->eval(self->property.p_statement->child);
	}
	else
	{
		return self->property.p_statement->child->eval(self->property.p_statement->child);
	}
}

ASTnode* cb_eval_statement_list(void* arg)
{
	ASTnode* self = (ASTnode*)arg;
	ASTnode_statement_list* now_statement = self->property.p_statement_list;
	while (now_statement != NULL)
	{
		ASTnode* result = now_statement->statement->eval(now_statement->statement);
		if (result != NULL)
		{
			if (result->type == break_node || result->type == return_node)
			{
				return result;
			}

		}
		now_statement = now_statement->next_statement;
	}
	return NULL;
}

ASTnode* cb_eval_if_else(void* arg)
{
	ASTnode* self = (ASTnode*)arg;
	ASTnode_if_statement* tmp = self->property.p_if;
	int control = 0;
	if (tmp->type == K_IF)
	{
		int condition = 0;
		ASTnode* result = tmp->condition->eval(tmp->condition);

		if (result->type != number)
		{
			printf("error : if statements need number to judge\n");
			exit(-1);
		}
		if (result->property.p_number->type != C_INT)
		{
			printf("error : if statements need int number to judge\n");
			exit(-1);
		}
		condition = atoi(result->property.p_number->value);
		if (condition != 0)
		{
			control = 1;
			result = tmp->exec_statement->eval(tmp->exec_statement);
			if (result != NULL)
			{
				if (result->type == break_node || result->type == return_node)
				{
					return result;
				}
			}
		}
		if (tmp->other != NULL)
			tmp = tmp->other;
		else
			return NULL;
	}
	else
	{
		printf("error if type\n");
	}
	while (tmp->type == K_ELIF && control == 0)
	{
		int condition = 0;
		ASTnode* result = tmp->condition->eval(tmp->condition);

		if (result->type != number)
		{
			printf("error : if statements need number to judge\n");
			exit(-1);
		}
		if (result->property.p_number->type != C_INT)
		{
			printf("error : if statements need int number to judge\n");
			exit(-1);
		}
		condition = atoi(result->property.p_number->value);
		if (condition != 0)
		{
			control = 1;
			result = tmp->exec_statement->eval(tmp->exec_statement);
			if (result != NULL)
			{
				if (result->type == break_node || result->type == return_node)
				{
					return result;
				}
			}
		}
		if (tmp->other != NULL)
			tmp = tmp->other;
		else
			return NULL;
	}

	if (tmp->type == K_ELSE && control == 0)
	{
		ASTnode* result = tmp->exec_statement->eval(tmp->exec_statement);
		if (result != NULL)
		{
			if (result->type == break_node || result->type == return_node)
			{
				return result;
			}
		}
	}
	return NULL;
}

ASTnode* cb_eval_for_else(void* arg)
{
	ASTnode* self = (ASTnode*)arg;


	//A语句执行
	if (self->property.p_for->statement_A != NULL)
	{
		self->property.p_for->statement_A->eval(self->property.p_for->statement_A);
	}

	if (self->property.p_for->statement_B != NULL)
	{
		int control = 0;
		while (1)
		{
			ASTnode* condition = self->property.p_for->statement_B->eval(self->property.p_for->statement_B);
			if (condition->type != number)
			{
				printf("for statements need a number to judge\n");
				exit(-1);
			}
			if (condition->property.p_number->type != C_INT)
			{
				printf("for statements need a int number to judge\n");
				exit(-1);
			}
			control = atoi(condition->property.p_number->value);

			if (control == 0)
				break;

			ASTnode* result = self->property.p_for->exec_statement->eval(self->property.p_for->exec_statement);
			if (result != NULL)
			{
				if (result->type == break_node)
				{
					if (self->property.p_for->else_statement != NULL)
					{
						result = self->property.p_for->else_statement->eval(self->property.p_for->else_statement);
					}
					else
					{
						result = NULL;
					}
					if (result != NULL)
					{
						if (result->type == break_node || result->type == return_node)
						{
							return result;
						}
					}
					break;
				}
				else if (result->type == return_node)
				{
					return result;
				}
			}
			if (self->property.p_for->statement_C != NULL)
			{
				ASTnode* result = self->property.p_for->statement_C->eval(self->property.p_for->statement_C);
			}
		}


	}
	else
	{
		while (1)
		{
			ASTnode* result = self->property.p_for->exec_statement->eval(self->property.p_for->exec_statement);
			if (result != NULL)
			{
				if (result->type == break_node)
				{
					if (self->property.p_for->else_statement != NULL)
					{
						result = self->property.p_for->else_statement->eval(self->property.p_for->else_statement);
					}
					else
					{
						result = NULL;
					}
					if (result != NULL)
					{
						if (result->type == break_node || result->type == return_node)
						{
							return result;
						}
					}
					break;
				}
				else if (result->type == return_node)
				{
					return result;
				}
			}
			if (self->property.p_for->statement_C != NULL)
			{
				result = self->property.p_for->statement_C->eval(self->property.p_for->statement_C);
			}

		}
	}


	return NULL;
}

ASTnode* cb_eval_break(void* arg)
{
	ASTnode* self = (ASTnode*)arg;
	self->type = break_node;
	return self;
}




/********************************************************
*                                                       *
*               函数调用处理相关eval                    *
*                                                       *
********************************************************/



//函数参数列表
/*
typedef struct AST_node_function_parameter
{
	dian_en_token_id type;
	char id[MAX_ID_LENGTH];
	struct AST_node_function_parameter* next_param;
}ASTnode_func_param;
*/


/*
调用流程：cb_eval_func_call 调用 exec_func函数，exec_func函数执行代码，碰到return语句返回，调用cb_eval_return

cb_eval_return将返回一个return_node节点，其中包含有返回的expression_list（完全执行完毕）
该节点返回至函数exec_func，将其节点内容(expression_list)返回给cb_eval_func_call
cb_eval_func_call再进行返回值检查

在调用流程中，exec_func需要进行检查其参数类型是否对应，并且进行相应的参数赋值（如果该参数被重复定义则将曾经的值压入堆栈）
*/


ASTnode* cb_eval_return(void* arg)
{
	ASTnode* self = (ASTnode*)arg;
	return self;
}

ASTnode* cb_eval_func_call(void* arg)
{
	ASTnode* self = (ASTnode*)arg;
	ASTnode* node = malloc(sizeof(ASTnode));
	//预定义函数的调用
	if (!strcmp(self->property.p_func_call->id, "readi"))
	{
		int int_num = 0;
		fscanf(stdin, "%d", &int_num);
		node->type = number;
		node->eval = cb_eval_number;
		node->property.p_number = malloc(sizeof(ASTnode_number));
		node->property.p_number->type = C_INT;
		selflib_itoa(int_num, node->property.p_number->value);
		return node;
	}
	else if (!strcmp(self->property.p_func_call->id, "readf"))
	{
		float float_num = 0;
		fscanf(stdin, "%f", &float_num);
		if (is_float(float_num))
		{
			node->type = number;
			node->eval = cb_eval_number;
			node->property.p_number = malloc(sizeof(ASTnode_number));
			node->property.p_number->type = C_FLOAT;
			selflib_ftoa(float_num, node->property.p_number->value);
		}
		else
		{
			int int_num = (int)float_num;
			node->type = number;
			node->eval = cb_eval_number;
			node->property.p_number = malloc(sizeof(ASTnode_number));
			node->property.p_number->type = C_INT;
			selflib_itoa(int_num, node->property.p_number->value);
		}
		return node;
	}
	else if (!strcmp(self->property.p_func_call->id, "reads"))
	{
		char read_string[64] = { 0 };
		fscanf(stdin, "%s", read_string);
		node->type = string;
		node->eval = cb_eval_string;
		node->property.p_string = malloc(sizeof(ASTnode_string));
		memset(node->property.p_string->value, 0, MAX_SIZE_TOKEN_VALUE);
		strcpy(node->property.p_string->value, read_string);
		return node;
	}
	else if (!strcmp(self->property.p_func_call->id, "print"))
	{
		if (self->property.p_func_call->param->type != string)
		{
			printf("error call for func print\n");
			exit(-1);
		}
		explain_string(self->property.p_func_call->param->property.p_string->value);
		return NULL;
	}


	dian_env_func_node* func_node = get_func_node(self->property.p_func_call->id, get_top_env_stack());
	if (func_node == NULL)
	{
		printf("undefined func call\n");
		exit(-1);
	}

	//进行函数调用(一定要先确定值，再进行新的环境变量设定)
	//1.确定参数
	ASTnode* function = func_node->fun_def_pos;
	ASTnode* real_param = self->property.p_func_call->param;
	ASTnode_func_param* function_param = function->property.p_def_func->param_list;


	ASTnode_func_param* tmp_f = function_param;


	ASTnode* real_param_expression_list = (ASTnode*)malloc(sizeof(ASTnode));
	real_param_expression_list->eval = cb_eval_expression_list;
	real_param_expression_list->type = expression_list;
	real_param_expression_list->property.p_expression_list = malloc(sizeof(ASTnode_expression_list));
	real_param_expression_list->property.p_expression_list->expression = NULL;
	real_param_expression_list->property.p_expression_list->next_expression = NULL;

	int i = 1;
	while (tmp_f != NULL)
	{
		int p = i;
		ASTnode* expression = get_expr_from_exprlist(real_param, &p);
		if (expression == NULL)
		{
			printf("param lack\n");
			exit(-1);
		}
		if (tmp_f->type == T_FUNC)
		{
			if (expression->type != variable)
			{
				printf("error func pass\n");
				exit(-1);
			}
			expression->type = func_addr;
			char func_name[32] = { 0 };
			strcpy(func_name, expression->property.p_variable->variable_id);
			expression->property.p_addr = malloc(sizeof(ASTnode_addr));
			expression->property.p_addr->func_addr = get_func_node(func_name,get_top_env_stack());
			if (expression->property.p_addr->func_addr == NULL)
			{
				printf("error param\n");
				exit(-1);
			}	
		}
		else
		{
			expression = expression->eval(expression);
		}
		tmp_f = tmp_f->next_param;
		i++;
		create_expression_list(real_param_expression_list, expression);
	}
	ASTnode* expression = get_expr_from_exprlist(real_param, &i);
	if (expression != NULL)
	{
		printf("param number is not matched\n");
		exit(-1);
	}
	//确定参数是否一致


	//2.设定新的环境变量
	dian_env* func_env = malloc(sizeof(dian_env));
	dian_init_env(func_env);
	push_variable_env(*func_env);


	//3.设定形参为局部变量
	define_param(function_param, real_param_expression_list);


	//3.执行对应函数
	ASTnode* return_result = exec_func(function);


	if (return_result == NULL)
	{
		dian_env* ret_env = malloc(sizeof(dian_env));
		pop_variable_env(ret_env);
		return NULL;
	}


	//需要进行一些返回值类型的检测
	//ASTnode_func_typelist* return_list = func_node->fun_def_pos->property.p_def_func->ret_list;
	//exam_func_return(return_list, return_result);

	return_result = return_result->eval(return_result);



	dian_env* ret_env = malloc(sizeof(dian_env));
	pop_variable_env(ret_env);

	return return_result;
}



//重写
void define_param(ASTnode_func_param* function_param, ASTnode* real_param)
{
	ASTnode_func_param* tmp_f = function_param;
	int i = 1;
	while (tmp_f != NULL)
	{
		int p = i;
		ASTnode* expression = get_expr_from_exprlist(real_param, &p);
		if (tmp_f->type == T_FUNC)
		{
			if (expression->type != func_addr)
			{
				printf("error\n");
				exit(-1);
			}
			register_func_node(tmp_f->id, expression->property.p_addr->func_addr->fun_def_pos, get_top_env_stack());
		}
		else if(tmp_f->type == T_FLOAT || tmp_f->type == T_INT)
		{
			if (expression->type != number)
			{
				printf("error param\n");
				exit(-1);
			}
			register_variable_node(tmp_f->id, expression->property.p_number->type-6, 1, get_top_env_stack());
			modify_variable_node(tmp_f->id, expression->property.p_number->value, expression->property.p_number->type-6, 1, get_top_env_stack());

		}
		else if(tmp_f->type == T_STRING)
		{
			if (expression->type != string)
			{
				printf("error param\n");
				exit(-1);
			}
			register_variable_node(tmp_f->id, T_STRING, 1, get_top_env_stack());
			modify_variable_node(tmp_f->id, expression->property.p_string->value, T_STRING, 1, get_top_env_stack());
		}
		else
		{
			printf("error define of func param\n");
			exit(-1);
		}
		i++;
		tmp_f = tmp_f->next_param;
	}
}


//函数检查（未完成）
int exam_func_point()
{
	return 1;
}

//返回值检查（未完成）
int exam_func_return()
{
	return 1;
}

ASTnode* exec_func(ASTnode* func_def_pos)
{
	ASTnode* function = func_def_pos;

	ASTnode* result = function->property.p_def_func->exec_statement->eval(function->property.p_def_func->exec_statement);

	if(result!= NULL)
	result = result->property.p_return->return_expr;

	return result;
}


//print函数
void explain_string(char* param_string)
{
	char print_string[128];
	memset(print_string, 0, 128);
	char* tmp = param_string;
	char string_to_translate[64];
	char string_after_translate[64];
	char string_to_print[128];
	while (1)
	{
		char* lbracket = strstr(tmp, "{");
		if (lbracket == NULL)
		{
			strcat(print_string, tmp);
			break;
		}
		memset(string_to_print, 0, 128);
		int length_orign = lbracket - tmp;
		strncpy(string_to_print, tmp, length_orign);
		strcat(print_string, string_to_print);
		tmp = lbracket + 1;
		char* rbracket = strstr(tmp, "}");
		if(rbracket == NULL)
		{
			printf("print string error: no matched \"}\"\n");
			exit(-1);
		}
		tmp = rbracket + 1;
		int length_trans = rbracket - lbracket;
		memset(string_to_translate, 0, 64);
		strncpy(string_to_translate, lbracket + 1, length_trans - 1);
		memset(string_after_translate, 0, 64);
		translate_string(string_to_translate, string_after_translate);
		strcat(print_string, string_after_translate);
	}
	fprintf(stdout, "%s", print_string);
	return;
}


void translate_string(char *string_trans,char return_string[])
{
	memset(return_string, 0, 64);
	dian_st_lexer* tmp_lexer = malloc(sizeof(dian_st_lexer));
	dian_st_parser* tmp_parser = malloc(sizeof(dian_st_parser));
	dian_init_lexer(string_trans, tmp_lexer);
	dian_init_parser(tmp_parser, tmp_lexer);
	tmp_parser->p_current_token = tmp_lexer->cb_get_next_token(tmp_lexer);
	ASTnode* node = tmp_parser->cb_expression(tmp_parser);
	if (node->type == variable)
	{
		dian_env_var_node* tmp_variable = get_variable_node(node->property.p_variable->variable_id, get_top_env_stack(),0);
		if (tmp_variable == NULL)
		{
			printf("{该变量不存在}\n");
			exit(-1);
		}
		else if (tmp_variable->T_type == T_FLOAT_ARRAY || tmp_variable->T_type == T_INT_ARRAY)
		{
			int length = tmp_variable->length;
			strcat(return_string, "[");
			for (int i = 0; i < length; i++)
			{
				ASTnode* tmp_number_node = get_variable_value(node->property.p_variable->variable_id, i, get_top_env_stack());
				if (tmp_number_node->property.p_number->type == C_FLOAT)
				{
					float number = atof(tmp_number_node->property.p_number->value);
					char tmp_string[32];
					memset(tmp_string, 0, 32);
					sprintf(tmp_string, "%.5f", number);
					int length = strlen(tmp_string);
					for (int i = length - 1; i >= 0; i--)
					{
						if (tmp_string[i] == '0')
						{
							tmp_string[i] = '\0';
						}
						else
						{
							break;
						}
					}
					strcat(return_string, tmp_string);
					strcat(return_string, " ");
				}
				else if (tmp_number_node->property.p_number->type == C_INT)
				{
					strcat(return_string, tmp_number_node->property.p_number->value);
					strcat(return_string, " ");
				}
			}
			strcat(return_string, "]");
		}
		else if (tmp_variable->T_type == T_STRING_ARRAY)
		{
			int length = tmp_variable->length;
			strcat(return_string, "[");
			for (int i = 0; i < length; i++)
			{
				ASTnode* node = get_variable_value(tmp_variable->id, i, get_top_env_stack());
				strcat(return_string, node->property.p_string->value);
				strcat(return_string, " ");
			}
			strcat(return_string, "]");
		}
		else if (tmp_variable->T_type == T_FLOAT)
		{
			node = node->eval(node);
			float number = atof(node->property.p_number->value);
			char temp_string[32];
			sprintf(temp_string, "%.5f", number);
			int length = strlen(temp_string);
			for (int i = length-1; i >= 0; i--)
			{
				if (temp_string[i] == '0')
				{
					temp_string[i] = '\0';
				}
				else
				{
					break;
				}
			}
			strcpy(return_string, temp_string);
		}
		else
		{
			node = node->eval(node);
			if (node->type == number)
			{
				strcpy(return_string, node->property.p_number->value);
			}
			else if (node->type == string)
			{
				strcpy(return_string, node->property.p_string->value);
			}
		}
	}
	else
	{
		node = node->eval(node);
		if (node->type == number)
		{
			if (node->property.p_number->type == C_FLOAT)
			{
				float number = atof(node->property.p_number->value);
				char temp_string[32];
				memset(temp_string, 0, 32);
				sprintf(temp_string, "%.5f", number);
				int length = strlen(temp_string);
				for (int i = length - 1; i >= 0; i--)
				{
					if (temp_string[i] == '0')
					{
						temp_string[i] = '\0';
					}
					else
					{
						break;
					}
				}
				strcpy(return_string, temp_string);
			}
			else
			{
				strcpy(return_string, node->property.p_number->value);
			}
		}
		else if (node->type == string)
		{
			strcpy(return_string, node->property.p_string->value);
		}
	}

	return;
}