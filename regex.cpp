#include "regex.h"
#include <iostream>
using namespace std;

Regex::Regex()
{
	root = new Node();
}

Regex::Regex(Symbol symbol)
{
	root = new Node();
	root->type = SYMB;
	root->symbol = symbol;
}

Regex::Regex(Regex* regexLeft, Regex* regexRight, Operator op)
{
	root = new Node();
	root->type = OPERATOR;
	root->op = op;
	root->left = regexLeft->root;
	root->right = regexRight->root;
}

Regex::Regex(Regex* regex, Operator op)
{
	root = new Node();
	root->type = OPERATOR;
	root->op = op;
	root->left = regex->root;
}


// Simplifca expresia regulata
void Regex::simplify()
{
	simplify(root);
}

void Regex::simplify(Node* &root)
{
	if(root->left)
	{
		simplify(root->left);
	}
	
	if(root->right)
	{
		simplify(root->right);
	}
	
	if(root->type == OPERATOR && root->op == KLEEN_STAR)
	{
		// e* = e
		
		if(root->left && root->left->type == SYMB)
		{
			if(root->left->symbol == "e")
			{
				Node* left = root->left;
				delete root;
				root = left;
				return;
			}
		}
	}
	
	// O* = e
	
	if(root->type == OPERATOR && root->op == KLEEN_STAR)
	{
		if(root->left && root->left->type == SYMB)
		{
			if(root->left->symbol == "O")
			{
				delete root->left;
				delete root;
				root = (new Regex("e"))->root; // very bad programming :(
				return;
			}
		}
	}
	
	
	if(root->type == OPERATOR && root->op == CONCATENATE)
	{
		// Ox = xO = O
		
		if(root->left && root->left->type == SYMB)
		{
			if(root->left->symbol == "O")
			{
				delete root->right;
				delete root->left;
				delete root;
				root = (new Regex("O"))->root;
				return;
			}
		}
		
		if(root->right && root->right->type == SYMB)
		{
			if(root->right->symbol == "O")
			{
				delete root->left;
				delete root->right;
				delete root;
				root = (new Regex("O"))->root;
				return;
			}
		}
		
		// ex = xe = x
		
		if(root->left && root->left->type == SYMB)
		{
			if(root->left->symbol == "e")
			{
				Node* right = root->right;
				delete root->left;
				delete root;
				root = right;
				return;
			}
		}
		
		if(root->right && root->right->type == SYMB)
		{
			if(root->right->symbol == "e")
			{
				Node* left = root->left;
				delete root->right;
				delete root;
				root = left;
				return;
			}
		}
				
	}
	
	
	if(root->type == OPERATOR && root->op == ALTERNATE)
	{
		// O | x = x | O = x
		
		if(root->left && root->left->type == SYMB)
		{
			if(root->left->symbol == "O")
			{
				Node* right = root->right;
				delete root->left;
				delete root;
				root = right;
				return;
			}
		}
		
		if(root->right && root->right->type == SYMB)
		{
			if(root->right->symbol == "O")
			{
				Node* left = root->left;
				delete root->right;
				delete root;
				root = left;
				return;
			}
		}
		
		// x | x = x
		
		if(root->left && root->left->type == SYMB &&
		   root->right && root->right->type == SYMB &&
		   root->right->symbol == root->left->symbol)
		{
			Symbol symb = root->right->symbol;
			delete root->left;
			delete root->right;
			delete root;
			root = (new Regex(symb))->root;
		}
			
	}
}

void Regex::set_alphabet(Alphabet alphabet)
{
	this->alphabet = alphabet;
}

void print_regex(ostream &out, Node* root)
{
	// Sa nu punem paranteze aiurea doar in jurului unui caracter
	if(!(root->left) && !(root->right))
	{
		out << *root;
		return;
	}
	
	out << "(";
	if(root->left)
	{
		print_regex(out, root->left);
	}
	
	out << *root;
	
	if(root->right)
	{
		print_regex(out, root->right);
	}
	out << ")";
}

void print_regex(ostream &out, Regex &regex)
{
	print_regex(out, regex.root);
}

ostream& operator<< (ostream &out, Node& nod)
{
	if(nod.type == OPERATOR)
	{
		switch(nod.op)
		{
			case CONCATENATE: out << ""; break;
			case KLEEN_STAR: out << "*"; break;
			case PLUS: out << "+"; break;
			case ALTERNATE: out << "|"; break;
			case OPTIONAL: out << "?"; break;
		}
	}
	else
	{
		out << nod.symbol;
	}
	
	return out;
}

ostream& operator<< (ostream &out, Regex &regex)
{
	string formated_alphabet = "{";
	for(auto it: regex.alphabet)
	{
		formated_alphabet += it;
		formated_alphabet += ",";
	}
	formated_alphabet.pop_back();
	formated_alphabet.push_back('}');
	
	out << formated_alphabet;
	out << ':';
	print_regex(out, regex);
	
	return out;
}
