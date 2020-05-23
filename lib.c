#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "type.h"
#include "lexer.h"
#include "lib.h"

int is_alpha(char word)
{
	if ( ( word >= 'A' && word <= 'Z') || (word >= 'a' && word <= 'z') || (word == '$'))
		return 1;
	else
		return 0;
}

int is_space(char word)
{
	if (word == ' ' || word == '\n' || word == '\t'|| word == '\r')
		return 1;
	else
		return 0;
}

int is_int_number(char word)
{
	if (word >= '0' && word <= '9')
		return 1;
	else
		return 0;
}

int is_float_number(char word)
{
	if ((word >= '0' && word <= '9') || word == '.')
		return 1;
	else
		return 0;
}

int is_operator(char word)
{
	for (int i = 0; i < NUM_OPERATOR; i++)
	{
		if (word == ga_operator[i])
		{
			return 1;
		}
	}
	return 0;
}

int is_keyword(char* buf)
{
	for (int i = 0; i < NUM_KEYWORD; i++)
		if (!strcmp(buf, ga_keyword[i]))
		{
			return 1;
		}
	return 0;
}

int is_typedef(char* buf)
{
		if (!strcmp(buf, "int"))
		{
			return 1;
		}
		if (!strcmp(buf, "float"))
		{
			return 1;
		}
		if (!strcmp(buf, "string"))
		{
			return 1;
		}
	return 0;
}


void selflib_fitoa(int n, char s[])
{
	int i, sign;
	if ((sign = n) < 0)
		n = -n;
	i = 1;
	s[0] = '.';
	do {
		s[i++] = n % 10 + '0';
	} while ((n /= 10) > 0);
	if (sign < 0)
		s[i++] = '-';
	s[i] = '\0';
	selflib_string_reverse(s);		//由于int类型数字存储关系，此时字符串为反向，需要翻转
}


/*将float类型转换成char类型*/
void selflib_ftoa(float n, char s[])
{
	sprintf(s, "%f", n);
}

void selflib_itoa(int n, char s[])
{
	sprintf(s, "%d", n);
}


int is_float(float number)
{
	char sf[20] = { 0 };
	char si[20] = { 0 };
	selflib_ftoa(number, sf);
	int i = strlen(sf);
	for (; i > 0; i--)
	{
		if (sf[i - 1] == 48)
		{
			sf[i - 1] = '\0';
		}
		else if (sf[i - 1] == '.')
		{
			break;
		}
	}
	selflib_fitoa(number, si);
	int ret = strcmp(sf, si);
	if (ret == 0)
	{
		return 0;
	}
	return 1;
}


/*字符串翻转函数*/
void selflib_string_reverse(char str[])
{
	int len_str = strlen(str);
	char tmp;
	for (int i = 0; i < len_str / 2; i++)
	{
		tmp = str[i];
		str[i] = str[len_str - 1 - i];
		str[len_str - 1 - i] = tmp;
	}
}