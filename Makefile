build:
	flex main.l
	g++ -std=c++11 lex.yy.c regex.cpp afn.cpp afd.cpp algorithms.cpp -lfl -o main
