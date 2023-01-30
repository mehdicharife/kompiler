#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "lexer.h"
#include <string.h>
#include "mystring.h"



 






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


















Symbol* getp_corresponding_symbol(char* lexeme_content, Rule* prule, CFG* pgrammar) {
    if(prule) {
        return prule->prights[0];
    }

    if(is_alpha(lexeme_content[0])) {
        return pgrammar->psymbols[pgrammar->id_pos];
    }

    if(is_digit(lexeme_content[0])) {
        return pgrammar->psymbols[pgrammar->numc_pos];
    }

    for(int k = 0; k < pgrammar->rules_count; k++) {
        if(pgrammar->rules[k].rights_count > 1 || pgrammar->rules[k].prights[0]->type == NONTERMINAL) {
            continue;
        }

        if(!strcmp(lexeme_content, pgrammar->rules[k].prights[0]->content)) {
            return pgrammar->rules[k].prights[0];
        }

    }

}


int fits_token_pattern(char* code, int start, int index, char* lexeme_content, CFG* pgrammar, Rule** ppfit_rule) {
    // keywords or identifiers
    if(is_alpha(code[start])) {
        return  is_alpha(code[index]) ||  is_digit(code[index]);
    }

    // numeric_constants
    if(is_digit(code[start])) {
        if(index == strlen(code)) {
            return  1;
        }
        return  is_digit(code[index]);
    }

    // comments
    if(code[start] == '\'') {
        return ((code[index - 1] != '\'' || index == 1));
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
        *ppfit_rule = prule;

        return 1;
    }

    // Normally, we should never reach here. Otherwise, the character is unrecognized.
    return 0;
}


Lexeme* readLexeme(char* code, int index, CFG* pgrammar) {
    Lexeme* plexeme = malloc(sizeof(Lexeme));
    plexeme->content = malloc(30*sizeof(char));

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
    } while(fits_token_pattern(code, index, index + p, lexeme_content, pgrammar, &prule) && (index + p) < strlen(code));

         
    plexeme->psymbol = getp_corresponding_symbol(lexeme_content, prule, pgrammar);
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
            ((Symbol**)ppthings)[things_count] = plexeme->psymbol;
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





/*
int main(int argc, char* argv[]) {
    char code[] = "int main(int argc) {\n    int k += 0;\nreturn 0;}";

    //lex(code);

    return 0;
}


*/






















/*
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
*/