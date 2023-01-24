#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include <stdarg.h>


#define RULES_COUNT 4



enum SymbolType {
    TERMINAL,
    NONTERMINAL
} typedef SymbolType;

struct Symbol {
    char label;
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





int set_symbol_rules(Symbol* pS, Rule* grules, Rule** psrules) {
    int srules_count = 0;

    for(int r = 0; r < RULES_COUNT; r++) {
        if(grules[r].pleft == pS) {
            psrules[srules_count] = grules + r;
            srules_count++;
        }
    }

    //printf("%c: %d\n", pS->label, srules_count);

    return srules_count;
}



int devolves_to_eps(Symbol* pS, Rule* grules) {
    Rule** psrules = malloc(RULES_COUNT*sizeof(Rule*));
    int srules_count = set_symbol_rules(pS, grules, psrules);

    for(int i = 0; i < srules_count; i++) {
        if(psrules[i]->prights[0]->label = 'o') { // # here denotes epsilon
            return 1;
        }
    }

    return 0;
}



void pFIRSTS(Symbol* pS, Rule* grules, Symbol* pfirsts[], int* pfirsts_count) {
    printf("Symbol under treatment: %c\n", pS->label);
    Rule** psrules = malloc(RULES_COUNT*sizeof(Rule*));
    int srules_count = set_symbol_rules(pS, grules, psrules);


    for(int k = 0; k < srules_count; k++) {
        Symbol* pY = psrules[k]->prights[0];
        printf("%c: %c\n", pS->label, pY->label);
        if(pY->type == TERMINAL) {
            pfirsts[*pfirsts_count] = pY;
            ++*pfirsts_count;
            printf("New FIrstt: %c\n", pfirsts[*pfirsts_count - 1]->label);
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



int main(int argc, char* argv[]) {
    Symbol S, E, K, a, eps, b;
    S.label = 'S';
    S.type = NONTERMINAL;

    E.label = 'E';
    E.type = NONTERMINAL;

    K.label = 'K';
    K.type = NONTERMINAL;

    eps.label = 'o';
    eps.type = TERMINAL;


    a.label = 'a';
    a.type = TERMINAL;

    b.label = 'b';
    b.type = TERMINAL;

    Rule* grules = malloc(4*sizeof(Rule));

    set_rule(&grules[0], &S, 2, &E, &K);
    set_rule(&grules[1], &E, 1, &a);
    set_rule(&grules[2], &E, 1, &eps);
    set_rule(&grules[3], &K, 1, &b);


    Symbol** pfirsts = malloc(20*sizeof(Symbol*));
    int firsts_count = 0; 
    pFIRSTS(&S, grules, pfirsts, &firsts_count);


    printf("FIRSTS: %d\n", firsts_count);

    for(int k = 0; k < firsts_count; k++) {
        printf("%c\n", pfirsts[k]->label);
    }


    return 0;

}