#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "lexer.h"
#include "lib.h"


char* ga_keyword[NUM_KEYWORD] = {
	"func",
	"return",
	"if",
	"elif",
	"else",
	"assign",
	"for",
	"break"
};

char ga_operator[NUM_OPERATOR] = {
	'+',
	'-',
	'*',
	'/',
	'%',
	'<',
	'>',
	'=',
	'&',
	'|',
	'!',
	'~',
	'^'
};

void lexer_callback_skip_next(void* arg)
{
	dian_st_lexer* self = (dian_st_lexer*)(arg);
	self->pos++;
}

void lexer_callback_peek_next_token(void* arg)
{
	free(gp_peek_token);
	dian_st_lexer* self = (dian_st_lexer*)(arg);
	int pos = self->pos;
	gp_peek_token = self->cb_get_next_token(self);
	self->pos = pos;
}

dian_st_token* lexer_callback_get_next_token(void* arg)
{
	dian_st_lexer* self = (dian_st_lexer*)(arg);
	while (self->text[self->pos]!=0)
	{
		char tmp = self->text[self->pos];
		//空格，换行，制表符的处理
		if (is_space(tmp))
		{
			self->cb_skip_next(self);
			continue;
		}
		//标识符的识别
		else if (is_alpha(tmp))
		{
			return self->cb_identifier(self);
		}
		//数字型常量的识别
		else if (is_float_number(tmp))
		{
			return self->cb_number(self);
		}
		//运算符的识别
		else if (is_operator(tmp))
		{
			return self->cb_operator(self);
		}
		//字符串常量的识别
		else if (tmp == '\"')
		{
			return self->cb_string(self);
		}
		//注释的处理
		else if (tmp == '@')
		{
			self->cb_comment(self);
			continue;
		}
		else if (tmp == ',')
		{
			dian_st_token* p_new_token = malloc(sizeof(dian_st_token));
			memset(p_new_token->value, 0, MAX_SIZE_TOKEN_VALUE);
			p_new_token->type = COMMA;
			strcpy(p_new_token->value, ",");
			self->cb_skip_next(self);
			return p_new_token;
		}
		else if (tmp == '{')
		{
			dian_st_token* p_new_token = malloc(sizeof(dian_st_token));
			memset(p_new_token->value, 0, MAX_SIZE_TOKEN_VALUE);
			p_new_token->type = LB_BRACE;
			strcpy(p_new_token->value, "{");
			self->cb_skip_next(self);
			return p_new_token;
		}
		else if (tmp == '}')
		{
			dian_st_token* p_new_token = malloc(sizeof(dian_st_token));
			memset(p_new_token->value, 0, MAX_SIZE_TOKEN_VALUE);
			p_new_token->type = RB_BRACE;
			strcpy(p_new_token->value, "}");
			self->cb_skip_next(self);
			return p_new_token;
		}
		else if (tmp == '[')
		{
			dian_st_token* p_new_token = malloc(sizeof(dian_st_token));
			memset(p_new_token->value, 0, MAX_SIZE_TOKEN_VALUE);
			p_new_token->type = LM_BRACE;
			strcpy(p_new_token->value, "[");
			self->cb_skip_next(self);
			return p_new_token;
		}
		else if (tmp == ']')
		{
			dian_st_token* p_new_token = malloc(sizeof(dian_st_token));
			memset(p_new_token->value, 0, MAX_SIZE_TOKEN_VALUE);
			p_new_token->type = RM_BRACE;
			strcpy(p_new_token->value, "]");
			self->cb_skip_next(self);
			return p_new_token;
		}
		else if (tmp == '(')
		{
			dian_st_token* p_new_token = malloc(sizeof(dian_st_token));
			memset(p_new_token->value, 0, MAX_SIZE_TOKEN_VALUE);
			p_new_token->type = LS_BRACE;
			strcpy(p_new_token->value, "(");
			self->cb_skip_next(self);
			return p_new_token;
		}
		else if (tmp == ')')
		{
			dian_st_token* p_new_token = malloc(sizeof(dian_st_token));
			memset(p_new_token->value, 0, MAX_SIZE_TOKEN_VALUE);
			p_new_token->type = RS_BRACE;
			strcpy(p_new_token->value, ")");
			self->cb_skip_next(self);
			return p_new_token;
		}
		else if (tmp == ';')
		{
			dian_st_token* p_new_token = malloc(sizeof(dian_st_token));
			memset(p_new_token->value, 0, MAX_SIZE_TOKEN_VALUE);
			p_new_token->type = SEMI;
			strcpy(p_new_token->value, ";");
			self->cb_skip_next(self);
			return p_new_token;
		}
		else if (tmp == ':')
		{
			dian_st_token* p_new_token = malloc(sizeof(dian_st_token));
			memset(p_new_token->value, 0, MAX_SIZE_TOKEN_VALUE);
			p_new_token->type = COLON;
			strcpy(p_new_token->value, ":");
			self->cb_skip_next(self);
			return p_new_token;
		}
		//填写报错
		else
		{
			printf("未定义的字符\n");
			exit(-1);
		}
		return NULL;
	}
	dian_st_token* p_new_token = malloc(sizeof(dian_st_token));
	memset(p_new_token->value, 0, MAX_SIZE_TOKEN_VALUE);
	p_new_token->type = END;
	return p_new_token;
}

