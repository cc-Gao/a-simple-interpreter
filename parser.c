#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "env.h"
#include "lexer.h"
#include "lib.h"
#include "parser.h"
#include "eval.h"

/******************************************************************************************\
*                                  parser����                                              *
*   �ò��ֻ���Ϊparser�ṹ�Ļص����������ڹ���AST�﷨����ÿ���ڵ�Ϊһ��ASTnode���͵Ľṹ   *
*              ���ڲ�ͬ���͵Ľڵ㣬ASTnode�е�property����ָ����в�ͬ                     *
*           ��ͬ�Ľ���Ӧ�Ų�ͬ�Ļص���eval�����������������ı���ִ�������﷨��           *
*                                                                                          *
\******************************************************************************************/

//eat�����������ǰ��token
int parser_callback_eat(void* arg, dian_en_token_id type)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	if (self->p_current_token->type == type)
	{
		free(self->p_current_token);
		self->p_current_token =self->lexer->cb_get_next_token(self->lexer);
		return 0;
	}
	else
	{
		printf("invaild syntax\n");
		exit(-1);
	}
}

//�������򷵻�����AST��
ASTnode* parser_program_AST(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode* node = self->cb_statement(self);
	g_def_fun_limit = 0;
	ASTnode* statement_list = create_statement_list(node);
	while (self->p_current_token->type != END)
	{
		add_statement_list(self->cb_statement(self), statement_list);
		g_def_fun_limit = 0;
	}

	return statement_list;
}


//�ⲿmallocһ��expression-list�ṹ���ú������Խ�expression��������
ASTnode* create_expression_list(ASTnode* expression_list, ASTnode* expression)
{
	if (expression == NULL)
	{
		return NULL;
	}
	expression_list->eval = cb_eval_expression_list;
	ASTnode_expression_list* tmp = expression_list->property.p_expression_list;
	if (tmp->expression == NULL)
	{
		expression_list->property.p_expression_list->expression = expression;
		expression_list->property.p_expression_list->next_expression = NULL;
	}
	else
	{
		while (tmp->next_expression != NULL)
		{
			tmp = tmp->next_expression;
		}
		tmp->next_expression = (ASTnode_expression_list*)malloc(sizeof(ASTnode_expression_list));
		tmp = tmp->next_expression;

		tmp->expression = expression;
		tmp->next_expression = NULL;
	}
	return NULL;
}

/****************************************
*                                       *
*          ���ֻ����ڵ����             *
*                                       *
****************************************/

ASTnode* create_unaryop(dian_en_token_id op, ASTnode* child)
{
	ASTnode* node = malloc(sizeof(ASTnode));
	node->type = unaryop;
	node->property.p_unaryop = (ASTnode_unaryop*)malloc(sizeof(ASTnode_unaryop));
	node->property.p_unaryop->op = op;
	node->property.p_unaryop->child = child;
	return node;
}

ASTnode* create_binop(ASTnode* left, dian_en_token_id op,ASTnode* right)
{
	ASTnode* node = malloc(sizeof(ASTnode));
	node->type = binop;
	node->eval = cb_eval_binop;
	node->property.p_binop = (ASTnode_binop*)malloc(sizeof(ASTnode_binop));
	node->property.p_binop->op = op;
	node->property.p_binop->left = left;
	node->property.p_binop->right = right;
	return node;
}

ASTnode* create_assignop(ASTnode* left, ASTnode* right)
{
	ASTnode* node = malloc(sizeof(ASTnode));
	node->type = assignop;
	node->eval = cb_eval_assignop;
	node->property.p_assignop = (ASTnode_assign*)malloc(sizeof(ASTnode_assign));
	node->property.p_assignop->left = left;
	node->property.p_assignop->right = right;
	return node;
}

ASTnode* create_arraynode(char* id, ASTnode* expression)
{
	ASTnode* node = malloc(sizeof(ASTnode));
	node->type = array_node;
	node->eval = cb_eval_array;
	node->property.p_array = (ASTnode_array*)malloc(sizeof(ASTnode_array));
	strcpy(node->property.p_array->array_id, id);
	node->property.p_array->expression_start = expression;
	node->property.p_array->expression_end = NULL;
	return node;
}

ASTnode* create_funcall(char* id, ASTnode* expression_list)
{
	ASTnode* node = malloc(sizeof(ASTnode));
	node->type = func_call;
	node->eval = cb_eval_func_call;
	node->property.p_func_call = malloc(sizeof(ASTnode_func_call));
	strcpy(node->property.p_func_call->id, id);
	if (expression_list == NULL)
	{
		node->property.p_func_call->param = NULL;
	}
	node->property.p_func_call->param = expression_list;
	
	return node;
}


/****************************************
*                                       *
*          expression����               *
*                                       *
****************************************/


