#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include <stdarg.h>
#include <unistd.h>

#include "lexer.h"

#define RULES_COUNT 8



int is_in(void* list, int size, void* pelement, int (*equals)(void* this, int k, void* that)) {
    for(int k = 0; k < size; k++) {
        if(equals(list, k, pelement)) {
            return 1;
        }
    }
    return 0;
}


struct node {
    void* pcontent;
    struct node* next;
} typedef node;



void push(node** pstack, void* pcontent) {
    node* pnewhead = malloc(sizeof(node));
    pnewhead->pcontent = pcontent;
    pnewhead->next = *pstack;

    *pstack = pnewhead;
}

void* pop(node** pstack) {
    node* pnewtop = (*pstack)->next;
    void* oldtopcontent = (*pstack)->pcontent;
    free(*pstack);
    *pstack = pnewtop;

    return oldtopcontent;
}

void* top(node** pstack) {
    return (*pstack)->pcontent;
}

void* bottom(node** pstack) {
    node* pcurrent_node = *pstack;

    while(pcurrent_node->next) {
        pcurrent_node = pcurrent_node->next;
    }

    return pcurrent_node->pcontent;
}

void* push_many(node** pstack, int nodes_count, ...){

    va_list args;
    va_start(args, nodes_count);

    for(int k = 0; k < nodes_count; k++) {
        push(pstack, va_arg(args, void*));
    }

    va_end(args);
};

void push_array(node** pstack, void** array, int array_size, void (*push_from_index_of)(node** pstak, int k, void** array)) {
    for(int k = 0; k < array_size; k++) {
        push_from_index_of(pstack, k, array);
    }
}



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



// globals:
Symbol dollar = {"$", TERMINAL};




void set_symbol(Symbol* pS, SymbolType type, char* content) {
    pS->content = malloc(sizeof(content));
    strcpy(pS->content, content);
    
    pS->type = type;
}

void push_from_index_of_psymbol_array(node** pstack, int k, void** array) {
    push(pstack, ((Symbol**) array)[k]);
}

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

void print_rule(Rule* prule) {
    printf("%s -> ", prule->pleft->content);
    for(int k = 0; k < prule->rights_count; k++) {
        printf(" %s", prule->prights[k]->content);
    }
    
    printf("\n");
}

int streq(const void* str1, const void* str2) {
    return !strcmp((char*) str1, (char*) str2);
}

int ptreq(void* ptr1, int k, void* ptr2) {
    return (*(((Symbol**) ptr1) + k) == (Symbol*) ptr2);
}





int set_symbol_rules(Symbol* pS, Rule* grules, Rule** psrules) {
    int srules_count = 0;

    for(int r = 0; r < RULES_COUNT; r++) {
        if(grules[r].pleft == pS) {
            psrules[srules_count] = grules + r;
            srules_count++;
        }
    }

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



Rule* devolves_to_eps(Symbol* pS, Rule* grules) {
    Rule** psrules = malloc(RULES_COUNT*sizeof(Rule*));
    int srules_count = set_symbol_rules(pS, grules, psrules);

    for(int i = 0; i < srules_count; i++) {
        if(!strcmp(psrules[i]->prights[0]->content, "#")) { // # here denotes epsilon
            return psrules[i];
        }
    }

    return NULL;
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


Rule* analysis_cell(Symbol* pNT, Symbol* pT, Rule* grules) {
    Rule** psrules = malloc(RULES_COUNT*sizeof(Rule*));
    int srules_count = set_symbol_rules(pNT, grules, psrules);

    for(int k = 0; k < srules_count; k++) {
        int i = 0;
        do {
            int firsts_count = 0;
            Symbol** ppfirsts = malloc(20*sizeof(Symbol));
            pFIRSTS(psrules[k]->prights[i], grules, ppfirsts, &firsts_count);

            if(is_in(ppfirsts, firsts_count, pT, &ptreq)) {
                return psrules[k];
            }

            i++;

            free(ppfirsts);
            firsts_count = 0;

        } while( i < psrules[k]->rights_count && devolves_to_eps(psrules[k]->prights[i - 1], grules));

        Rule* roll;

        if((roll = devolves_to_eps(pNT, grules))) {
            int follows_count = 0;
            Symbol** ppfollows = malloc(20*sizeof(Symbol*));
            pFOLLOWS(pNT, grules, ppfollows, &follows_count);

            if(is_in(ppfollows, follows_count, pT, &ptreq)) {
                free(ppfollows);
                return roll;
            }
        }

        return NULL;

    }
}



int verify_rect(node** psymbol_stack, node** plexeme_stack, Rule* grules) {
    if(top(psymbol_stack) == &dollar){
        return (top(plexeme_stack) == &dollar);
    }

    if(top(psymbol_stack) == top(plexeme_stack)) {
        pop(psymbol_stack);
        pop(plexeme_stack);

        return verify_rec(psymbol_stack, plexeme_stack, grules);
    }

    Rule* prule = analysis_cell(top(psymbol_stack), top(plexeme_stack), grules);
    if(!prule) {
        return 0;
    } 

    pop(psymbol_stack);
    push_array(psymbol_stack, prule->prights, prule->rights_count);
    return verify_rec(psymbol_stack, plexeme_stack);
}


int verify(Rule* grules, Symbol* pS, char* statement) {
    node* psymbol_stack = malloc(sizeof(node*));
    node* ptoken_stack = malloc(sizeof(node*));

    // push dollar sign
    push(psymbol_stack, pS);

    Lexeme** pplexemes = malloc(strlen(statement)*sizeof(Lexeme*));
    int lexemes_count = lex(statement, pplexemes);
    // push dollar sign
    push_array(ptoken_stack, pplexemes, lexemes_count);


    return verify_rec(psymbol_stack, ptoken_stack);

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

    
    /*Symbol** ppfollows = malloc(20*sizeof(Symbol*));
    int follows_count = 0;
    
    pFOLLOWS(&F, grules, ppfollows, &follows_count);

    printf("FOLLOWS(%s):\n\n", F.content);
    for(int k = 0; k < follows_count; k++) {
        printf("%d: %s\n", k, ppfollows[k]->content);
    }*/

    /*Rule* prula = analysis_cell(&E, &lpar, grules);
    if(prula) {
        print_rule(prula);
    }*/
    


    node** psymbol_stack = malloc(sizeof(node*));
    node** ptoken_stack = malloc(sizeof(node*));

    node nE, nT, nF;
    node nt1, nt2, nt3;


    push_many(psymbol_stack, 3, &F, &T, &E);
    push_many(ptoken_stack,2, "jkjk", "k");

    printf("Popped: %s\nNew top: %s\n", ((Symbol*) pop(psymbol_stack))->content, ((Symbol*) top(psymbol_stack))->content);

    printf("Popped: %s\nNew top: %s\n", ((char*) pop(ptoken_stack)), ((char*) top(ptoken_stack)));
    printf("Popped: %s\nNew top: %s\n", ((char*) pop(ptoken_stack)), ((char*) top(ptoken_stack)));

    return 0;
}