dian_st_token* lexer_callback_string(void* arg)
{
	dian_st_lexer* self = (dian_st_lexer*)(arg);
	dian_st_token* p_new_token = malloc(sizeof(dian_st_token));
	memset(p_new_token->value, 0, MAX_SIZE_TOKEN_VALUE);
	p_new_token->type = C_STRING;
	self->cb_skip_next(self);
	int i = 0;
	char tmp = self->text[self->pos];
	while (tmp != 0 && tmp != '\"' && i < 64)
	{
		if (tmp == '\\')
		{
			self->cb_skip_next(self);
			if (self->text[self->pos] == 'n')
			{
				p_new_token->value[i] = '\n';
				i++;
				self->cb_skip_next(self);
				tmp = self->text[self->pos];
			}
			else if (self->text[self->pos] == 't')
			{
				p_new_token->value[i] = '\t';
				i++;
				self->cb_skip_next(self);
				tmp = self->text[self->pos];
			}
			else if (self->text[self->pos] == '"')
			{
				p_new_token->value[i] = '\"';
				i++;
				self->cb_skip_next(self);
				tmp = self->text[self->pos];
			}
			else if (self->text[self->pos] == '\\')
			{
				p_new_token->value[i] = '\\';
				i++;
				self->cb_skip_next(self);
				tmp = self->text[self->pos];
			}
			//添加其余转义字符的设置
			else
			{
				p_new_token->value[i] = tmp;
				i++;
				tmp = self->text[self->pos];
			}
		}
		else
		{
			p_new_token->value[i] = tmp;
			i++;
			self->cb_skip_next(self);
			tmp = self->text[self->pos];
		}
	}

	if (tmp == '\"')
	{
		self->cb_skip_next(self);
		return p_new_token;
	}
	else if (i >= 64)
	{
		//报错处理	字符串过长
		printf("error: string is more than length\n");
		exit(-1);
	}
	else 
	{
		//报错处理	缺少匹配的"
		printf("error:no \" to end the string\n");
		exit(-1);
	}
	return NULL;
}

dian_st_token* lexer_callback_identifier(void* arg)
{
	dian_st_lexer* self = (dian_st_lexer*)(arg);
	dian_st_token* p_new_token = malloc(sizeof(dian_st_token));
	memset(p_new_token->value, 0, MAX_SIZE_TOKEN_VALUE);
	p_new_token->type = ID;
	int i = 0;
	char tmp = self->text[self->pos];
	while (tmp != 0 && (is_alpha(tmp) || is_int_number(tmp)))
	{
		p_new_token->value[i] = tmp;
		i++;
		self->cb_skip_next(self);
		tmp = self->text[self->pos];
	}
	if (strlen(p_new_token->value) > MAX_ID_LENGTH)
	{
		//报错处理
	}
	//关键词的处理
	if (is_keyword(p_new_token->value))
	{
		if (!strcmp(p_new_token->value, "if"))
		{
			p_new_token->type = K_IF;
		}
		else if (!strcmp(p_new_token->value, "elif"))
		{
			p_new_token->type = K_ELIF;
		}
		else if (!strcmp(p_new_token->value, "else"))
		{
			p_new_token->type = K_ELSE;
		}
		else if (!strcmp(p_new_token->value, "for"))
		{
			p_new_token->type = K_FOR;
		}
		else if (!strcmp(p_new_token->value, "break"))
		{
			p_new_token->type = K_BREAK;
		}
		else if (!strcmp(p_new_token->value, "func"))
		{
			p_new_token->type = K_FUNC;
		}
		else if (!strcmp(p_new_token->value, "return"))
		{
			p_new_token->type = K_RETURN;
		}
		else if (!strcmp(p_new_token->value, "assign"))
		{
			p_new_token->type = K_ASSIGN;
		}
		else
		{
			//报错处理
		}
	}
	//定义符的处理
	if(is_typedef(p_new_token->value))
	{
		if (!strcmp(p_new_token->value, "int"))
		{
			p_new_token->type = T_INT;
		}
		else if (!strcmp(p_new_token->value, "float"))
		{
			p_new_token->type = T_FLOAT;
		}
		else if (!strcmp(p_new_token->value, "string"))
		{
			p_new_token->type = T_STRING;
		}
		else
		{
			//报错处理
		}
	}


	return p_new_token;
}

