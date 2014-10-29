#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include <string>
#include <map>


using namespace std;

typedef string State;
typedef string Symbol;

typedef pair<State, Symbol> Transition;
typedef vector<Transition> Neighbours;
typedef map<State, Neighbours> Graph;
typedef vector<Symbol> Alphabet;
typedef vector<Symbol> Expression;
typedef vector<State> States;

enum Operator { CONCATENATE, KLEEN_STAR, PLUS, ALTERNATE, OPTIONAL };
enum Representation {_NONE, _REGEX, _AFD, _AFN, _REGULAR_GRAMMAR};
enum Format { NORMAL, XML};
enum NodeType { SYMB, OPERATOR }; 


#endif