ASTnode* parser_callback_expression(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode* node = self->cb_term0(self);

	if(self->p_current_token->type == COMMA)
	{
		ASTnode* expr_list = (ASTnode*)malloc(sizeof(ASTnode));
		expr_list->type = expression_list;
		expr_list->property.p_expression_list = (ASTnode_expression_list*)malloc(sizeof(ASTnode_expression_list));
		expr_list->property.p_expression_list->expression = NULL;
		expr_list->property.p_expression_list->next_expression = NULL;
		create_expression_list(expr_list, node);

		while (self->p_current_token->type == COMMA)
		{
			self->cb_eat(self, COMMA);
			create_expression_list(expr_list, self->cb_term0(self));
		}
		return expr_list;
	}
	return node;
}

//�߼�����
ASTnode* parser_callback_term0(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode* node = self->cb_term1(self);

	while (self->p_current_token->type == O_LOGIC_AND \
		|| self->p_current_token->type == O_LOGIC_OR)
	{
		dian_en_token_id tmp_type = self->p_current_token->type;
		if (tmp_type == O_LOGIC_AND)
		{
			self->cb_eat(self, O_LOGIC_AND);
			node = create_binop(node, tmp_type, self->cb_term1(self));
		}
		else if (tmp_type == O_LOGIC_OR)
		{
			self->cb_eat(self, O_LOGIC_OR);
			node = create_binop(node, tmp_type, self->cb_term1(self));
		}
	}
	return node;
}

//�Ƚ�����
ASTnode* parser_callback_term1(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode* node = self->cb_term2(self);

	/*
	O_GREAT,
	O_LESS,
	O_GREAT_EQUAL,
	O_LESS_EQUAL,
	O_EQUAL,
	*/

	while(self->p_current_token->type == O_GREAT		\
		|| self->p_current_token->type == O_LESS		\
		|| self->p_current_token->type == O_GREAT_EQUAL \
		|| self->p_current_token->type == O_LESS_EQUAL	\
		|| self->p_current_token->type == O_EQUAL)
	{
		dian_en_token_id tmp_type = self->p_current_token->type;
		if (tmp_type == O_GREAT)
		{
			self->cb_eat(self, O_GREAT);
		}
		else if (tmp_type == O_LESS)
		{
			self->cb_eat(self, O_LESS);
		}
		else if (tmp_type == O_GREAT_EQUAL)
		{
			self->cb_eat(self, O_GREAT_EQUAL);
		}
		else if (tmp_type == O_LESS_EQUAL)
		{
			self->cb_eat(self, O_LESS_EQUAL);
		}
		else if (tmp_type == O_EQUAL)
		{
			self->cb_eat(self, O_EQUAL);
		}

		node = create_binop(node, tmp_type, self->cb_term2(self));
	}
	return node;
}

//λ����
ASTnode* parser_callback_term2(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode* node = self->cb_term3(self);

	/*
	O_BIT_AND,
	O_BIT_OR,
	O_BIT_XNOR,
	O_BIT_LEFT,
	O_BIT_RIGHT,
	*/

	while(self->p_current_token->type == O_BIT_AND   \
		|| self->p_current_token->type == O_BIT_OR   \
		|| self->p_current_token->type == O_BIT_XNOR \
		|| self->p_current_token->type == O_BIT_LEFT \
		|| self->p_current_token->type == O_BIT_RIGHT)
	{
		dian_en_token_id tmp_type = self->p_current_token->type;
		if (tmp_type == O_BIT_AND)
		{
			self->cb_eat(self, O_BIT_AND);
		}
		else if (tmp_type == O_BIT_OR)
		{
			self->cb_eat(self, O_BIT_OR);
		}
		else if (tmp_type == O_BIT_XNOR)
		{
			self->cb_eat(self, O_BIT_XNOR);
		}
		else if (tmp_type == O_BIT_LEFT)
		{
			self->cb_eat(self, O_BIT_LEFT);
		}
		else if (tmp_type == O_BIT_RIGHT)
		{
			self->cb_eat(self, O_BIT_RIGHT);
		}

		node = create_binop(node, tmp_type, self->cb_term3(self));
	}
	return node;
}

//�Ӽ�����
ASTnode* parser_callback_term3(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode* node = self->cb_term4(self);

	/*
	O_PLUS,
	O_MINUS,
	*/
	while(self->p_current_token->type == O_PLUS || self->p_current_token->type == O_MINUS)
	{
		dian_en_token_id tmp_type = self->p_current_token->type;
		if (tmp_type == O_PLUS)
		{
			self->cb_eat(self, O_PLUS);
		}
		else if (tmp_type == O_MINUS)
		{
			self->cb_eat(self, O_MINUS);
		}
		
		node = create_binop(node, tmp_type, self->cb_term4(self));
	}
	return node;
}

