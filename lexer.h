#pragma once


extern dian_st_lexer g_lexer;
extern dian_st_token* gp_peek_token;
extern char* ga_keyword[NUM_KEYWORD];
extern char ga_operator[NUM_OPERATOR];

void lexer_callback_peek_next_token(void* arg);

dian_st_token* lexer_callback_get_next_token(void* arg);

dian_st_token* lexer_callback_identifier(void* arg);

dian_st_token* lexer_callback_number(void* arg);

dian_st_token* lexer_callback_operator(void* arg);

dian_st_token* lexer_callback_string(void* arg);

dian_st_token* lexer_callback_comment(void* arg);

void lexer_callback_skip_next(void* arg);