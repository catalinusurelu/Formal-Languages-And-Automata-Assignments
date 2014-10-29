%{
#include <iostream>
#include <cstring>

#include "regex.h"
#include "afn.h"
#include "afd.h"
#include "algorithms.h"
#include "types.h"

void yyerror(const char *s);

using namespace std; 

// Reprezentarile sunt declarate global (sunt create/modificate direct in lexer)
Regex* regex;
AFN* afn;
AFD* afd;

// Variabile in care se retin rezultate parsate in lexer pentru
// a fi folosite ca output de la o stare la alta
State read_state;
State read_source;
State read_destination;
Symbol read_symbol;
Symbol read_word;

Format from_format = NORMAL;
Format to_format = NORMAL;

Alphabet alphabet;
Expression expression;

// Variabile pentru controlul starii initiale citite in flex (initializam
// din main prima stare din flex)
Representation input_representation = _NONE;
bool started = false;

// Un token este reprezentat de o tranzitie completa (analiza lexicala); aici o impartim in bucati
// pentru a le pasa automatului
void split_transition(char* txt, State& from, State& to, Symbol& symbol)
{
    string str = txt;
    
    int poz = str.find(',');
    from = str.substr(2, poz - 2);
    
    int poz2 = str.find(')', poz);
    symbol = str.substr(poz + 1, poz2 -(poz + 1));
    
    to = str.substr(poz2 + 2);
}

// Un token este reprezentat de o relatie completa (analiza lexicala); aici o impartim in bucati
// pentru a le pasa automatului
void split_relation(char* txt, State& from, State& to, Symbol& symbol)
{
    string str = txt;
    
    int poz = str.find(',');
    from = str.substr(1, poz - 1);
    
    int poz2 = str.find(',', poz + 1);
    symbol = str.substr(poz + 1, poz2 -(poz + 1));
    
    int poz3 = str.find(')', poz2 + 1);
    to = str.substr(poz2 + 1, poz3 - (poz2 + 1));
}

%}

%x READ_REGEX READ_AFN READ_AFD RG_STATE
%x ALPHABET_STATE EXPRESSION_STATE
%x ASEP EXPRESSION_SEPARATOR


%x READ_STATES SSEP
%x READ_TRANSITIONS TSEP
%x READ_FINAL_STATES FSSEP
%x READ_START_STATE
%x READ_RELATIONS RSEP


%x READ_REGEX_XML READ_AFN_XML READ_AFD_XML
%x XML_READ_ALPHABET
%x XML_READ_TRANSITIONS XML_READ_RELATIONS
%x XML_READ_SYMBOL XML_READ_WORD XML_READ_STATE
%x XML_READ_RELATION XML_READ_TRANSITION
%x XML_READ_INITIAL_STATE XML_READ_FINAL_STATES XML_READ_STATES 

%option stack