//�˳�����
ASTnode* parser_callback_term4(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode* node = self->cb_term5(self);

	/*
	O_MUL,
	O_F_DIV,
	O_I_DIV,
	O_REM,
	*/
	while(self->p_current_token->type == O_MUL		\
		|| self->p_current_token->type == O_F_DIV	\
		|| self->p_current_token->type == O_I_DIV	\
		|| self->p_current_token->type == O_REM)
	{
		dian_en_token_id tmp_type = self->p_current_token->type;
		if (tmp_type == O_MUL)
		{
			self->cb_eat(self, O_MUL);
		}
		else if (tmp_type == O_F_DIV)
		{
			self->cb_eat(self, O_F_DIV);
		}
		else if (tmp_type == O_I_DIV)
		{
			self->cb_eat(self, O_I_DIV);
		}
		else if (tmp_type == O_REM)
		{
			self->cb_eat(self, O_REM);
		}
		node = create_binop(node, tmp_type, self->cb_term5(self));
	}
	return node;
}

//������
ASTnode* parser_callback_term5(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode* node = self->cb_term6(self);

	/*
		O_POWER,
	*/

	while(self->p_current_token->type == O_POWER)
	{
		dian_en_token_id tmp_type = self->p_current_token->type;
		self->cb_eat(self, O_POWER);
		node = create_binop(node, tmp_type, self->cb_term6(self));
	}
	return node;
}

//ʶ���ַ��������Լ������������ڵ�Ľ�������֮�У�
ASTnode* parser_callback_term6(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode* node = NULL;

	dian_en_token_id tmp_type = self->p_current_token->type;

	if (tmp_type == O_PLUS)
	{
		self->cb_eat(self, O_PLUS);
		node = create_unaryop(tmp_type, self->cb_term6(self));
			return node;
	}
	else if (tmp_type == O_MINUS)
	{
		self->cb_eat(self, O_MINUS);
		node = create_unaryop(tmp_type, self->cb_term6(self));
		return node;
	}
	else if (tmp_type == O_LOGIC_NOR)
	{
		self->cb_eat(self, O_LOGIC_NOR);
		node = create_unaryop(tmp_type, self->cb_term6(self));
		return node;
	}
	else if (tmp_type == O_BIT_NOR)
	{
		self->cb_eat(self, O_BIT_NOR);
		node = create_unaryop(tmp_type, self->cb_term6(self));
		return node;
	}
	else if (tmp_type == ID)
	{
		dian_st_token token = *self->p_current_token;
		self->cb_eat(self, ID);
		if (self->p_current_token->type == LM_BRACE)
		{
			self->cb_eat(self, LM_BRACE);
			node = create_arraynode(token.value, self->cb_simple_statement(self));
			if (self->p_current_token->type == COLON)
			{
				self->cb_eat(self, COLON);
				node->type = array_node_slice;
				node->property.p_array->expression_end = self->cb_simple_statement(self);
			}
			self->cb_eat(self, RM_BRACE);
		}
		else if (self->p_current_token->type == LS_BRACE)
		{
			//��������
			self->cb_eat(self, LS_BRACE);
			if (self->p_current_token->type != RS_BRACE)
			{
				node = create_funcall(token.value, self->cb_expression(self));
			}
			else
			{
				node = create_funcall(token.value, NULL);
			}
				self->cb_eat(self, RS_BRACE);
			return node;
		}

		else
		{
			node = malloc(sizeof(ASTnode));
			node->type = variable;
			node->eval = cb_eval_variable;
			node->property.p_variable = (ASTnode_variable*)malloc(sizeof(ASTnode_variable));
			strcpy(node->property.p_variable->variable_id, token.value);
		}
		return node;
	}
	
	else if (tmp_type == C_STRING)
	{
		dian_st_token token = *self->p_current_token;
		self->cb_eat(self, C_STRING);
		node = malloc(sizeof(ASTnode));
		node->type = string;
		node->eval = cb_eval_string;
		node->property.p_string = (ASTnode_string*)malloc(sizeof(ASTnode_string));
		memset(node->property.p_string->value, 0, MAX_SIZE_TOKEN_VALUE);
		strcpy(node->property.p_string->value, token.value);
		return node;
	}
	else if (tmp_type == C_FLOAT)
	{
		dian_st_token token = *self->p_current_token;
		self->cb_eat(self, C_FLOAT);
		node = malloc(sizeof(ASTnode));
		node->type = number;
		node->eval = cb_eval_number;
		node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
		node->property.p_number->type = C_FLOAT;
		strcpy(node->property.p_number->value,token.value);
		return node;
	}

	else if (tmp_type == C_INT)
	{
		dian_st_token token = *self->p_current_token;
		self->cb_eat(self, C_INT);
		node = malloc(sizeof(ASTnode));
		node->type = number;
		node->eval = cb_eval_number;
		node->property.p_number = (ASTnode_number*)malloc(sizeof(ASTnode_number));
		node->property.p_number->type = C_INT;
		strcpy(node->property.p_number->value,token.value);
		return node;
	}

	else if (tmp_type == LS_BRACE)
	{
		self->cb_eat(self, LS_BRACE);
		node = self->cb_simple_statement(self);
		self->cb_eat(self, RS_BRACE);
		return node;
	}

	else
	{
		printf("syntax error\n");
		exit(-1);
	}

	return node;
}


