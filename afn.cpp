#include "afn.h"
#include <ctype.h>
#include <sstream>
#include <set>
#include <cstdlib>

using namespace std;


AFN::AFN()
{
}

AFN::AFN(Symbol symbol)
{
	start = State("_start");
	addState(start);
	
	// Nu avem stari finale
	if(symbol == "O")
	{
		return;
	}
	
	final.push_back(State("_finish"));
	addState(final.back());
	graph[start].push_back(Transition(final.back(), symbol));
}

AFN::AFN(AFN* afnLeft, AFN* afnRight, Operator op)
{
	switch(op)
	{
		case CONCATENATE: concatenate(afnLeft, afnRight); break;
		case ALTERNATE: alternate(afnLeft, afnRight); break;
	}
}

AFN::AFN(AFN* afn, Operator op)
{
	switch(op)
	{
		case KLEEN_STAR: kleenStar(afn); break;
		case PLUS: plus(afn); break;
		case OPTIONAL: optional(afn); break;
	}	
}

void AFN::concatenate(AFN* afnLeft, AFN* afnRight)
{
	afnLeft->rename("l");
	afnRight->rename("r");
	
	graph.insert(afnRight->graph.begin(), afnRight->graph.end());
	graph.insert(afnLeft->graph.begin(), afnLeft->graph.end());
    for(auto it: afnLeft->final)
    {
    	addTransition(it, afnRight->start, "e");
    }
	start = afnLeft->start;
	final = afnRight->final;
}

void AFN::alternate(AFN* afnLeft, AFN* afnRight)
{
	afnLeft->rename("l");
	afnRight->rename("r");
	
	graph.insert(afnRight->graph.begin(), afnRight->graph.end());
	graph.insert(afnLeft->graph.begin(), afnLeft->graph.end());
			
	start = generateNewStateName(graph);
	addState(start);
	addTransition(start, afnLeft->start, "e");
	addTransition(start, afnRight->start, "e");
	
	final.push_back(generateNewStateName(graph));
	addState(final.back());
    for(auto it: afnLeft->final)
    {
    	addTransition(it, final.back(), "e");
    }
    for(auto it: afnRight->final)
    {
    	addTransition(it, final.back(), "e");
    }
}

void AFN::kleenStar(AFN* afn)
{
	graph.insert(afn->graph.begin(), afn->graph.end());
	start = generateNewStateName(graph);
	addState(start);
	final.push_back(generateNewStateName(graph));
	addState(final.back());
	
	
	addTransition(start, afn->start, "e");
	addTransition(start, final.back(), "e");
	for(auto it: afn->final)
	{
		addTransition(it, final.back(), "e");
	}
	addTransition(final.back(), afn->start, "e");
}

void AFN::plus(AFN* afn)
{
	AFN* aStar = new AFN(afn, KLEEN_STAR);
	concatenate(afn, aStar);
}

void AFN::optional(AFN* afn)
{
	AFN* eps = new AFN("e");
	alternate(eps, afn);
}

void AFN::set_alphabet(Alphabet alphabet)
{
	this->alphabet = alphabet;
}

void AFN::addTransition(State from, State to, Symbol symbol)
{
	graph[from].push_back(Transition(to, symbol));
}

void AFN::addState(State state)
{
	graph[state] = Neighbours();
}

// Cauta cea mai mare stare (lexicografic)
// Daca, contine o cifra la sfarsit, o incrementeaza
// Daca cifra devine 10, o transform in 0 si mai adaug un 1
// la sfarsitul sirului
State AFN::generateNewStateName(Graph graph)
{
	// luam cel mai mare string lexicografic
	string last_str = "";
	for(auto it: graph)
	{
		if(it.first > last_str)
		{
			last_str = it.first;
		}
	}
	
	// nu avem stari => dam o stare initiala
	if(last_str == "")
	{
		return State("S1");
	}
	else
	{
		if(isdigit(last_str.back()))
		{
			int digit = last_str.back() - '0';
			if(digit == 9)
			{
				return(Symbol(last_str + "0"));
			}
			else
			{
				last_str.pop_back();
				return(Symbol(last_str + to_string(digit + 1)));
			}
		}
		return(Symbol(last_str + "1"));
	}
}

Symbol to_symbol(char c);

// Transform tranzitiile pe cuvinte in tranzitii pe simboluri
// Adica il fac sa arate mai a DFA :) gen S1-> ab -> S2 devine
// S1 -> a -> S3 -> b -> S2
void AFN::normalize_transitions()
{
	Graph temp_graph = graph;
	Graph name_graph = graph;
	graph = Graph();

	for(auto it: temp_graph)
	{
		addState(it.first);
		for(auto tranz: it.second)
		{
			State prev_state = it.first;
			for(int i = 0; i < tranz.second.size() - 1; i++)
			{
				State new_state = generateNewStateName(name_graph);
				name_graph[new_state] = Neighbours(); // marcam starea noua ca sa mearga generateNewStateName
				addTransition(prev_state, new_state, to_symbol(tranz.second[i]));
				prev_state = new_state;
			}
			addTransition(prev_state, tranz.first, to_symbol(tranz.second.back()));
		}
	}
}

// Adaug un string la sfarsitul numelui tuturor starilor
// Astfel, cand fac un AFN din regex, si concatenez 2 automate spre exemplu
// sa nu am stari duplicate
void AFN::rename(string append)
{
	Graph temp_graph;
	State temp_start;
	States temp_final;
	
	for(auto it: graph)
	{
		temp_graph[it.first + append] = Neighbours();
		for(auto it2: it.second)
		{
			temp_graph[it.first + append].push_back(Transition(it2.first + append, it2.second));
		}
	}
	
	temp_start = start + append;
	
	for(auto it: final)
	{
		temp_final.push_back(it + append);
	}
	
	graph = temp_graph;
	start = temp_start;
	final = temp_final;
}


