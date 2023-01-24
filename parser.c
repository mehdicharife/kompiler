#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include <stdarg.h>


#define RULES_COUNT 8



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

    //printf("Symbol under treatment: %s\n", pS->content);


    Rule** psrules = malloc(RULES_COUNT*sizeof(Rule*));
    int srules_count = set_symbol_rules(pS, grules, psrules);


    for(int k = 0; k < srules_count; k++) {
        Symbol* pY = psrules[k]->prights[0];
        if(pY->type == TERMINAL) {
            //printf("%s: %s\n", pS->content, pY->content);
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


/*
int test() {
    Symbol S, E, K, a, eps, b, c;

    set_symbol(&S, NONTERMINAL, "S");
    set_symbol(&E, NONTERMINAL, "E");
    set_symbol(&K, NONTERMINAL, "K");
    set_symbol(&eps, TERMINAL, "#");
    set_symbol(&a, TERMINAL, "a");
    set_symbol(&b, TERMINAL, "b");
    set_symbol(&c, TERMINAL, "c");


    Rule* grules = malloc(RULES_COUNT*sizeof(Rule));

    set_rule(&grules[0], &S, 2, &E, &K);
    set_rule(&grules[1], &E, 1, &a);
    set_rule(&grules[2], &E, 1, &eps);
    set_rule(&grules[3], &K, 1, &b);
    set_rule(&grules[4], &K, 1, &c);


    Symbol** pfirsts = malloc(20*sizeof(Symbol*));
    int firsts_count = 0; 
    pFIRSTS(&S, grules, pfirsts, &firsts_count);


    for(int k = 0; k < firsts_count; k++) {
        printf("%d: %s\n", k, pfirsts[k]->content);
    }


    return 0;

}
*/


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

    

    Symbol** pfirsts = malloc(20*sizeof(Symbol*));
    int firsts_count = 0;

    pFIRSTS(&F, grules, pfirsts, &firsts_count);

    

    for(int k = 0; k < firsts_count; k++) {
        printf("%d: %s\n", k, pfirsts[k]->content);
    }

}