/****************************************
*                                       *
*           statement_list����          *
*                                       *
****************************************/

ASTnode* create_statement_list(ASTnode* statement)
{
	ASTnode* p_statement_list = (ASTnode*)malloc(sizeof(ASTnode));
	p_statement_list->type = statement_list;
	p_statement_list->eval = cb_eval_statement_list;
	p_statement_list->property.p_statement_list = (ASTnode_statement_list*)malloc(sizeof(ASTnode_statement_list));
	p_statement_list->property.p_statement_list->statement = statement;
	p_statement_list->property.p_statement_list->next_statement = NULL;
	return p_statement_list;
}

void add_statement_list(ASTnode* statement, ASTnode* statement_list)
{
	ASTnode_statement_list* tmp = statement_list->property.p_statement_list;
	while (tmp->next_statement != NULL)
	{
		tmp = tmp->next_statement;
	}
	tmp->next_statement = (ASTnode_statement_list*)malloc(sizeof(ASTnode_statement_list));
	tmp = tmp->next_statement;
	tmp->statement = statement;
	tmp->next_statement = NULL;
}

ASTnode* parser_callback_statement_list(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode* node = self->cb_statement(self);
	ASTnode* statement_list = create_statement_list(node);

	while (self->p_current_token->type != RB_BRACE)
	{
		add_statement_list(self->cb_statement(self), statement_list);
	}
	return statement_list;
}


/****************************************
*                                       *
*           statement����               *
*                                       *
****************************************/


ASTnode* parser_callback_statement(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode_statement_type statement_type;
	dian_en_token_id tmp_type = self->p_current_token->type;
	ASTnode* node = NULL;
	
	if (tmp_type == SEMI)
	{
		node = self->cb_empty_statement(self);
		statement_type = empty_statement;
		self->cb_eat(self, SEMI);
	}
	else if (tmp_type == K_BREAK)
	{
		node = self->cb_break_statement(self);
		statement_type = break_statement;
		self->cb_eat(self, SEMI);
	}
	else if (tmp_type == K_ASSIGN)
	{
		node = self->cb_assign_statement(self);
		statement_type = assign_statement;
		self->cb_eat(self, SEMI);
	}
	else if (tmp_type == K_RETURN)
	{
		node = self->cb_return_statement(self);
		statement_type = return_statement;
		self->cb_eat(self, SEMI);
	}
	else if (tmp_type == K_FUNC)
	{
		if (g_def_fun_limit != 0)
		{
			printf("here can not def a func\n");
			exit(-1);
		}
		node = self->cb_func_def_statement(self);
		statement_type = func_def_statement;
	}
	else if (tmp_type == T_INT || tmp_type == T_FLOAT || tmp_type == T_STRING)
	{
		node = self->cb_delaration_statement(self);
		statement_type = def_statement;
		self->cb_eat(self, SEMI);
	}
	else if (tmp_type == K_IF)
	{
		g_def_fun_limit = 1;
		node = self->cb_if_statement(self);
		statement_type = if_statement;
	}
	else if (tmp_type == K_FOR)
	{
		g_def_fun_limit = 1;
		node = self->cb_for_statement(self);
		statement_type = for_statement;
	}
	else if (tmp_type == LB_BRACE)
	{
		g_def_fun_limit = 1;
		node = self->cb_compound_statement(self);
		statement_type = compound_statement;
	}
	else
	{
		node = self->cb_simple_statement(self);
		statement_type = simple_statement;
		self->cb_eat(self, SEMI);
	}
	return node;
}


/****************************************
*                                       *
*        break_statement����            *
*                                       *
****************************************/

ASTnode* parser_callback_break_statement(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode* node = (ASTnode*)malloc(sizeof(ASTnode));
	if (self->p_current_token->type == K_BREAK)
	{
		self->cb_eat(self, K_BREAK);
		node->type = break_node;
		node->eval = cb_eval_break;
		node->property.p_break = malloc(sizeof(ASTnode_break));
		node->property.p_break->verification = NULL;
	}
	return node;
}

/****************************************
*                                       *
*       return_statement����            *
*                                       *
****************************************/

ASTnode* parser_callback_return_statement(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode* node = (ASTnode*)malloc(sizeof(ASTnode));
	if (self->p_current_token->type == K_RETURN)
	{
		self->cb_eat(self, K_RETURN);
		node->type = return_node;
		node->eval = cb_eval_return;
		node->property.p_return = malloc(sizeof(ASTnode_return));
		if(self->p_current_token->type != SEMI)
		node->property.p_return->return_expr = self->cb_expression(self);
		else
		{
			node->property.p_return->return_expr = NULL;
		}
	}
	return node;
}


