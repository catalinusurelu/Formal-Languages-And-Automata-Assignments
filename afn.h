#ifndef AFN_H
#define AFN_H

#include <vector>
#include <string>
#include <map>
#include <iostream>
#include "types.h"

using namespace std;

class AFN
{
public:
	AFN();
	AFN(Symbol symbol);
	AFN(AFN* afnLeft, AFN* afnRight, Operator op);
	AFN(AFN* afn, Operator op);
	
	void set_alphabet(Alphabet alphabet);
	void addTransition(State from, State to, Symbol symbol);
	void addState(State state);
	State generateNewStateName(Graph graph);

	Graph graph;
	Alphabet alphabet;
	State start;
	States final;
	Format format;
	
	void concatenate(AFN* afnLeft, AFN* afnRight);
	void alternate(AFN* afnLeft, AFN* afnRight);
	void kleenStar(AFN* afn);
	void optional(AFN* afn);
	void plus(AFN* afn);
	
	void rename(string append);
	void normalize_state_names();
	void normalize_transitions();
	States move(States nfa_states, Symbol x);
	
	
	friend ostream& operator<< (ostream &out, AFN &afn);
	
};

#endif