// Din re -> afn rezulta stari urate; redenumim toate starile frumos
void AFN::normalize_state_names()
{
	Graph temp_graph;
	State temp_start;
	States temp_final;
	
	map<State, State> new_names;
	
	int nr = 0;
	for(auto it: graph)
	{
		new_names[it.first] = "N" + to_string(nr);
		nr++;
	}
	
	for(auto it: graph)
	{
		temp_graph[new_names[it.first]] = Neighbours();
		for(auto it2: it.second)
		{
			temp_graph[new_names[it.first]].push_back(Transition(new_names[it2.first], it2.second));
		}
	}
	
	temp_start = new_names[start];
	
	for(auto it: final)
	{
		temp_final.push_back(new_names[it]);
	}
	
	graph = temp_graph;
	start = temp_start;
	final = temp_final;
}

// Face tranzitii din starile nfa_states in toate starile avand ca simbol
// de tranzitie x
States AFN::move(States nfa_states, Symbol x)
{
    set<State> rez; // Ca sa nu bagam de 2 ori acelasi element

    for(auto it: nfa_states)
    {
        for(auto it2: graph[it])
        {
            if (it2.second == x)
            {
            	rez.insert(it2.first);
            }
        }
    }

    return States(rez.begin(), rez.end());
}

// Afisare format normal
void print_normal(ostream &out, AFN &afn)
{
	out << "(";
	
	// afiseaza stari
	string states = "{";
	for(auto it: afn.graph)
	{
		states += it.first;
		states += ",";
	}
	states.pop_back();
	states.push_back('}');
	out << states;
	
	out << ",";
	
	// afiseaza alfabet
	string formated_alphabet = "{";
	for(auto it: afn.alphabet)
	{
		formated_alphabet += it;
		formated_alphabet += ",";
	}
	formated_alphabet.pop_back();
	formated_alphabet.push_back('}');
	out << formated_alphabet;
	
	out << ",";
	
	// afiseaza relatii
	int nr_relations = 0;
	string relations = "{";
	for(auto it: afn.graph)
	{
		for(auto it2: it.second)
		{
			relations += "(" + it.first + "," + it2.second + "," + it2.first + ")";
			relations += ",";
			nr_relations++;
		}
	}
	relations.pop_back();
	relations.push_back('}');
	if(nr_relations == 0) relations = "O";
	out << relations;
	
	out << ",";
	
	// afiseaza start
	out << afn.start;
	
	out << ",";
	
	// afiseaza stari final
	if(afn.final.size() == 0)
	{
		out << "O";
	}
	else
	{
		states = "{";
		for(auto it: afn.final)
		{
			states += it;
			states += ",";
		}
		states.pop_back();
		states.push_back('}');
		out << states;
	}
	
	out << ")";
}


// Afisare format XML
void print_xml(ostream &out, AFN &afn)
{
	out << "<NFA>\n";
	
	// afiseaza stari
	string states = "\t<states>\n";
	for(auto it: afn.graph)
	{
		states += "\t\t<state>";
		states += it.first;
		states += "</state>\n";
	}
	states += "\t</states>\n";
	
	out << states;
	
	// afiseaza alfabet
	string formated_alphabet = "\t<alphabet>\n";
	for(auto it: afn.alphabet)
	{
		formated_alphabet += "\t\t<symbol>";
		formated_alphabet += it;
		formated_alphabet += "</symbol>\n";
	}
	formated_alphabet += "\t</alphabet>\n";
	
	out << formated_alphabet;
	
	// afiseaza relatii
	string relations = "\t<Delta>\n";
	for(auto it: afn.graph)
	{
		for(auto it2: it.second)
		{
			relations += "\t\t<relation>\n";
			
			relations += "\t\t\t<source>\n";
			relations += "\t\t\t\t<state>";
			relations += it.first;
			relations += "</state>\n";
			relations += "\t\t\t</source>\n";
			
			relations += "\t\t\t<word>\n";
			
			if(it2.second == "e")
			{
				relations += "\t\t\te";	
			}
			else
			{
				for(auto chr: it2.second)
				{
					relations += "\t\t\t\t<symbol>";
					relations += chr;
					relations += "</symbol>\n";
				}
			}
			
			relations += "\t\t\t</word>\n";
						
			relations += "\t\t\t<destination>\n";
			relations += "\t\t\t\t<state>";
			relations += it2.first;
			relations += "</state>\n";
			relations += "\t\t\t</destination>\n";
			
			relations += "\t\t</relation>\n";
		}
	}
	relations += "\t</Delta>\n";

	out << relations;
	
	// afiseaza start
	string initial = "\t<initial>\n";
	initial += "\t\t<state>";
	initial += afn.start;
	initial += "</state>\n";
	initial += "\t</initial>\n";
	
	out << initial;
	
	
	// afiseaza stari final

	states = "\t<final>\n";
	for(auto it: afn.final)
	{
		states += "\t\t<state>";
		states += it;
		states += "</state>\n";
	}
	states += "\t</final>\n";
	
	out << states;

	out << "</NFA>\n";
}

ostream& operator<< (ostream &out, AFN &afn)
{
	if(afn.format == NORMAL)
	{
		print_normal(out, afn);
	}
	else
	{
		print_xml(out, afn);
	}
	
	return out;
}