/****************************************
*                                       *
*       assign_statement����            *
*                                       *
****************************************/

ASTnode* parser_callback_assign_statement(void* arg) 
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	self->cb_eat(self, K_ASSIGN);
	ASTnode* node = (ASTnode*)malloc(sizeof(ASTnode));
	node->type = def_assign;
	node->eval = cb_eval_def_assign;
	node->property.p_def_assign = (ASTnode_def_assign*)malloc(sizeof(ASTnode_def_assign));
	if (self->p_current_token->type != ID)
	{
		printf("assign statement must be one variable\n");
		exit(-1);
	}
	strcpy(node->property.p_def_assign->variable_id, self->p_current_token->value);
	self->cb_eat(self, ID);
	if (self->p_current_token->type != O_ASSIGN)
	{
		printf("assign statement must be only one variable\n");
		exit(-1);
	}
	self->cb_eat(self, O_ASSIGN);
	node->property.p_def_assign->value_expression = self->cb_expression(self);
	return node;
}


/****************************************
*                                       *
*        empty_statement����            *
*                                       *
****************************************/

ASTnode* parser_callback_empty_statement(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode* node = (ASTnode*)malloc(sizeof(ASTnode));
	if (self->p_current_token->type == SEMI)
	{
		//�ô�����Ҫeat
		//self->cb_eat(&g_parser, SEMI);
		node->type = empty;
	}
	else
	{
		//����
	}
	return node;
}

/****************************************
*                                       *
*       simple_statement����            *
*                                       *
****************************************/

ASTnode* parser_callback_simple_statement(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode* node = self->cb_expression(self);
	
	while (	self->p_current_token->type == O_F_DIV_ASSIGN	|| self->p_current_token->type == O_PLUS_ASSIGN		||\
			self->p_current_token->type == O_MINUS_ASSIGN	|| self->p_current_token->type == O_MUL_ASSIGN		||\
			self->p_current_token->type == O_REM_ASSIGN		|| self->p_current_token->type == O_I_DIV_ASSIGN	||\
			self->p_current_token->type == O_ASSIGN)
	{
		if (self->p_current_token->type == O_ASSIGN)
		{
			self->cb_eat(self, O_ASSIGN);
			node = create_assignop(node, self->cb_expression(self));
		}
		else if (self->p_current_token->type == O_PLUS_ASSIGN)
		{
			self->cb_eat(self, O_PLUS_ASSIGN);
			node = create_assignop(node, create_binop(node, O_PLUS, self->cb_expression(self)));
		}
		else if (self->p_current_token->type == O_MINUS_ASSIGN)
		{
			self->cb_eat(self, O_MINUS_ASSIGN);
			node = create_assignop(node, create_binop(node, O_MINUS, self->cb_expression(self)));
		}
		else if (self->p_current_token->type == O_MUL_ASSIGN)
		{
			self->cb_eat(self, O_MUL_ASSIGN);
			node = create_assignop(node, create_binop(node, O_MUL, self->cb_expression(self)));
		}
		else if (self->p_current_token->type == O_REM_ASSIGN)
		{
			self->cb_eat(self, O_REM_ASSIGN);
			node = create_assignop(node, create_binop(node, O_REM, self->cb_expression(self)));
		}
		else if (self->p_current_token->type == O_I_DIV_ASSIGN)
		{
			self->cb_eat(self, O_I_DIV_ASSIGN);
			node = create_assignop(node, create_binop(node, O_I_DIV, self->cb_expression(self)));
		}
		else if (self->p_current_token->type == O_F_DIV_ASSIGN)
		{
			self->cb_eat(self, O_F_DIV_ASSIGN);
			node = create_assignop(node, create_binop(node, O_F_DIV, self->cb_expression(self)));
		}

	}
	return node;
}

/****************************************
*                                       *
*       declaration_statement����       *
*                                       *
****************************************/

//�����������
ASTnode* parser_callback_delaration_statement(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode* node = (ASTnode*)malloc(sizeof(ASTnode));
	node->type = def_var;
	node->eval = cb_eval_def_var;
	node->property.p_def_var = (ASTnode_def_var*)malloc(sizeof(ASTnode_def_var));
	node->property.p_def_var->assign = NULL;
	node->property.p_def_var->expression = NULL;
	dian_en_token_id tmp_type = self->p_current_token->type;
	if (tmp_type == T_INT || tmp_type == T_FLOAT || tmp_type == T_STRING)
	{
		node->property.p_def_var->type = tmp_type;
		self->cb_eat(self, tmp_type);
		node->property.p_def_var->expression = self->cb_expression(self);
		if (self->p_current_token->type == O_ASSIGN)
		{
			self->cb_eat(self, O_ASSIGN);
			node->property.p_def_var->assign = self->cb_expression(self);
		}
	}
	else
	{
		printf("error deifine statements\n");
		exit(-1);
	}
	return node;
}

