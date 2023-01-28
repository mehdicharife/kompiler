#ifndef PARSER_H

#define PARSER_H

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


struct CFG {
    Symbol* pstart;
    Symbol** psymbols;
    int symbols_count;
    int numc_pos;
    int id_pos;
    
    Rule* rules;
    int rules_count;

} typedef CFG;


#endif