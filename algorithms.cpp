#include "regex.h"
#include "afn.h"
#include "afd.h"
#include "algorithms.h"
#include "types.h"

#include <stack>
#include <cctype>
#include <algorithm>
#include <sstream>

using namespace std;

AFN* regex_to_afn(Node* node)
{
	if(node->type == OPERATOR)
	{
		switch (node->op)
		{	
			case ALTERNATE:
				return new AFN(regex_to_afn(node->left), regex_to_afn(node->right), ALTERNATE); break;
			case CONCATENATE:
				return new AFN(regex_to_afn(node->left), regex_to_afn(node->right), CONCATENATE); break;
			case KLEEN_STAR:
				return new AFN(regex_to_afn(node->left), KLEEN_STAR); break;
			case PLUS:
				return new AFN(regex_to_afn(node->left), PLUS); break;
			case OPTIONAL:
				return new AFN(regex_to_afn(node->left), OPTIONAL); break;
		}
	}
	else
	{
		return new AFN(node->symbol);
	}
}

AFN* regex_to_afn(Regex* regex)
{
	AFN* x = regex_to_afn(regex->root);
	x->set_alphabet(regex->alphabet);
	x->normalize_state_names();
	
	return x;
}

string substr(const Expression& expr, int& poz)
{
	string s;
	for(int i = poz; i < expr.size(); i++)
		s+=expr[i];
	s+="\n";
	return s;
}

bool isSymbol(char x)
{
	string other = "!#$%&-./:;<>=@[]^'~";
	return isalpha(x) || isdigit(x) || (other.find(x) != string::npos);
}

// re ::= concat '|' re
//      | concat
Regex* re(const Expression& expr, int& poz)
{
	Regex* st = concat(expr, poz);
	
	// am terminat
	if(poz == expr.size())
	{
		return st;
	}
	
    if (expr[poz] == "|")
    {
        poz++; // trec peste |
        Regex* dr = re(expr, poz);
        Regex* _re = new Regex(st, dr, ALTERNATE);
        return _re;
    }
    else
    {
        return st;
    }
}

Regex* re(const string& expr, int& poz)
{
	Expression exp;
	for(int i = 0; i < expr.size(); i++)
	{
		exp.push_back(to_symbol(expr[i]));
	}
	return re(exp, poz);
}

// concat ::= expr_operator concat
//          | expr_operator
Regex* concat(const Expression& expr, int& poz)
{
	Regex* st = expr_operator(expr, poz);
	
	// am terminat 
	if(poz == expr.size())
	{
		return st;
	}
	
	// verific ca in pozitia precedenta sa se fi terminat un expr_operator
	// iar in pozitia curenta nu se termina unul (adica o paranteza ")" )
	// sau sa inceapa o alternare
    if ((poz != 0 && (isSymbol(expr[poz - 1][0]) || expr[poz - 1] == "*" ||
        expr[poz - 1] == "+" || expr[poz - 1] == "?" || expr[poz - 1] == ")")) && expr[poz] != "|" && expr[poz] != ")")
    {
        Regex* dr = concat(expr, poz);
        Regex* concat_expr = new Regex(st, dr, CONCATENATE);
        return concat_expr;
    }
    else
    {
        return st;
    }
}

// expr_operator ::= atom '*'
//                 | atom '?'
//                 | atom '+'
//                 | atom
Regex* expr_operator(const Expression& expr, int& poz)
{
	Regex* _atom = atom(expr, poz);
	
	// am terminat 
	if(poz == expr.size())
	{
		return _atom;
	}
	
    if (expr[poz] == "*")
    {
        poz++; // sare peste *
        Regex* _expr_operator = new Regex(_atom, KLEEN_STAR);
        return _expr_operator;
    }
    else if (expr[poz] == "?")
    {
        poz++; // sare peste ?
        Regex* _expr_operator = new Regex(_atom, OPTIONAL);
        return _expr_operator;
    }
    else if (expr[poz] == "+")
        {
            poz++; // sare peste +
            Regex* _expr_operator = new Regex(_atom, PLUS);
            return _expr_operator;
        }
    else
    {
        return _atom;
    }
}

// atom ::= symbol
//        | '(' re ')'
Regex* atom(const Expression& expr, int& poz)
{
	if(poz == expr.size())
	{
		return NULL;
	}
	
	Regex* _atom;
    if (expr[poz] == "(")
    {
        poz++;
        _atom = re(expr, poz);
        poz++; // sar peste ")"
    }
    else
    {
    	_atom = symbol(expr, poz);
    }

    return _atom;
}

// parsez symbol | e | O 
Regex* symbol(const Expression& expr, int& poz)
{
	if(poz == expr.size())
	{
		return NULL;
	}
	
	if(isSymbol(expr[poz][0]))
	{
		poz++;
		return new Regex(expr[poz - 1]);
	}
}

// Verifica daca setul de stari current contine o stare finala
bool contains_final(States current, States final)
{
	for(auto it: current)
	{
		if(find(final.begin(), final.end(), it) != final.end())
			return true;
	}
	return false;
}

States construieste_lambda_inchidere(AFN* afn, States s)
{
	States lambda_inchidere = s;
	stack<State> st;
	
	for_each(s.begin(), s.end(), [&](const State& x) { st.push(x); });
	
	while(!st.empty())
	{
		State from = st.top();
		st.pop();
		for_each(afn->graph[from].begin(), afn->graph[from].end(), [&](const Transition& t) 
		{
			if(t.second == "e" && find(lambda_inchidere.begin(), lambda_inchidere.end(), t.first) == lambda_inchidere.end())
			{
				lambda_inchidere.push_back(t.first);
				st.push(t.first);
			}
		});
	}
	
	return lambda_inchidere;
}

