#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "regex.h"
#include "afn.h"
#include "afd.h"


class RegularGrammar;

AFN* regex_to_afn(Regex* regex);
AFD* afn_to_afd(AFN* afn);
Regex* afd_to_regex(AFD* afd);

Regex* re(const Expression& expr, int& poz);
Regex* concat(const Expression& expr, int& poz);
Regex* expr_operator(const Expression& expr, int& poz);
Regex* atom(const Expression& expr, int& poz);
Regex* symbol(const Expression& expr, int& poz);

Symbol to_symbol(char c);
#endif
