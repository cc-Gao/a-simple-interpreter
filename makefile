dian:main.o lib.o lexer.o parser.o eval.o env.o
	gcc -o dian main.o lib.o lexer.o parser.o eval.o env.o
main.o:main.c type.h
	gcc -c main.c
lib.o:lib.c type.h
	gcc -c lib.c
lexer.o:lexer.c lexer.h type.h
	gcc -c lexer.c
parser.o:parser.c parser.h type.h
	gcc -c parser.c
eval.o:eval.c eval.h type.h
	gcc -c eval.c
env.o:env.c env.h type.h
	gcc -c env.c
clean:
	rm dian *.o