bool contains_unvisited(map<State, States>& afd_states, vector<State>& vizitat)
{
	return afd_states.size() != vizitat.size();
}

// Returneaza o stare nevizitat din afd_states (adica un grup de stari din NDA)
States get_unvisited(map<State, States>& afd_states, vector<State>& vizitat)
{
	for(auto it: afd_states)
		if(find(vizitat.begin(), vizitat.end(), it.first) == vizitat.end())
			return it.second;
}

void dbg_print(States s)
{
	for(auto it: s)
		cout << it << " ";
	cout <<endl;
}

// Verific daca cei doi vectori de stari x, y sunt egali
bool isEqual(States x, States y)
{
	if(x.size() != y.size())
		return false;

	for(int i = 0; i < x.size(); i++)
		if(x[i] != y[i])
			return false;
	
	return true;
}
// NFA states = DFA state, aici intoarcem DFA state-ul corespunzator
State get_afd_state_name(map<State, States>& afd_states, States& x)
{
	for(auto it: afd_states)
		if(isEqual(it.second, x))
			return it.first;
	return ""; // Starea asta nu exista
}


AFD* afn_to_afd(AFN* afn)
{
	// Transformam tranzitiile pe cuvinte in tranzitii pe litere (sa arate ca afd)
	afn->normalize_transitions();
	
	AFD* afd = new AFD();
	map<State, States> afd_states;
	vector<State> vizitat;
	
	afd->start = afd->generateNewStateName(afd->graph);
	afd->addState(afd->start);
	States afn_start;
	afn_start.push_back(afn->start);
	afd_states[afd->start] = construieste_lambda_inchidere(afn, afn_start);
	
	while(contains_unvisited(afd_states, vizitat))
	{
		States current = get_unvisited(afd_states, vizitat);
		State current_state = get_afd_state_name(afd_states, current);
		vizitat.push_back(current_state); // am terminat cu starea asta (stare dfa = stari nfa)

		if(contains_final(current, afn->final))
		{
			afd->final.push_back(current_state);
		}
		
		for(auto it: afn->alphabet)
		{
			States next = construieste_lambda_inchidere(afn, afn->move(current, it));
			State next_state;
			
			// verific sa nu mai fi adaugat odata starea
			if(afd_states.find(get_afd_state_name(afd_states, next)) == afd_states.end())
			{
				next_state = afd->generateNewStateName(afd->graph);
				afd->addState(next_state);
				afd_states[next_state] = next;
			}
			else
			{
				// starea exista deja in afd_states
				// ii luam numele (avem corespondenta state NFA = state DFA)
				next_state = get_afd_state_name(afd_states, next);
			}
			afd->addTransition(current_state, next_state, it);
		}
	}
	
	afd->set_alphabet(afn->alphabet);
	
	return afd;
}

void print_regex(ostream &out, Regex &regex);

// Algoritmul k-Path (calculeaza inchiderea tranzitiva)
Regex* afd_to_regex(AFD* afd)
{
	int nr_stari = afd->graph.size();
	
	vector<vector<vector<string> > > R = vector<vector<vector<string> > >(nr_stari + 1, 
			                             vector<vector<string> >(nr_stari + 1,
			                             vector<string>(nr_stari + 1)));

	vector<State> state_name;
	map<State, int> state_nr;
	state_name.push_back(""); // incepe numararea de la 1

	for(auto it: afd->graph)
	{
		state_nr[it.first] = state_name.size();
		state_name.push_back(it.first);
	}

	for(int i = 1; i <= nr_stari; i++ )
		for(int j = 1; j <= nr_stari; j++ )
		{
			// Fac union cu toate simbolurile pentru care exista tranzitie de la starea i la j
			string sum_labels;
			for(auto symbol: afd->alphabet)
				for(auto trans: afd->graph[state_name[i]])
					// from == transition(to, symbol) <=> exista tranzitia (from, to, symbol)
					if(trans.first == state_name[j] && trans.second == symbol)
					{
						sum_labels = sum_labels + "|" + symbol;
						break;
					}
			if(!sum_labels.empty())  sum_labels = sum_labels.substr(1); // sar peste "|"
				
			if(i != j)
			{
				
				if(sum_labels.empty())
				{
					R[i][j][0] = "O";
				}
				else
				{
					R[i][j][0] = sum_labels;
				}
			}
			else if(i == j)
			{
				if(sum_labels.empty())
				{
					R[i][j][0] = "e";
				}
				else
				{
					R[i][j][0] = string("e") + string("|") + sum_labels;
				}
			}		
		}

	for(int k = 1; k <= nr_stari; k++ )
		for(int i = 1; i <= nr_stari; i++ )
			for(int j = 1; j <= nr_stari; j++ )
			{
				R[i][j][k] =  R[i][j][k-1] + "|((" + R[i][k][k-1] + ")(" + R[k][k][k-1] + ")*(" + R[k][j][k-1] + "))";
			}

	Regex* regex;
	string expression;
	expression = "O"; // element neutru pentru |
	for(auto state: afd->final)
	{
		int i = state_nr[state];
		int poz_start = state_nr[afd->start];
		expression += "|";
	    expression += R[poz_start][i][nr_stari];
	}
	
    int poz = 0;
	regex = re(expression, poz);
	regex->set_alphabet(afd->alphabet);
	regex->simplify();
	
	return regex;
}

Symbol to_symbol(char c)
{
    Symbol s;
    s.push_back(c);
    return s;
}