//����������䣨��Ҫ�޸ģ�
ASTnode* parser_callback_function_delaration_statement(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode* node = (ASTnode*)malloc(sizeof(ASTnode));
	node->type = def_func;
	node->eval = cb_eval_def_func;
	node->property.p_def_func = (ASTnode_def_func*)malloc(sizeof(ASTnode_def_func));
	node->property.p_def_func->param_list = NULL;
	node->property.p_def_func->ret_list = NULL;
	if (self->p_current_token->type == K_FUNC)
	{
		self->cb_eat(self, K_FUNC);
		if (self->p_current_token->type == ID)
		{
			strcpy(node->property.p_def_func->name, self->p_current_token->value);
			self->cb_eat(self, ID);
			self->cb_eat(self, LS_BRACE);
			dian_en_token_id tmp_type = self->p_current_token->type;

			//�޲���
			if (tmp_type == RS_BRACE)
			{
				node->property.p_def_func->param_list = NULL;
			}
			//��ɲ����ṹ�Ľ���
			while (self->p_current_token->type != RS_BRACE)
			{
				ASTnode_func_param* tmp = node->property.p_def_func->param_list;
				if (tmp_type == T_INT || tmp_type == T_FLOAT || tmp_type == T_STRING)
				{
					if (tmp == NULL)
					{
						node->property.p_def_func->param_list = (ASTnode_func_param*)malloc(sizeof(ASTnode_func_param));
						tmp = node->property.p_def_func->param_list;
						tmp->type = tmp_type;
						self->cb_eat(self, tmp_type);
						tmp_type = self->p_current_token->type;
					}
					else
					{
						while (tmp->next_param != NULL)
						{
							tmp = tmp->next_param;
						}
						tmp->next_param = (ASTnode_func_param*)malloc(sizeof(ASTnode_func_param));
						tmp = tmp->next_param;
						tmp->type = tmp_type;
						self->cb_eat(self, tmp_type);
						tmp_type = self->p_current_token->type;
					}
					if (self->p_current_token->type == ID)
					{
						strcpy(tmp->id, self->p_current_token->value);
						self->cb_eat(self, ID);
						tmp_type = self->p_current_token->type;
					}
					else
					{
						printf("�����������\n");
						exit(-1);
					}
					if (self->p_current_token->type == COMMA)
					{
						self->cb_eat(self, COMMA);
						tmp_type = self->p_current_token->type;
						if (tmp_type == RS_BRACE)
						{
							printf("�����������\",\"����Ҫ�������\n");
							exit(-1);
						}
					}
				}
				//������Ϊ�������ݵĶ���
				else if (tmp_type == K_FUNC)
				{
					self->cb_eat(self, K_FUNC);
					if (tmp == NULL)
					{
						node->property.p_def_func->param_list = (ASTnode_func_param*)malloc(sizeof(ASTnode_func_param));
						tmp = node->property.p_def_func->param_list;
						tmp->type = T_FUNC;
						if (self->p_current_token->type != ID)
						{
							printf("function define error\n");
							exit(-1);
						}
						strcpy(tmp->id, self->p_current_token->value);
						self->cb_eat(self, ID);
						tmp->extra_message = parser_function_pointer(self);
					}
					else
					{
						while (tmp->next_param != NULL)
						{
							tmp = tmp->next_param;
						}
						tmp->next_param = (ASTnode_func_param*)malloc(sizeof(ASTnode_func_param));
						tmp = tmp->next_param;
						tmp->next_param = NULL;
						tmp->type = T_FUNC;
						if (self->p_current_token->type != ID)
						{
							printf("function define error\n");
							exit(-1);
						}
						strcpy(tmp->id, self->p_current_token->value);
						self->cb_eat(self, ID);
						tmp->extra_message = parser_function_pointer(self);
					}
					
				}
				else 
				{
					printf("�����������\n");
					exit(-1);
				}
			}

			self->cb_eat(self, RS_BRACE);
			tmp_type = self->p_current_token->type;

			//�޷���ֵ
			if(tmp_type == LB_BRACE)
			{ 
				node->property.p_def_func->ret_list = NULL;
				node->property.p_def_func->exec_statement = self->cb_compound_statement(self);
				return node;
			}
			
			//�з���ֵ
			while (tmp_type != LB_BRACE)
			{
				ASTnode_func_typelist* tmp = node->property.p_def_func->ret_list;
				if (tmp_type == T_INT || tmp_type == T_FLOAT || tmp_type == T_STRING)
				{
					if (tmp == NULL)
					{
						node->property.p_def_func->ret_list = (ASTnode_func_typelist*)malloc(sizeof(ASTnode_func_typelist));
						tmp = node->property.p_def_func->ret_list;
						tmp->type = tmp_type;
						self->cb_eat(self, tmp_type);
						tmp_type = self->p_current_token->type;
					}
					else
					{
						while (tmp->next_type != NULL)
						{
							tmp = tmp->next_type;
						}
						tmp->next_type = (ASTnode_func_typelist*)malloc(sizeof(ASTnode_func_typelist));
						tmp = tmp->next_type;
						tmp->type = tmp_type;
						self->cb_eat(self, tmp_type);
						tmp_type = self->p_current_token->type;
					}
				}
				else
				{
					printf("����ķ���ֵ����\n");
					exit(-1);
					//����
				}

				if (self->p_current_token->type != COMMA && self->p_current_token->type != LB_BRACE)
				{
					printf("����ķ���ֵ����\n");
					exit(-1);
					//����
				}
				if (self->p_current_token->type == COMMA)
				{
					self->cb_eat(self, COMMA);
					tmp_type = self->p_current_token->type;
					if (tmp->type == LB_BRACE)
					{
						printf("����ķ���ֵ����\",\"����Ҫ���෵��ֵ\n");
						exit(-1);
					}
				}
			}



			if (self->p_current_token->type == LB_BRACE)
			{
				node->property.p_def_func->exec_statement = self->cb_compound_statement(self);
			}


			else
			{
				printf("this function need function statements\n");
				exit(-1);
			}

		}
		else
		{
			//����
			printf("please give the function a name\n");
			exit(-1);
		}
	}
	return node;
}

