#include <stdlib.h>
#include <stdio.h>
#include "string.h"
#include <stdarg.h>
#include <unistd.h>

#include "lexer.h"
#include "parser.h"



int set_from_code(char* code, CFG* pgrammar, void** ppthings, int lexemes);
 
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
    for(int k = array_size - 1; k > -1; k--) {
        push_from_index_of(pstack, k, array);
    }
}

char* get_symbol_as_str(void* pS) {
    return ((Symbol*) pS)->content;
}

void print_stack(node** pstack, char*(*get_as_str)(void*)) {
    node* pcurrent_node = *pstack;

    do {
        printf("%s ", get_as_str(pcurrent_node->pcontent));
        pcurrent_node = pcurrent_node->next;
    }while(pcurrent_node != NULL);

    printf("\n");
}








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





int set_symbol_rules(Symbol* pS, CFG* pgrammar, Rule** psrules) {
    int srules_count = 0;

    for(int r = 0; r < pgrammar->rules_count ; r++) {
        if(pgrammar->rules[r].pleft == pS) {
            psrules[srules_count] = pgrammar->rules + r;
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



Rule* devolves_to_eps(Symbol* pS, CFG* pgrammar) {
    Rule** psrules = malloc((pgrammar->rules_count)*sizeof(Rule*));
    int srules_count = set_symbol_rules(pS, pgrammar, psrules);

    for(int i = 0; i < srules_count; i++) {
        if(!strcmp(psrules[i]->prights[0]->content, "#")) { // # here denotes epsilon
            return psrules[i];
        }
    }

    return NULL;
}



void pFIRSTS(Symbol* pS, CFG* pgrammar, Symbol* pfirsts[], int* pfirsts_count) {

    Rule** psrules = malloc((pgrammar->rules_count)*sizeof(Rule*));
    int srules_count = set_symbol_rules(pS, pgrammar, psrules);


    for(int k = 0; k < srules_count; k++) {
        Symbol* pY = psrules[k]->prights[0];
        if(pY->type == TERMINAL) {
            pfirsts[*pfirsts_count] = pY;
            ++*pfirsts_count;
            
            continue;
        }

        pFIRSTS(pY, pgrammar, pfirsts, pfirsts_count);
        for(int l = 0; l + 1 < psrules[k]->rights_count; l++) {
            if(devolves_to_eps(psrules[k]->prights[l], pgrammar)) {
                pFIRSTS(psrules[k]->prights[l + 1], pgrammar, pfirsts, pfirsts_count);
            } else {
                break;
            }
        }
        
    }
}


void pFOLLOWS(Symbol* pS, CFG* pgrammar, Symbol** ppfollows, int* pfollows_count) {
    if(pgrammar->pstart == pS && !is_in(ppfollows, *pfollows_count, pS, &ptreq)) {
        ppfollows[*pfollows_count] = &dollar;
        ++*pfollows_count;

    }

    for(int k = 0; k < pgrammar->rules_count; k++) {
        int index;
        
        if((index = in_right_of(pS, pgrammar->rules[k])) != -1) {
            if(index < pgrammar->rules[k].rights_count) {
                if(pgrammar->rules[k].prights[index]->type == TERMINAL && !is_in(ppfollows, *pfollows_count, pgrammar->rules[k].prights[index], &ptreq)) {
                    ppfollows[*pfollows_count] = pgrammar->rules[k].prights[index];
                    ++*pfollows_count;
                } else {
                    Symbol** ppfirsts = malloc(20*sizeof(Symbol*));
                    int firsts_count = 0;
                    int r = 0;

                    do {
                        if(index + r < pgrammar->rules[k].rights_count) {
                            if(pgrammar->rules[k].prights[index + r]->type == TERMINAL && !is_in(ppfollows, *pfollows_count, pgrammar->rules[k].prights[index + r], &ptreq)) {
                                ppfollows[*pfollows_count] = pgrammar->rules[k].prights[index + r];
                                ++*pfollows_count;
                                break;
                            }

                            pFIRSTS(pgrammar->rules[k].prights[index + r], pgrammar, ppfirsts, &firsts_count);
                            r++;
                            continue;
                        }
                        if(pgrammar->rules[k].pleft != pS) {
                            pFOLLOWS(pgrammar->rules[k].pleft, pgrammar, ppfollows, pfollows_count);
                            break;
                        }
                    } while(((index + r) <= pgrammar->rules[k].rights_count)  && devolves_to_eps(pgrammar->rules[k].prights[index + r - 1], pgrammar));

                    for(int s = 0; s < firsts_count; s++) {
                        if(!is_in(ppfollows, *pfollows_count, ppfirsts[s], &ptreq)) {
                            ppfollows[*pfollows_count] = ppfirsts[s];
                            ++*pfollows_count;
                        }

                    }

                }
            } else if(pgrammar->rules[k].pleft != pS) {
                pFOLLOWS(pgrammar->rules[k].pleft, pgrammar, ppfollows, pfollows_count);
            }

        }
    }
}


Rule* analysis_cell(Symbol* pNT, Symbol* pT, CFG* pgrammar) {
    Rule** psrules = malloc((pgrammar->rules_count)*sizeof(Rule*));
    int srules_count = set_symbol_rules(pNT, pgrammar, psrules);

    for(int k = 0; k < srules_count; k++) {
        if(psrules[k]->prights[0]->type == TERMINAL) {
            if(psrules[k]->prights[0] == pT) {
                return psrules[k];
            }

            continue;
        }


        int i = 0;
        do {
            int firsts_count = 0;
            Symbol** ppfirsts = malloc(20*sizeof(Symbol));
            pFIRSTS(psrules[k]->prights[i], pgrammar, ppfirsts, &firsts_count);

            if(is_in(ppfirsts, firsts_count, pT, &ptreq)) {
                return psrules[k];
            }

            i++;

            free(ppfirsts);
            firsts_count = 0;

        } while(i < psrules[k]->rights_count && devolves_to_eps(psrules[k]->prights[i - 1], pgrammar));

    }


    Rule* roll;
    if((roll = devolves_to_eps(pNT, pgrammar))) {
        int follows_count = 0;
        Symbol** ppfollows = malloc(20*sizeof(Symbol*));
        pFOLLOWS(pNT, pgrammar, ppfollows, &follows_count);

        if(is_in(ppfollows, follows_count, pT, &ptreq)) {
            free(ppfollows);
            return roll;
        }
    }

    
    return NULL;
}



int verify_rec(node** psymbol_stack, node** plexeme_stack, CFG* pgrammar) {
    if(top(psymbol_stack) == &dollar){
        return (top(plexeme_stack) == &dollar);
    }

    if(top(psymbol_stack) == top(plexeme_stack)) {
        pop(psymbol_stack);
        pop(plexeme_stack);

        return verify_rec(psymbol_stack, plexeme_stack, pgrammar);
    }

    Rule* prule = analysis_cell(top(psymbol_stack), top(plexeme_stack), pgrammar);
    if(!prule) {
        return 0;
    } 

    pop(psymbol_stack);
    push_array(psymbol_stack, (void**) prule->prights, prule->rights_count, &push_from_index_of_psymbol_array);


    return verify_rec(psymbol_stack, plexeme_stack, pgrammar);
}


int verify(char* statement, CFG* pgrammar) {
    node** psymbols_stack = malloc(sizeof(node*));
    node** pterminals_stack = malloc(sizeof(node*));

    
    push(psymbols_stack, &dollar);
    push(psymbols_stack, pgrammar->pstart);
    

    Symbol** ppterminals = malloc(strlen(statement)*sizeof(Symbol*));
    int terminals_count = set_from_code(statement, pgrammar, ((void**) ppterminals), 0);
    
    push(pterminals_stack, &dollar);
    push_array(pterminals_stack, (void**) ppterminals, terminals_count, &push_from_index_of_psymbol_array);


    return verify_rec(psymbols_stack, pterminals_stack, pgrammar);

}



int main(int argc, char* argv[]) {
    Symbol decl, type, id, operator, numc, _int, _char, assign, sep, semicolon;

    set_symbol(&decl, NONTERMINAL, "declaration");
    set_symbol(&type, NONTERMINAL, "type");
    set_symbol(&id, TERMINAL, "identifier");
    set_symbol(&operator, NONTERMINAL, "operator");
    set_symbol(&numc, TERMINAL, "numeric_constant");
    set_symbol(&_int, TERMINAL, "int");
    set_symbol(&_char, TERMINAL, "char");
    set_symbol(&assign, TERMINAL, "=");
    set_symbol(&sep, NONTERMINAL, "separator");
    set_symbol(&semicolon, TERMINAL, ";");


    Rule* grules = malloc(sizeof(Rule)*5);

    set_rule(&grules[0], &decl, 5, &type, &id, &operator, &numc, &semicolon);
    set_rule(&grules[1], &type, 1, &_int);
    set_rule(&grules[2], &type, 1, &_char);
    set_rule(&grules[3], &operator, 1, &assign);
    set_rule(&grules[4], &sep, 1, &semicolon);


    CFG* pgrammar = malloc(sizeof(CFG));
    pgrammar->rules = grules;
    pgrammar->rules_count = 5;

    pgrammar->symbols_count = 10;

    pgrammar->psymbols = malloc((pgrammar->symbols_count)*sizeof(Symbol*));
    pgrammar->psymbols[0] = &decl;
    pgrammar->psymbols[1] = &type;
    pgrammar->psymbols[2] = &id;
    pgrammar->psymbols[3] = &operator;
    pgrammar->psymbols[4] = &numc;
    pgrammar->psymbols[5] = &_int;
    pgrammar->psymbols[6] = &_char;
    pgrammar->psymbols[7] = &assign;
    pgrammar->psymbols[8] = &sep;
    pgrammar->psymbols[9] = &semicolon;

    
    pgrammar->pstart = &decl;
    pgrammar->id_pos = 2;
    pgrammar->numc_pos = 4;


    char* statement = "int hhhhhhh = 78954;";

    Symbol** ppsymbols = malloc(strlen(statement)*sizeof(Symbol*));
    int symbols_count = set_from_code(statement, pgrammar, (void**) ppsymbols, 0);
    
    for(int k = 0; k < symbols_count; k++) {
        printf("%s ", ppsymbols[k]->content);
    }
    printf("\n");

    printf("%d\n", verify(statement, pgrammar));

}






