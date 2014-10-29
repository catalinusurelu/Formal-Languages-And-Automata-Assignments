#include "afd.h"
#include <algorithm>

Symbol to_symbol(char c);

AFD::AFD()
{
}

// Functia de contains 
// Fac tranzitii pe fiecare simbol al sirului
// Daca am ajuns intr-o stare finala, il accept, altfel, nu
bool AFD::contains(const string& str)
{
	State current = start;
	int pozSymbol = 0;

	while(pozSymbol < str.size())
	{

		for(auto trans: graph[current])
		{
			if(trans.second == to_symbol(str[pozSymbol]))
			{
				current = trans.first;
				break;
			}
		}
		
		pozSymbol++;
	}
	
	// returnam daca am ajuns intr-o pozitie finala
	return (std::find(final.begin(), final.end(), current) != final.end());
}

void print_normal(ostream &out, AFD &afd)
{
	out << "(";

	// afiseaza stari
	string states = "{";
	for(auto it: afd.graph)
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
	for(auto it: afd.alphabet)
	{
		formated_alphabet += it;
		formated_alphabet += ",";
	}
	formated_alphabet.pop_back();
	formated_alphabet.push_back('}');
	out << formated_alphabet;

	out << ",";
	
	// afiseaza relatii
	int nr_transitions = 0;
	string transitions = "{";
	for(auto it: afd.graph)
	{
		for(auto it2: it.second)
		{
			transitions += "d(" + it.first + "," + it2.second + ")=" + it2.first;
			transitions += ",";
			nr_transitions++;
		}
	}
	transitions.pop_back();
	transitions.push_back('}');
	if(nr_transitions == 0) transitions = "O";
	out << transitions;

	out << ",";
	
	// afiseaza start
	out << afd.start;
	
	out << ",";
	
	// afiseaza stari final
	if(afd.final.size() == 0)
	{
		out << "O";
	}
	else
	{
		states = "{";
		for(auto it: afd.final)
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



void print_xml(ostream &out, AFD &afd)
{
	out << "<DFA>\n";
	
	// afiseaza stari
	string states = "\t<states>\n";
	for(auto it: afd.graph)
	{
		states += "\t\t<state>";
		states += it.first;
		states += "</state>\n";
	}
	states += "\t</states>\n";
	
	out << states;
	
	// afiseaza alfabet
	string formated_alphabet = "\t<alphabet>\n";
	for(auto it: afd.alphabet)
	{
		formated_alphabet += "\t\t<symbol>";
		formated_alphabet += it;
		formated_alphabet += "</symbol>\n";
	}
	formated_alphabet += "\t</alphabet>\n";
	
	out << formated_alphabet;
	
	// afiseaza relatii
	string relations = "\t<delta>\n";
	for(auto it: afd.graph)
	{
		for(auto it2: it.second)
		{
			relations += "\t\t<transition>\n";
			
			relations += "\t\t\t<source>\n";
			relations += "\t\t\t\t<state>";
			relations += it.first;
			relations += "</state>\n";
			relations += "\t\t\t</source>\n";
			
			relations += "\t\t\t<symbol>";
			relations += it2.second;
			relations += "</symbol>\n";
						
			relations += "\t\t\t<destination>\n";
			relations += "\t\t\t\t<state>";
			relations += it2.first;
			relations += "</state>\n";
			relations += "\t\t\t</destination>\n";
			
			relations += "\t\t</transition>\n";
		}
	}
	relations += "\t</delta>\n";

	out << relations;
	
	// afiseaza start
	string initial = "\t<initial>\n";
	initial += "\t\t<state>";
	initial += afd.start;
	initial += "</state>\n";
	initial += "\t</initial>\n";
	
	out << initial;
	
	
	// afiseaza stari final

	states = "\t<final>\n";
	for(auto it: afd.final)
	{
		states += "\t\t<state>";
		states += it;
		states += "</state>\n";
	}
	states += "\t</final>\n";
	
	out << states;

	out << "</DFA>\n";
}

ostream& operator<< (ostream &out, AFD &afd)
{
	if(afd.format == NORMAL)
	{
		print_normal(out, afd);
	}
	else
	{
		print_xml(out, afd);
	}
	
	return out;
}