//����ָ�����
ASTnode_func_point* parser_function_pointer(dian_st_parser* parser)
{
	dian_st_parser* self = parser;
	ASTnode_func_point* func_pointer = malloc(sizeof(ASTnode_func_point));
	self->cb_eat(self, LS_BRACE);
	dian_en_token_id tmp_type = self->p_current_token->type;
	//ȷ������ָ��Ĳ���
	if (self->p_current_token->type == RS_BRACE)
	{
		self->cb_eat(self, RS_BRACE);
		func_pointer->param_list = NULL;
	}
	else
	{
		ASTnode_func_typelist* tmp = func_pointer->param_list;

		//��ɲ����б�Ķ�ȡ
		while (self->p_current_token->type != RS_BRACE)
		{
			if (tmp_type != T_INT && tmp_type != T_FLOAT && tmp_type != T_STRING)
			{
				printf("������");
				exit(-1);
			}
			else
			{
				if (tmp == NULL)
				{
					func_pointer->param_list = malloc(sizeof(ASTnode_func_typelist));
					tmp = func_pointer->param_list;
					tmp->type = tmp_type;
					tmp->next_type = NULL;
				}
				else
				{
					while (tmp->next_type != NULL)
					{
						tmp = tmp->next_type;
					}
					tmp->next_type = malloc(sizeof(ASTnode_func_typelist));
					tmp = tmp->next_type;
					tmp->type = tmp_type;
					tmp->next_type = NULL;
				}
				self->cb_eat(self, tmp_type);
				if (self->p_current_token->type == COMMA)
				{
					self->cb_eat(self, COMMA);
					tmp_type = self->p_current_token->type;
					if (tmp_type != T_INT && tmp_type != T_FLOAT && tmp_type != T_STRING)
					{
						printf("�����������\n");
						exit(-1);
					}
				}
			}
		}
		self->cb_eat(self, RS_BRACE);
	}
		
	tmp_type = self->p_current_token->type;
	if (tmp_type != T_INT && tmp_type != T_FLOAT && tmp_type != T_STRING)
	{
		func_pointer->return_list = NULL;
	}
	else
	{
		ASTnode_func_typelist* tmp = func_pointer->return_list;

		while (1)
		{
			tmp_type = self->p_current_token->type;
			self->lexer->cb_get_peek_token(self->lexer);
			dian_en_token_id next_type = gp_peek_token->type;
			if ((tmp_type != T_INT && tmp_type != T_FLOAT && tmp_type != T_STRING) || next_type != COMMA && next_type != RS_BRACE)
			{
				break;
			}
			else
			{
				if (tmp == NULL)
				{
					func_pointer->return_list = malloc(sizeof(ASTnode_func_typelist));
					tmp = func_pointer->return_list;
					tmp->type = tmp_type;
					tmp->next_type = NULL;
				}
				else
				{
					while (tmp->next_type != NULL)
					{
						tmp = tmp->next_type;
					}
					tmp->next_type = malloc(sizeof(ASTnode_func_typelist));
					tmp = tmp->next_type;
					tmp->type = tmp_type;
					tmp->next_type = NULL;
				}
				self->cb_eat(self, tmp_type);
				if(self->p_current_token->type == COMMA)
				self->cb_eat(self, COMMA);
				else
				{
					break;
				}

			}
		}

	}

	return func_pointer;
}

