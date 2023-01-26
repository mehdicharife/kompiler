


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
    LexemeType type;
    char* content;
    place location;
} typedef Lexeme;

int lex(char* code, Lexeme** pplexemes);