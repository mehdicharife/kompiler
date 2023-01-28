#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "string.h"
#include "lexer.h"


#define KEYWORDS_NUMBER 3
#define DOPERATORS_NUMBER 10
#define SEPARATORS_COUNT 6
#define OPERATORS_COUNT 22


char* keywords[] = {"int", "chad", "return"};


/*char operators[] = {'+', '-', '*', '/', '%', // Arithmetic operators
                    '&', '|', '^', '~', '<<', '>>', // Bitwise operators
                    '&&', '||', '!', // Logical operators
                    '==', '!=', '>', '<', '>=', '<=', // Relational operators
                    '=' 
};*/

//char soperators[] = {'+', '-', '*', '/', '%', '=', '!','&', '|', '^', '~','<'};
//char* doperators[] = {"++", "--", "==", "+=", "-=", "<<", ">>", "<=", ">=", "!="};

char* separators[] = {"(",")", "[", "]", ",", ";"};
char* operators[] = {"+", "-", "*", "/", "%", "=", "!", "&", "|", "^", "~", "<", "++", "--", "==", "+=", "-=", "<<", ">>", "<=", ">=", "!="};


char** sep_ops[] = {separators, operators};
// Symbol




char* typeNames[] = {"identifier", "keyword", "separator", "operator", "string_literal", "numeric_constant", "comment"};
 




void init_string(char* str, int size) {
    for(int k = 0; k < size; k++) {
        str[k] = '\0';
    }
}

void set_substr(char* src, char* dest, int from, int to) {
    if(from < 0 || to >= strlen(src)) {
        return;
    }

    int dest_i = 0;
    for(int k = from; k <= to; k++) {
        dest[dest_i] = src[k];
        dest_i++;
    }

}

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


int is_separator(char* str) {
    return belongs(separators, SEPARATORS_COUNT, str);
}



int is_operator(char* str) {
    return belongs(operators, OPERATORS_COUNT, str);
}


int is_keyword(char* sequence) {
    return belongs(keywords, KEYWORDS_NUMBER, sequence);
}







int fits_token_pattern(char* code, int start, int index, char* lexeme_content) {
    // keyword or identifier
    if(is_alpha(code[start])) {
        return is_alpha(code[index]) || is_digit(code[index]);
    }

    // numeric_constant
    if(is_digit(code[start])) {
        if(index == strlen(code)) {
            return 1;
        }
        return is_digit(code[index]);
    }

    // comment
    if(code[start] == '\'') {
        return (code[index - 1] != '\'' || index == 1);
    }

    // pattern-hostile lexemes (currently separators and operators)
    for(int k = 0; k < 2; k++) {
        int elements_count = (k == 0)? SEPARATORS_COUNT : OPERATORS_COUNT;
        for(int l = 0; l < elements_count; l++) {
            int element_size = strlen(sep_ops[k][l]);
            if(start + element_size - 1 >= strlen(code)) {
                continue;
            }

            char* temp = malloc(element_size*sizeof(char));
            set_substr(code, temp, start, start + element_size - 1);

            if(strcmp(sep_ops[k][l], temp)) {
                free(temp);
                continue;
            }

            strcpy(lexeme_content, sep_ops[k][l]);
            free(temp);    
            return 1;

        }
    }

    return 0;
}