/****************************************
*                                       *
*       compound_statement����          *
*                                       *
****************************************/

ASTnode* parser_callback_compound_statement(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode* node = NULL;
	if (self->p_current_token->type == LB_BRACE)
	{
		self->cb_eat(self, LB_BRACE);
		node = self->cb_statement_list(self);
		self->cb_eat(self, RB_BRACE);
	}
	return node;
}


/****************************************
*                                       *
*           if_statement����            *
*                                       *
****************************************/

/*if_else�ڵ�ṹ*/
//typedef struct AST_node_if_statement
//{
//	dian_en_token_id type;
//	ASTnode* condition;
//	ASTnode* exec_statement;
//	struct AST_node_if_statement* other;
//}ASTnode_if_statement;
ASTnode* parser_callback_if_statement(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode* node = (ASTnode*)malloc(sizeof(ASTnode));
	node->type = node_if;
	node->eval = cb_eval_if_else;
	node->property.p_if = (ASTnode_if_statement*)malloc(sizeof(ASTnode_if_statement));
	node->property.p_if->type = K_IF;
	node->property.p_if->other = NULL;
	if (self->p_current_token->type == K_IF)
	{
		self->cb_eat(self, K_IF);
		node->property.p_if->condition = self->cb_simple_statement(self);
		node->property.p_if->exec_statement = self->cb_compound_statement(self);
		node->property.p_if->type = K_IF;
		while (self->p_current_token->type == K_ELIF)
		{
			self->cb_eat(self, K_ELIF);
			ASTnode_if_statement* tmp = node->property.p_if;
			while (tmp->other != NULL)
			{
				tmp = tmp->other;
			}
			tmp->other = (ASTnode_if_statement*)malloc(sizeof(ASTnode_if_statement));
			tmp = tmp->other;

			tmp->type = K_ELIF;
			tmp->other = NULL;
			tmp->condition = self->cb_simple_statement(self);
			tmp->exec_statement = self->cb_compound_statement(self);
		}
		if (self->p_current_token->type == K_ELSE)
		{
			self->cb_eat(self, K_ELSE);
			ASTnode_if_statement* tmp = node->property.p_if;
			while (tmp->other != NULL)
			{
				tmp = tmp->other;
			}
			tmp->other = (ASTnode_if_statement*)malloc(sizeof(ASTnode_if_statement));
			tmp = tmp->other;
			tmp->type = K_ELSE;
			tmp->other = NULL;
			tmp->exec_statement = self->cb_compound_statement(self);
		}
	}
	else
	{
		//error
		exit(-1);
	}
	return node;
}

/****************************************
*                                       *
*           for_statement����           *
*                                       *
****************************************/

/*for�ڵ�ṹ*/
//typedef struct AST_node_for_statement
//{
//	ASTnode* statement_A;
//	ASTnode* statement_B;
//	ASTnode* statement_C;
//	ASTnode* exec_statement;
//	ASTnode* else_statement;
//}ASTnode_for_statement;
ASTnode* parser_callback_for_statement(void* arg)
{
	dian_st_parser* self = (dian_st_parser*)(arg);
	ASTnode* node = (ASTnode*)malloc(sizeof(ASTnode));
	node->type = node_for;
	node->eval = cb_eval_for_else;
	node->property.p_for = (ASTnode_for_statement*)malloc(sizeof(ASTnode_for_statement));
	if (self->p_current_token->type == K_FOR)
	{
		self->cb_eat(self, K_FOR);
		if (self->p_current_token->type == LB_BRACE)
		{
			node->property.p_for->statement_A = NULL;
			node->property.p_for->statement_B = NULL;
			node->property.p_for->statement_C = NULL;
			node->property.p_for->exec_statement = self->cb_compound_statement(self);
		}
		else
		{
			if (self->p_current_token->type == SEMI)
			{
				node->property.p_for->statement_A = NULL;
				self->cb_eat(self, SEMI);
			}
			else
			{
				node->property.p_for->statement_A = self->cb_simple_statement(self);
				self->cb_eat(self, SEMI);
			}
			if (self->p_current_token->type == SEMI)
			{
				node->property.p_for->statement_B = NULL;
				self->cb_eat(self, SEMI);
			}
			else
			{
				node->property.p_for->statement_B = self->cb_simple_statement(self);
				self->cb_eat(self, SEMI);
			}
			if (self->p_current_token->type == LB_BRACE)
			{
				node->property.p_for->statement_C = NULL;
			}
			else
			{
				node->property.p_for->statement_C = self->cb_simple_statement(self);
			}
			node->property.p_for->exec_statement = self->cb_compound_statement(self);
		}

		if (self->p_current_token->type == K_ELSE)
		{
			self->cb_eat(self, K_ELSE);
			node->property.p_for->else_statement = self->cb_compound_statement(self);
		}
		else
		{
			node->property.p_for->else_statement = NULL;
		}
	}
	return node;
}