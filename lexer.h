
#include "parser.h"

struct place {
    int line;
    int col;
} typedef place;


enum LexemeType {
    identifier,
    keyword,
    separator,
    grroperator,
    string_literal,
    numeric_constant,
    comment,
} typedef LexemeType;

struct Lexeme {
    Symbol* psymbol;
    char* content;
    place location;
} typedef Lexeme;


int set_from_code(char* code, CFG* pgrammar, void** ppthings, int lexemes);