ws              [ \t\r\n]*
lower           [a-z]{-}[e]
upper           [A-Z]{-}[O]
other           [!#$&%\-./:;<>=@\[\]\^'~]
digit           [0-9]
symbol          {lower}|{digit}|{other}
name            ({upper}|"_")({lower}|{digit}|"_")*
word            e|({symbol}+)

%%
    // Initializez starea initial pentru flex
    if(!started && input_representation == _REGEX && from_format == NORMAL) { BEGIN(READ_REGEX); started = true; }
    if(!started && input_representation == _AFD && from_format == NORMAL)   { BEGIN(READ_AFD); started = true; }
    if(!started && input_representation == _AFN && from_format == NORMAL)   { BEGIN(READ_AFN); started = true; }

    if(!started && input_representation == _REGEX && from_format == XML) { BEGIN(READ_REGEX_XML); started = true; }
    if(!started && input_representation == _AFD && from_format == XML)   { BEGIN(READ_AFD_XML); started = true; }
    if(!started && input_representation == _AFN && from_format == XML)   { BEGIN(READ_AFN_XML); started = true; }
    
    
    
    
%{
 // AFN, AFD
%}


<READ_AFN>{
    "(" ;
    
    {ws}  ;
    
    "{"   {
            yy_push_state(READ_STATES); afn = new AFN();
          }
 }
 
<READ_AFD>{
    "(" ;
    
    {ws}   ;
    
    "{"   { 
            yy_push_state(READ_STATES); afd = new AFD();
         }
 }
 
 
<READ_STATES>{
     {name}   { 
                if(input_representation == _AFN)
                {
                    afn->addState(yytext);
                }
                else
                {
                    afd->addState(yytext);
                }
                
                yy_push_state(SSEP);
              }
    {ws}   ;
}

<SSEP>{
    "," { 
            yy_pop_state();
        }
        
    "},{"   {
                yy_pop_state();
                yy_pop_state(); 
                yy_push_state(ALPHABET_STATE);
            }
            
    {ws}   ;
}


 
<ALPHABET_STATE>{
     {symbol} { 
                alphabet.push_back(to_symbol(yytext[0])); 
                yy_push_state(ASEP);
              }
    {ws}   ;
}

<ASEP>{
    "," {
            yy_pop_state();
        }
    "},{" {
                    if(input_representation == _AFN)
                    {
                        afn->set_alphabet(alphabet);
                    }
                    else
                    {
                        afd->set_alphabet(alphabet);
                    }
                
                    yy_pop_state();
                    yy_pop_state(); 
                    
                    if(input_representation == _AFN)
                    {
                        yy_push_state(READ_RELATIONS);
                    }
                    else
                    {
                        yy_push_state(READ_TRANSITIONS);
                    }
              }
    
          
    "}" {
            // Pentru expresii regulate   
            yy_pop_state();
            yy_pop_state(); 
            yy_push_state(EXPRESSION_SEPARATOR);
        }
    {ws}   ;       
}

<READ_TRANSITIONS>{
     "d("{name}","{symbol}")="{name} {
                                        State from;
                                        State to;
                                        Symbol symbol;
                                        
                                        split_transition(yytext, from, to, symbol);
                                        
                                        afd->addTransition(from, to, symbol);
                                        yy_push_state(TSEP);
                                     }
    {ws}   ;
}


<TSEP>{
    "," {
            yy_pop_state();
        }

    "}," {
            yy_pop_state();
            yy_pop_state(); 
            yy_push_state(READ_START_STATE);
         }
         
    {ws}   ;
}
 
<READ_RELATIONS>{
     "("{name}","{word}","{name}")" {
                                        State from;
                                        State to;
                                        Symbol symbol;
                                        split_relation(yytext, from, to, symbol);
                                        
                                        afn->addTransition(from, to, symbol);
                                        yy_push_state(RSEP);
                                    }
     "O" {
            yy_pop_state(); 
            yy_push_state(RSEP);
         }
         
    {ws}   ;
}


<RSEP>{
    "," {
            yy_pop_state();
        }
    "}," {
            yy_pop_state();
            yy_pop_state(); 
            yy_push_state(READ_START_STATE);
         }
         
    {ws}   ;
}

<READ_START_STATE>{
    {name}  {
                if(input_representation == _AFN)
                {
                    afn->start = yytext;
                }
                else
                {
                    afd->start = yytext;
                }
            
                yy_pop_state();
                yy_push_state(READ_FINAL_STATES);
            }
            
    {ws}   ;
}

<READ_FINAL_STATES>{
     ",{"         ;
     
     "})"         BEGIN(INITIAL);;
     
     ",O"")"    {
                  BEGIN(INITIAL);
                }
     
      
     {name}   {
                if(input_representation == _AFN)
                {
                    afn->final.push_back(yytext); 
                }
                else
                {
                    afd->final.push_back(yytext); 
                }
                
                yy_push_state(FSSEP);
              }
              
    {ws}   ;
}

<FSSEP>{
    "," {
            yy_pop_state();
        }
        
    "})"   {
             BEGIN(INITIAL);
           }
    {ws}   ;
}



%{
 // REGEX
%}

<READ_REGEX>{
    "{"   {
            yy_push_state(ALPHABET_STATE);
          }
}

<EXPRESSION_SEPARATOR>{
    ":" { 
            yy_pop_state();
            yy_push_state(EXPRESSION_STATE);
        }
    {ws}   ;
}

<EXPRESSION_STATE>{
    {symbol}    {
                    expression.push_back(to_symbol(yytext[0]));
                }
    "*" {   expression.push_back(to_symbol('*')); }
    "+" {   expression.push_back(to_symbol('+')); }
    "|" {   expression.push_back(to_symbol('|')); }
    "?" {   expression.push_back(to_symbol('?')); }
    "O" {   expression.push_back(to_symbol('O')); }
    "e" {   expression.push_back(to_symbol('e')); }
    "(" {   expression.push_back(to_symbol('(')); }
    ")" {   expression.push_back(to_symbol(')')); }
    
    . {  BEGIN(INITIAL); }
}
    
%{
 // XML - AFN, AFD
%}


<READ_AFN_XML>{
   
    "<NFA>"    {
                    afn = new AFN();
               }

    "<states>" {
                    yy_push_state(XML_READ_STATES);
               }
                           
    "</NFA>"   {
                   BEGIN(INITIAL);
               }
    {ws}    ;
 }

<READ_AFD_XML>{
    
    "<DFA>"     {
                    afd = new AFD();
                }
              
    "<states>"  {
                    yy_push_state(XML_READ_STATES);
                }
              
    "</DFA>"    {
                   BEGIN(INITIAL);
                }
   
    {ws}        ;
}

<XML_READ_STATE>{
    
    {name}  {
                read_state = yytext;
                yy_pop_state();
            }
    {ws}    ;    
}

<XML_READ_SYMBOL>{
    {symbol}    {
                    read_symbol = yytext;
                    yy_pop_state();
                }
    {ws}    ;  
}

<XML_READ_STATES>{

    "</states>" {
                    yy_pop_state();
                    yy_push_state(XML_READ_ALPHABET);
                }

    "<state>"  {
                    yy_push_state(XML_READ_STATE);
               }
               
    "</state>"  {
                    if(input_representation == _AFN)
                    {
                        afn->addState(read_state);
                    }
                    else
                    {
                        afd->addState(read_state);
                    }
                }
    {ws}    ;
}

<XML_READ_ALPHABET>{
    "<alphabet>"     ;
    
    "</alphabet>"    {
                        yy_pop_state();
                        if(input_representation == _AFN)
                        {
                            afn->set_alphabet(alphabet);
                            yy_push_state(XML_READ_RELATIONS);
                        }
                        else
                        {
                            afd->set_alphabet(alphabet);
                            yy_push_state(XML_READ_TRANSITIONS);
                        }
                    }
                  
    "<symbol>"  {
                    yy_push_state(XML_READ_SYMBOL);
                }
                
    "</symbol>" {
                     alphabet.push_back(read_symbol); 
                }
                
    {ws}    ;
}

<XML_READ_TRANSITION>{
    "<source>" ;
               
    "</source>" {
                    read_source = read_state;
                }
                
    "<destination>" ;
               
    "</destination>" {
                        read_destination = read_state;
                        yy_pop_state(); // am terminat de citit tranzitia
                     }
                
    "<symbol>" {
                    yy_push_state(XML_READ_SYMBOL);
               }
               
    "</symbol>" ;
    
    "<state>"  {
                    yy_push_state(XML_READ_STATE);
               }
               
    "</state>" ;
    
    {ws}    ;
}

<XML_READ_TRANSITIONS>{
    "<delta>"   ;
    "</delta>"  {
                    yy_pop_state();
                    yy_push_state(XML_READ_INITIAL_STATE);
                }
               
    "<transition>"   {
                        yy_push_state(XML_READ_TRANSITION);
                     }
              
    "</transition>"  {
                        afd->addTransition(read_source, read_destination, read_symbol);
                     }
    {ws}    ;
}

<XML_READ_WORD>{
    "e" {
            read_word = "e";
        }
        
    "<symbol>"  {
                    yy_push_state(XML_READ_SYMBOL);
                }
               
    "</symbol>" {   
                    read_word += read_symbol;
                }
                
    "</word>"   {
                    yy_pop_state();
                }
                
    {ws}    ;
} 

<XML_READ_RELATION>{
    "<source>" ;
               
    "</source>" {
                    read_source = read_state;
                }
                
    "<destination>" ;
               
    "</destination>" {
                        read_destination = read_state;
                        yy_pop_state();
                     }
                
    "<word>" {
                    read_word = "";
                    yy_push_state(XML_READ_WORD);
             }
             
    "<state>"  {
                    yy_push_state(XML_READ_STATE);
               }
               
    "</state>" ;
    
    
    {ws}    ;
}

 
<XML_READ_RELATIONS>{
    "<Delta>"  ;
    
    "</Delta>" {
                    yy_pop_state();
                    yy_push_state(XML_READ_INITIAL_STATE);
               }
               
    "<relation>"   {
                        yy_push_state(XML_READ_RELATION);
                   }
              
    "</relation>"  {
                        afn->addTransition(read_source, read_destination, read_word);
                   }
    {ws}    ;
}


<XML_READ_INITIAL_STATE>{
    "<initial>" ;

    "<state>"   {
                    yy_push_state(XML_READ_STATE);
                }
    
    "</state>"  {
                    if(input_representation == _AFN)
                    {
                        afn->start = read_state;
                    }
                    else
                    {
                        afd->start = read_state;
                    }
                 }
                 
    "</initial>" {
                    yy_pop_state();
                    yy_push_state(XML_READ_FINAL_STATES);
                 }
    {ws}    ;               
}


<XML_READ_FINAL_STATES>{
    "<final>" ;
    
    "</final>"  {
                    yy_pop_state();
                }
               
    "<state>"   {
                    yy_push_state(XML_READ_STATE);
                }
   
    "</state>"  {
                    if(input_representation == _AFN)
                    {
                        afn->final.push_back(read_state); 
                    }
                    else
                    {
                        afd->final.push_back(read_state); 
                    }
                }
    {ws}    ;   
}

{ws}    ;

.   ;

%%



void convert(Representation& form, Representation& to);
void read_input(int argc, char* argv[],
                Representation& from, Representation& to,
                Format& from_format, Format& to_format, 
                bool& is_contains_query, int& first_querry_pos);
                

int main(int argc, char* argv[])
{
    Representation from = _NONE;
    Representation to = _NONE;
    bool is_contains_query = false;
    int first_query_pos;    // indicele din argv de unde incepe cuvintele testate cu contains

    // Citesc totul de la stdin intr-un string si i-l dau lui flex sa-l parseze
    std::string line, input;
    while(std::getline(cin, line))
    {
       input += line;
    }
    
    read_input(argc, argv, from, to, from_format, to_format, is_contains_query, first_query_pos);
    input_representation = from;
    
    YY_BUFFER_STATE my_string_buffer = yy_scan_string(input.c_str());
    yy_switch_to_buffer(my_string_buffer);
    yylex();

    // Convertesc regexul citit intr-un abstract syntax tree (flex nu ma poate ajuta aici
    // poate doar Bison ar fi facut treaba mai usoara)
    if(from == _REGEX)
    {
        int poz = 0;
        regex = re(expression, poz);
        regex->set_alphabet(alphabet);
    }

    if(is_contains_query == true)
    {
        to = _AFD;
        convert(from, to);
        for(int i = first_query_pos; i < argc; i++)
        {
            if(afd->contains(argv[i]))
            {
                std::cout << "True" << '\n';
            }
            else
            {
                std::cout << "False" << '\n';
            }
        }
    }
    else
    {
        if(to == _NONE)
        {
            to = from;
        }
        
        convert(from, to);
        if(to == _REGEX)
        {
            regex->format = to_format;
            cout << *regex << '\n';
        }
        else if(to == _AFN)
        {
            afn->format = to_format;
            cout << *afn << '\n';
        }  
        else if(to == _AFD)
        {
            afd->format = to_format;
            cout << *afd << '\n';
        }
    }
    
    return 0;
}

void read_input(int argc, char* argv[],
                Representation& from, Representation& to,
                Format& from_format, Format& to_format, 
                bool& is_contains_query, int& first_query_pos)
{
    for(int i = 1; i < argc; i++)
    {
        if(argv[i] == string("--from-RE"))
        {
            from = _REGEX;
        }
        else if(argv[i] == string("--from-DFA"))
        {
            from = _AFD;
        }
        else if(argv[i] == string("--from-NFA"))
        {
            from = _AFN;
        }
        else if(argv[i] == string("--to-RE"))
        {
            to = _REGEX;
        }
        else if(argv[i] == string("--to-DFA"))
        {
            to = _AFD;
        }
        else if(argv[i] == string("--to-NFA"))
        {
            to = _AFN;
        }
        else if(argv[i] == string("--from-XML"))
        {
            from_format = XML;
        }
        else if(argv[i] == string("--to-XML"))
        {
            to_format = XML;
        }
        else if(argv[i] == string("--contains"))
        {
            is_contains_query = true;
            first_query_pos = i + 1;
            break;
        }
    }
}

void convert(Representation& from, Representation& to)
{
    Representation start_state = from;
    Representation finish_state = to;
    Representation current_state = start_state;
    
    while(current_state != finish_state)
    {
        if(current_state == _REGEX)
        {
            afn = regex_to_afn(regex);
            current_state = _AFN;
        }
        else if(current_state == _AFN)
        {
            afd = afn_to_afd(afn);
            current_state = _AFD;
        }
        else if(current_state == _AFD && finish_state == _AFN)
        {
            afn = dynamic_cast<AFN*>(afd);
            current_state = _AFN;
        }
        else if(current_state == _AFD)
        {
            regex = afd_to_regex(afd);
            current_state = _REGEX;
        }

    }
}

void yyerror(const char *s) {
    cout << s;
}