dian_st_token* lexer_callback_number(void* arg)
{
	dian_st_lexer* self = (dian_st_lexer*)(arg);
	dian_st_token* p_new_token = malloc(sizeof(dian_st_token));
	memset(p_new_token->value, 0, MAX_SIZE_TOKEN_VALUE);
	int type_float = 0;
	int i = 0;
	char tmp = self->text[self->pos];
	if (tmp == '.')
	{
		p_new_token->value[i] = '0';
		i++;
	}
	while (tmp != 0 && is_float_number(tmp))
	{
		if (tmp == '.')
		{
			type_float = 1;
		}
		p_new_token->value[i] = tmp;
		i++;
		self->cb_skip_next(self);
		tmp = self->text[self->pos];
	}
	if (type_float == 1)
		p_new_token->type = C_FLOAT;
	else
		p_new_token->type = C_INT;
	return p_new_token;
}

dian_st_token* lexer_callback_comment(void* arg)
{
	dian_st_lexer* self = (dian_st_lexer*)(arg);
	self->cb_skip_next(self);
	char tmp = self->text[self->pos];
	if (tmp == '@')
	{
		while (tmp != '\n')
		{
			self->cb_skip_next(self);
			tmp = self->text[self->pos];
			if (tmp == 0)
			{
				return;
			}
		}
		return NULL;
	}
	else if (tmp == '*')
	{
		self->cb_skip_next(self);
		while (1)
		{
			if (tmp == '*' && self->text[self->pos + 1] == '@')
			{
				break;
			}
			self->cb_skip_next(self);
			tmp = self->text[self->pos];
			if (tmp == 0)
			{
				printf("no matched comment symbol\n");
				exit(-1);
			}
		}
		self->cb_skip_next(self);
		self->cb_skip_next(self);
		return NULL;
	}
	else
	{
		printf("未定义的字符\n");
		exit(-1);
	}
}

