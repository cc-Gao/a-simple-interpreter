#pragma once

int is_alpha(char word);

int is_space(char word);

int is_int_number(char word);

int is_float_number(char word);

int is_operator(char word);

int is_keyword(char* buf);

int is_typedef(char* buf);

int is_float(float number);

void selflib_itoa(int n, char s[]);

void selflib_ftoa(float n, char s[]);

