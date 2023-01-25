#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include <stdarg.h>
#include <unistd.h>

#define RULES_COUNT 8



int streq(const void* str1, const void* str2) {
    return !strcmp((char*) str1, (char*) str2);
}

int ptreq(void** ptr1, void* ptr2) {
    return (*ptr1 == ptr2);
}

int is_in(void* list, int size, void* pelement, int (*equals)(void** this, void* that)) {
    for(int k = 0; k < size; k++) {
        if(equals(list + k, pelement)) {
            return 1;
        }
    }
    return 0;
}

/*init_set_from_array(void* set, void* array, int size) {
*/







enum SymbolType {
    TERMINAL,
    NONTERMINAL
} typedef SymbolType;

struct Symbol {
    char* content;
    SymbolType type;
} typedef Symbol;

struct Rule {
    Symbol* pleft;
    Symbol** prights;
    int rights_count;
} typedef Rule;

struct Grammar {
    Symbol* pstart;
    Rule* rules;
} typedef Grammar;


void set_rule(Rule* prule, Symbol* pleft, int rights_count, ... ) {
    prule->pleft = pleft;
    prule->rights_count = rights_count;
    prule->prights = malloc(rights_count*sizeof(Symbol*));

    va_list args;
    va_start(args, rights_count);

    for(int i = 0; i < rights_count; i++) {
        prule->prights[i] = va_arg(args, Symbol*);
    }

    va_end(args);
}


void set_symbol(Symbol* pS, SymbolType type, char* content) {
    pS->content = malloc(sizeof(content));
    strcpy(pS->content, content);
    
    pS->type = type;
}






int set_symbol_rules(Symbol* pS, Rule* grules, Rule** psrules) {
    int srules_count = 0;

    for(int r = 0; r < RULES_COUNT; r++) {
        if(grules[r].pleft == pS) {
            psrules[srules_count] = grules + r;
            srules_count++;
        }
    }

    //printf("%c: %d\n", pS->content, srules_count);

    return srules_count;
}


int in_right_of(Symbol* pS, Rule rule) {
    int index = -1;

    for(int k = 0; k < rule.rights_count; k++) {
        if(rule.prights[k] == pS) {
            index = k + 1;
        }
    }

    return index;
}



int devolves_to_eps(Symbol* pS, Rule* grules) {
    Rule** psrules = malloc(RULES_COUNT*sizeof(Rule*));
    int srules_count = set_symbol_rules(pS, grules, psrules);

    for(int i = 0; i < srules_count; i++) {
        if(!strcmp(psrules[i]->prights[0]->content, "#")) { // # here denotes epsilon
            return 1;
        }
    }

    return 0;
}



void pFIRSTS(Symbol* pS, Rule* grules, Symbol* pfirsts[], int* pfirsts_count) {

    Rule** psrules = malloc(RULES_COUNT*sizeof(Rule*));
    int srules_count = set_symbol_rules(pS, grules, psrules);


    for(int k = 0; k < srules_count; k++) {
        Symbol* pY = psrules[k]->prights[0];
        if(pY->type == TERMINAL) {
            pfirsts[*pfirsts_count] = pY;
            ++*pfirsts_count;
            
            continue;
        }

        pFIRSTS(pY, grules, pfirsts, pfirsts_count);
        for(int l = 0; l + 1 < psrules[k]->rights_count; l++) {
            if(devolves_to_eps(psrules[k]->prights[l], grules)) {
                pFIRSTS(psrules[k]->prights[l + 1], grules, pfirsts, pfirsts_count);
            } else {
                break;
            }
        }
        
    }
}


void pFOLLOWS(Symbol* pS, Rule* grules, Symbol** ppfollows, int* pfollows_count) {
    //printf("Symbol under treatment: %s %p\n", pS->content, (void*)pS);
    for(int k = 0; k < RULES_COUNT; k++) {
        int index;
        
        if((index = in_right_of(pS, grules[k])) != -1) {
            if(index < grules[k].rights_count) {
                if(grules[k].prights[index]->type == TERMINAL && !is_in(ppfollows, *pfollows_count, grules[k].prights[index], &ptreq)) {
                    ppfollows[*pfollows_count] = grules[k].prights[index];
                    ++*pfollows_count;
                } else {
                    Symbol** ppfirsts = malloc(20*sizeof(Symbol*));
                    int firsts_count = 0;
                    int r = 0;

                    do {
                        if(index + r < grules[k].rights_count) {
                            if(grules[k].prights[index + r]->type == TERMINAL && !is_in(ppfollows, *pfollows_count, grules[k].prights[index + r], &ptreq)) {
                                ppfollows[*pfollows_count] = grules[k].prights[index + r];
                                ++*pfollows_count;
                                break;
                            }

                            pFIRSTS(grules[k].prights[index + r], grules, ppfirsts, &firsts_count);
                            r++;
                            continue;
                        }
                        if(grules[k].pleft != pS) {
                            pFOLLOWS(grules[k].pleft, grules, ppfollows, pfollows_count);
                            break;
                        }
                    } while(((index + r) <= grules[k].rights_count)  && devolves_to_eps(grules[k].prights[index + r - 1], grules));

                    for(int s = 0; s < firsts_count; s++) {
                        if(!is_in(ppfollows, *pfollows_count, ppfirsts[s], &ptreq)) {
                            ppfollows[*pfollows_count] = ppfirsts[s];
                            ++*pfollows_count;
                        }

                    }

                }
            } else if(grules[k].pleft != pS) {
                pFOLLOWS(grules[k].pleft, grules, ppfollows, pfollows_count);
            }

        }
    }
}


int main(int argc, char* argv[]) {

    Symbol E, _E, T, _T, F, plus, times, lpar, rpar, intype, eps;

    set_symbol(&E, NONTERMINAL, "E");
    set_symbol(&_E, NONTERMINAL, "_E");
    set_symbol(&T, NONTERMINAL, "T");
    set_symbol(&_T, NONTERMINAL, "_T");
    set_symbol(&F, NONTERMINAL, "F");

    set_symbol(&eps, TERMINAL, "#");
    set_symbol(&plus, TERMINAL, "+");
    set_symbol(&times, TERMINAL, "*");
    set_symbol(&lpar, TERMINAL, "(");
    set_symbol(&rpar, TERMINAL, ")");
    set_symbol(&intype, TERMINAL, "int");



    Rule* grules = malloc(RULES_COUNT*sizeof(Rule));
    set_rule(&grules[0], &E, 2, &T, &_E);
    set_rule(&grules[1], &_E, 3, &plus, &T, &_E);
    set_rule(&grules[2], &_E, 1, &eps);
    set_rule(&grules[3], &T, 2, &F, &_T);
    set_rule(&grules[4], &_T, 3, &times, &F, &_T);
    set_rule(&grules[5], &_T, 1, &eps);    
    set_rule(&grules[6], &F, 3, &lpar, &E, &rpar);
    set_rule(&grules[7], &F, 1, &intype);

    

    Symbol** ppfollows = malloc(20*sizeof(Symbol*));
    int follows_count = 0;
    
    pFOLLOWS(&F, grules, ppfollows, &follows_count);

    
    for(int k = 0; k < follows_count; k++) {
        printf("%d: %s\n", k, ppfollows[k]->content);
    }


}