dian_st_token* lexer_callback_operator(void* arg)
{
	dian_st_lexer* self = (dian_st_lexer*)(arg);
	dian_st_token* p_new_token = malloc(sizeof(dian_st_token));
	memset(p_new_token->value, 0, MAX_SIZE_TOKEN_VALUE);
	char tmp = self->text[self->pos];
	if (tmp != 0 && is_operator(tmp))
	{
		if (tmp == '+')
		{
			self->cb_skip_next(self);
			char tmp = self->text[self->pos];
			if (tmp == '=')
			{
				p_new_token->type = O_PLUS_ASSIGN;
				strcpy(p_new_token->value, "+=");
				self->cb_skip_next(self);
			}
			else
			{
				strcpy(p_new_token->value, "+");
				p_new_token->type = O_PLUS;
			}
			return p_new_token;
		}
		if (tmp == '-')
		{
			self->cb_skip_next(self);
			char tmp = self->text[self->pos];
			if (tmp == '=')
			{
				p_new_token->type = O_MINUS_ASSIGN;
				strcpy(p_new_token->value, "-=");
				self->cb_skip_next(self);
			}
			else
			{
				strcpy(p_new_token->value, "-");
				p_new_token->type = O_MINUS;
			}
			return p_new_token;
		}
		if (tmp == '%')
		{
			self->cb_skip_next(self);
			char tmp = self->text[self->pos];
			if (tmp == '=')
			{
				p_new_token->type = O_REM_ASSIGN;
				strcpy(p_new_token->value, "%=");
				self->cb_skip_next(self);
			}
			else
			{
				strcpy(p_new_token->value, "%");
				p_new_token->type = O_REM;
			}
			return p_new_token;
		}
		if (tmp == '*')
		{
			self->cb_skip_next(self);
			char tmp = self->text[self->pos];
			if (tmp == '*')
			{
				p_new_token->type = O_POWER;
				strcpy(p_new_token->value, "**");
				self->cb_skip_next(self);
			}
			else if (tmp == '=')
			{
				p_new_token->type = O_MUL_ASSIGN;
				strcpy(p_new_token->value, "*=");
				self->cb_skip_next(self);
			}
			else
			{
				strcpy(p_new_token->value, "*");
				p_new_token->type = O_MUL;
			}
			return p_new_token;
		}
		if (tmp == '/')
		{
			self->cb_skip_next(self);
			char tmp = self->text[self->pos];
			if (tmp == '/')
			{
				self->cb_skip_next(self);
				char tmp = self->text[self->pos];
				if (tmp == '=')
				{
					strcpy(p_new_token->value, "//=");
					p_new_token->type = O_I_DIV_ASSIGN;
					self->cb_skip_next(self);
				}
				else 
				{
					strcpy(p_new_token->value, "//");
					p_new_token->type = O_I_DIV;
				}
			}
			else if (tmp == '=')
			{
				strcpy(p_new_token->value, "/=");
				p_new_token->type = O_F_DIV_ASSIGN;
				self->cb_skip_next(self);
			}
			else
			{
				strcpy(p_new_token->value, "/");
				p_new_token->type = O_F_DIV;
			}
			return p_new_token;
		}

		if (tmp == '^')
		{
			p_new_token->type = O_BIT_XNOR;
			self->cb_skip_next(self);
			return p_new_token;
		}
		if (tmp == '~')
		{
			p_new_token->type = O_BIT_NOR;
			self->cb_skip_next(self);
			return p_new_token;
		}

		if (tmp == '&')
		{
			self->cb_skip_next(self);
			char tmp = self->text[self->pos];
			if (tmp == '&')
			{
				p_new_token->type = O_LOGIC_AND;
				strcpy(p_new_token->value, "&&");
				self->cb_skip_next(self);
			}
			else
			{
				strcpy(p_new_token->value, "&");
				p_new_token->type = O_BIT_AND;
			}
			return p_new_token;
		}
		if (tmp == '|')
		{
			self->cb_skip_next(self);
			char tmp = self->text[self->pos];
			if (tmp == '|')
			{
				p_new_token->type = O_LOGIC_OR;
				strcpy(p_new_token->value, "||");
				self->cb_skip_next(self);
			}
			else
			{
				strcpy(p_new_token->value, "|");
				p_new_token->type = O_BIT_OR;
			}
			return p_new_token;
		}

		if (tmp == '<')
		{
			self->cb_skip_next(self);
			char tmp = self->text[self->pos];
			if (tmp == '<')
			{
				strcpy(p_new_token->value, "<<");
				p_new_token->type = O_BIT_LEFT;
				self->cb_skip_next(self);
			}
			else if (tmp == '=')
			{
				strcpy(p_new_token->value, "<=");
				p_new_token->type = O_LESS_EQUAL;
				self->cb_skip_next(self);
			}
			else
			{
				strcpy(p_new_token->value, "<");
				p_new_token->type = O_LESS;
			}
			return p_new_token;
		}		
		if (tmp == '>')
		{
			self->cb_skip_next(self);
			char tmp = self->text[self->pos];
			if (tmp == '>')
			{
				strcpy(p_new_token->value, ">>");
				p_new_token->type = O_BIT_RIGHT;
				self->cb_skip_next(self);
			}
			else if (tmp == '=')
			{
				strcpy(p_new_token->value, ">=");
				p_new_token->type = O_GREAT_EQUAL;
				self->cb_skip_next(self);
			}
			else
			{
				strcpy(p_new_token->value, ">");
				p_new_token->type = O_GREAT;
			}
			return p_new_token;
		}

		if (tmp == '=')
		{
			self->cb_skip_next(self);
			char tmp = self->text[self->pos];
			if (tmp == '=')
			{
				p_new_token->type = O_EQUAL;
				strcpy(p_new_token->value, "==");
				self->cb_skip_next(self);
			}
			else
			{
				strcpy(p_new_token->value, "=");
				p_new_token->type = O_ASSIGN;
			}
			return p_new_token;
		}
		if (tmp == '!')
		{
			self->cb_skip_next(self);
			char tmp = self->text[self->pos];
			if (tmp == '=')
			{
				p_new_token->type = O_NOT_EQUAL;
				strcpy(p_new_token->value, "!=");
				self->cb_skip_next(self);
			}
			else
			{
				strcpy(p_new_token->value, "!");
				p_new_token->type = O_LOGIC_NOR;
			}
			return p_new_token;
		}
	}

	return p_new_token;
}

