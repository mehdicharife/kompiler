#include <stdio.h>
#include <stdlib.h>
#include "string.h"

#define KEYWORDS_NUMBER 3
#define DOPERATORS_NUMBER 10

typedef struct Token Token;
typedef struct Lexeme Lexeme;
typedef struct place place;
typedef enum LexemeType LexemeType;

char* keywords[] = {"int", "chad", "return"};

char separators[] = {'(',')', '[', ']', ',', ';'};
/*char operators[] = {'+', '-', '*', '/', '%', // Arithmetic operators
                    '&', '|', '^', '~', '<<', '>>', // Bitwise operators
                    '&&', '||', '!', // Logical operators
                    '==', '!=', '>', '<', '>=', '<=', // Relational operators
                    '=' 
};*/

char soperators[] = {'+', '-', '*', '/', '%', '=', '!','&', '|', '^', '~','<'};
char* doperators[] = {"++", "--", "==", "+=", "-=", "<<", ">>", "<=", ">=", "!="};


struct place {
    int line;
    int col;
};


enum LexemeType {
    identifier,
    keyword,
    separator,
    operator,
    string_literal,
    numeric_constant,
    comment, 
};

struct Lexeme {
    LexemeType type;
    char* content;
    place location;
};




// Helper functions


char* typeNames[] = {"identifier", "keyword", "separator", "operator", "string_literal", "numeric_constant", "comment"};
 






int belongs(char** words, int size, char* word) {
    int k = 0;
    for(int k = 0; k < size; k++) {
        if(!strcmp(word,words[k])) {
            return 1;
        }
    }

    return 0;
}



int is_partof_rec(char* set, char c, int k) {
    if(set[k] == c) {
        return 1;
    } else if(k == strlen(set) - 1){
        return 0;
    } else {
        return  is_partof_rec(set,c, k + 1);
    }
}

int is_partof(void* set, char c) {
    return is_partof_rec(set, c, 0);
}







int is_alpha(char c) {
    if(c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z') {
        return 1;
    }
    return 0;
}


int is_digit(char c) {
    if(c >= '0' && c <= '9') {
        return 1;
    }
    return 0;
}


int is_separator(char c) {
    return is_partof(separators, c);
}


int is_operator(char c) {
    return is_partof(soperators, c);
}

int is_doperator(char* diquence) {
    return belongs(doperators, DOPERATORS_NUMBER, diquence);
}

int is_keyword(char* sequence) {
    return belongs(keywords, KEYWORDS_NUMBER, sequence);
}












int fits_token_pattern(char* code, int start, int index) {
    if(is_alpha(code[start])) {
        return is_alpha(code[index]) || is_digit(code[index]);
    }

    if(is_digit(code[start])) {
        if(index == strlen(code)) {
            return 1;
        }

        return is_digit(code[index]);
    }

    if(code[start] == '\'') {
        return (code[index - 1] != '\'' || index == 1);
    }

    if(is_operator(code[index])) {
        char c = code[index + 1];
        int goon = 0;
        code[index + 1] = '\0';
        if(is_doperator(code + start)) {
            goon = 1;
        }
        code[index + 1] = c;
        return goon;
    }

    return 0;
}




LexemeType get_read_lex_type(char* lexeme_content) {
    if(is_alpha(lexeme_content[0])) {
        if(is_keyword(lexeme_content)) {
            return keyword;
        }

        return identifier;
    }

    if(is_digit(lexeme_content[0])) {
        return numeric_constant;
    }

    if(is_separator(lexeme_content[0])) {
        return separator;
    }

    if(is_operator(lexeme_content[0])) {
        return operator;
    }

}


Lexeme* readLexeme(char* code, int index) {
    //int i = 0;
    //while(is_insignificant(code[index + i])) {i++;}
    Lexeme* plexeme = malloc(sizeof(Lexeme));
    plexeme->content = malloc(sizeof(strlen(code)));

    int p = 0;
    char* lexeme_content = malloc(1000*sizeof(char));

    do {
        lexeme_content[p] = code[index + p];
        p++;
        if(index + p >= strlen(code)) {
            break;
        }
    } while(fits_token_pattern(code, index, index + p) && (index + p) < strlen(code));
    lexeme_content[p] = '\0';

         
    strcpy(plexeme->content, lexeme_content);
    

    plexeme->type = get_read_lex_type(lexeme_content);

    return plexeme;
}








int lex(char* code) {
    Lexeme** pplexemes = malloc(strlen(code)*sizeof(Lexeme*));
    int k = 0, col = 0, lexeme_count = 0;
    int line = 1;


    while(k < strlen(code)) {

        if(code[k] == ' ') {
            k++;
            continue;
        }

        if(code[k] == '\n') {
            printf("newLine\n\n\n");
            line++;
            col = 0;
            k++;
            continue;
        }

        Lexeme* plexeme = readLexeme(code, k);
        
        plexeme->location.line = line;
        plexeme->location.col = col;
        pplexemes[lexeme_count] = plexeme;
        
        lexeme_count++;


        if(k + strlen(plexeme->content) >= strlen(code)) {
            break;
        }


        k = k + strlen(plexeme->content);
        col = col + strlen(plexeme->content);
    }


    printf("Lexemes Total: %d\n===================\n\n", lexeme_count);
    for(int k = 0; k < lexeme_count; k++) {
        printf("%s: %s\n", typeNames[pplexemes[k]->type], pplexemes[k]->content);
    }
    
    
}


int main(int argc, char* argv[]) {
    char code[] = "int main(int argc) {\n    int k += 0;\nreturn 0;}";

    lex(code);

    return 0;
}