Rule* fits_token_pattern_test(char* code, int start, int index, char* lexeme_content, CFG* pgrammar) {
    // keywords or identifiers
    if(is_alpha(code[start])) {
        return (Rule*) is_alpha(code[index]) || (Rule*) is_digit(code[index]);
    }

    // numeric_constants
    if(is_digit(code[start])) {
        if(index == strlen(code)) {
            return (Rule*) 1;
        }
        return (Rule*) is_digit(code[index]);
    }

    // comments
    if(code[start] == '\'') {
        return ((Rule*)(code[index - 1] != '\'' || index == 1));
    }

    // pattern-hostile lexemes (currently separators and operators)
    for(int k = 0; k < pgrammar->rules_count; k++) {
        Rule* prule = &(pgrammar->rules[k]);

        if(prule->rights_count > 1 || prule->prights[0]->type == NONTERMINAL) {
            continue;
        }

        char* ntcontent = pgrammar->rules[k].prights[0]->content;
        if(start + strlen(ntcontent) - 1 >= strlen(code)) {
            continue;
        }

        
        char* temp = malloc(strlen(ntcontent)*sizeof(char));
        set_substr(code, temp, start, start + strlen(ntcontent) - 1);
        if(strcmp(ntcontent, temp)) {
            free(temp);
            continue;
        }
            
        strcpy(lexeme_content, ntcontent);
        free(temp);    
        return prule;

        
    }

    // Normally, we should never reach here. Otherwise, the character is unrecognized.
    return NULL;
}




Symbol* getp_terminal_matching_lexeme(Lexeme* plexeme, CFG* pgrammar) {
    for(int k = 0; k < pgrammar->symbols_count; k++) {
        if(!(strcmp(plexeme->content, pgrammar->psymbols[k]->content)) || 
            !(strcmp(typeNames[plexeme->type], pgrammar->psymbols[k]->content))) {
            return pgrammar->psymbols[k];
        }
    }

    return NULL;
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

    if(is_separator(lexeme_content)) {
        return separator;
    }

    if(is_operator(lexeme_content)) {
        return grroperator;
    }

}

LexemeType get_read_lex_type_test(char* lexeme_content, Rule* prule) {
    if(prule) {
        //return prule->prights[0]->content; // transform to LexemeType, possibly a function that gets the type from the string
    }
    if(is_alpha(lexeme_content[0])) {
        if(is_keyword(lexeme_content)) {
            return keyword;
        }

        return identifier;
    }

    if(is_digit(lexeme_content[0])) {
        return numeric_constant;
    }

    if(is_separator(lexeme_content)) {
        return separator;
    }

    if(is_operator(lexeme_content)) {
        return grroperator;
    }

}



Lexeme* readLexeme(char* code, int index, CFG* pgrammar) {
    Lexeme* plexeme = malloc(sizeof(Lexeme));

    int p = 0;
    char* lexeme_content = malloc(1000*sizeof(char));
    init_string(lexeme_content, 1000);
    Rule* prule = NULL;
    do {
        lexeme_content[p] = code[index + p];
        p++;
        if(index + p >= strlen(code)) {
            break;
        }
    } while((prule = fits_token_pattern(code, index, index + p, lexeme_content)) && (index + p) < strlen(code));

         
    plexeme->type = get_read_lex_type(lexeme_content);
    strcpy(plexeme->content, lexeme_content);

    return plexeme;
}




int set_from_code(char* code, CFG* pgrammar, void** ppthings, int lexemes) {
    int k = 0, col = 0, things_count = 0;
    int line = 1;

    while(k < strlen(code)) {

        if(code[k] == ' ') {
            k++;
            continue;
        }

        if(code[k] == '\n') {
            line++;
            col = 0;
            k++;
            continue;
        }

        Lexeme* plexeme = readLexeme(code, k, pgrammar);

        if(lexemes) {
            plexeme->location.line = line;
            plexeme->location.col = col;
            ((Lexeme**)ppthings)[things_count] = plexeme;
        } else {
            ((Symbol**)ppthings)[things_count] = get_terminal_matching_lexeme(plexeme, pgrammar);
        }        
        things_count++;

        if(k + strlen(plexeme->content) >= strlen(code)) {
            break;
        }

        k = k + strlen(plexeme->content);
        col = col + strlen(plexeme->content);
    }


    /*printf("Lexemes Total: %d\n===================\n\n", lexeme_count);
    for(int k = 0; k < lexeme_count; k++) {
        printf("%s: %s\n", typeNames[pplexemes[k]->type], pplexemes[k]->content);
    }*/
    
    return things_count;
}






int main(int argc, char* argv[]) {
    char code[] = "int main(int argc) {\n    int k += 0;\nreturn 0;}";

    lex(code);

    return 0;
}
