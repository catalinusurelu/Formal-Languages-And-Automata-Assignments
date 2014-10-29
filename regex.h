#ifndef REGEX_H
#define REGEX_H

#include "types.h"

class Node
{
public:
	Node() { left = NULL; right = NULL; symbol = ""; }
	Node* left;
	Node* right;
	NodeType type;
	Symbol symbol;
	Operator op;
	
	friend ostream& operator<< (ostream &out, Node& node);
};

class Regex
{
public:
	Regex();
	Regex(Symbol symbol);
	Regex(Regex* RegexLeft, Regex* RegexRight, Operator op);
	Regex(Regex* Regex, Operator op);
	
	void set_alphabet(Alphabet alphabet);
	
	void simplify();
	void simplify(Node* &root);
	
	Node* root;
	Alphabet alphabet;
	Format format;
	
	/*
	void concatenate(Regex* regexLeft, Regex* regexRight);
	void alternate(Regex* regexLeft, Regex* regexRight);
	void kleenStar(Regex* regex);
	void optional(Regex* regex);
	void plus(Regex* regex);*/
	
	friend ostream& operator<< (ostream &out, Regex &regex);
};

#endif