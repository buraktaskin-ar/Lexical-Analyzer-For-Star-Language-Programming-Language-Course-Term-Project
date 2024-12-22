#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_IDENTIFIER_SIZE 10
#define MAX_INTEGER_SIZE 8
#define MAX_STRING_SIZE 256


enum States {
    ID,
    OPERATOR,
    COMMENT,
    STRING,
    PARANTEZ,
    ENDOFLINE
};

enum States active_state = ID;

int isIdentifier(char str[], FILE *outputFile) {

    int length = strlen(str);

    // Tüm karakterler sayı değilse integer değil diye belirtiyoruz.
    bool isInteger = true;
    for (int i = 0; i < length; i++) {
        if (isdigit(str[i]) == false) {
            isInteger = false;
            break;
        }
    }

    if (isInteger == true) {
        if (length > MAX_INTEGER_SIZE) {
            printf("Error: Integer must be shorter than 11 chars.\n");
            fclose(outputFile);
            exit(5);
            return 0;
        }

        fprintf(outputFile, "IntConst(%s)\n", str);
        return 0;
    }

    if (!isalpha(str[0])) {
        printf("Error: Identifier must start with a letter.\n");
        fclose(outputFile);
        exit(5);
        return 0;
    }

    if (length > MAX_IDENTIFIER_SIZE) {
        printf("Error: Identifier must be shorter than 10 chars.\n");
        fclose(outputFile);
        exit(5);
        return 0;
    }

    // Güncellenen keyword listesi
    char keywords[8][10] = {"int", "text", "is", "loop", "times", "read", "write", "newLine"};

    for (int i = 0; i < 8; ++i) {
        if (strcmp(keywords[i], str) == 0) {
            fprintf(outputFile, "Keyword(%s)\n", str);
            return 0;
        }
    }

    fprintf(outputFile, "Identifier(%s)\n", str);
    return 0;
}



int isOperator(char str[], FILE *outputFile) {
    // Operatörlerin listesi güncellendi
    char operators[5][2] = {"+", "-", "*", "/"};
    for (int i = 0; i < 4; ++i) {
        if (strcmp(operators[i], str) == 0) {
            fprintf(outputFile, "Operator(%s)\n", str);
            return 0;
        }
    }
    printf("Error: Invalid Operator(%s)\n", str);
    fclose(outputFile);
    exit(5);
    return 0;
}


int isParantez(char str[], FILE *outputFile) {
    char parantezler[2] = {'{', '}'};
    char parantez_adlari[2][19] = {"LeftCurlyBracket", "RightCurlyBracket"};

    for (int i = 0; i < strlen(str); i++) {
        for (int j = 0; j < 2; j++) {
            if (parantezler[j] == str[i]) {
                fprintf(outputFile, "%s\n", parantez_adlari[j]);
                break; // İlgili parantezi bulduktan sonra döngüyü kır
            }
        }
    }

    return 0;
}



int isEndOfLine(char str[], FILE *outputFile) {
    // Herhangi bir karakter olmadığında EndOfLine belirteci yazdır
    fprintf(outputFile, "EndOfLine\n");
    return 0;
}




void state_change(char token[], char new_char, enum States new_state, int (*f_name)(char str[], FILE *_outputFile),
                  FILE *outputFile) {
    //printf("Token: %s\n", token);
    if (strcmp(token, "") != 0)//farklıysa, kesinlikle != 0 kullan çünkü 0dan farklı herhangi bir değer olabilir.
    {
        f_name(token, outputFile);
    }
    active_state = new_state;
    strcpy(token, "");
    strncat(token, &new_char, 1);
}

void handle(char line[], char token[], FILE *outputFile) {
    int length = strlen(line);

    for (int i = 0; i < length; i++) {
        char new_char = line[i];

        if (active_state == ID) {
            if (new_char == ' ' || new_char == '\n' || new_char == '\0') {
                state_change(token, '\0', ID, isIdentifier, outputFile);
                continue;
            } else if (isalnum(new_char) || new_char == '_') {
                strncat(token, &new_char, 1);
                continue;
            } else if (new_char == '+' || new_char == '-' || new_char == '*' || new_char == '/' || new_char == ':' ||
                       new_char == '=') {
                state_change(token, new_char, OPERATOR, isIdentifier, outputFile);
                continue;
            } else if (new_char == '{' || new_char == '}') {
                state_change(token, new_char, PARANTEZ, isIdentifier, outputFile);
                continue;
            } else if (new_char == '"') {
                state_change(token, new_char, STRING, isIdentifier, outputFile);
                continue;
            } else if (new_char == '.') { // Yeni karakter: "."
                state_change(token, new_char, ENDOFLINE, isIdentifier, outputFile);
                continue;
            } else {
                printf("Invalid Character(%c)\n", new_char);
                fclose(outputFile);
                exit(5);
                continue;
            }
        }

        else if (active_state == OPERATOR) {
            if (new_char == ' ' || new_char == '\n' || new_char == '\0') {
                state_change(token, '\0', ID, isOperator, outputFile);
                continue;
            } else if (isalnum(new_char) || new_char == '_') {
                state_change(token, new_char, ID, isOperator, outputFile);
                continue;
            } else if (new_char == '+' || new_char == '-' || new_char == '*' || new_char == '/' || new_char == ':' ||
                       new_char == '=') {
                strncat(token, &new_char, 1);

                int token_length = strlen(token);

                if (token_length == 2 && strcmp(token, "/*") == 0) {
                    active_state = COMMENT;
                    strcpy(token, "");
                } else if (token_length > 2 && token[token_length - 2] == '/' && token[token_length - 1] == '*') {
                    //For döngüsü ile ekledik çünkü stringin sonunda strncopy bozuk çalışıyor.
                    char sub_token[] = "";
                    for (int a = 0; a < token_length - 2; a++) {
                        strncat(sub_token, &token[a], 1);
                    }

                    isOperator(sub_token, outputFile);

                    active_state = COMMENT;
                    strcpy(token, "");
                }
                continue;
            }
            else if (new_char == '{' || new_char == '}') {
                state_change(token, new_char, PARANTEZ, isOperator, outputFile);
                continue;
            } else if (new_char == '"') {
                state_change(token, new_char, STRING, isOperator, outputFile);
                continue;
            } else if (new_char == '.') {
                state_change(token, new_char, ENDOFLINE, isOperator, outputFile);
                continue;
            } else {
                //state_change(token, '\0', ID, isIdentifier, outputFile);
                printf("Invalid Character(%c)\n", new_char);
                fclose(outputFile);
                exit(5);
                continue;
            }
        }
        else if (active_state == COMMENT) {
            strncat(token, &new_char, 1);

            int token_length = strlen(token);

            if (token_length > 1 && token[token_length - 2] == '*' && token[token_length - 1] == '/') {
                active_state = ID;
                strcpy(token, "");
                continue;
            }
        }
        else if (active_state == STRING) {
            strncat(token, &new_char, 1);

            int token_length = strlen(token);

            if (token_length > 0 && token[token_length - 1] == '"') {
                fprintf(outputFile, "String(%s)\n", token);
                active_state = ID;
                strcpy(token, "");
                continue;
            }
        }
        else if (active_state == PARANTEZ) {
            if (new_char == ' ' || new_char == '\n' || new_char == '\0') {
                state_change(token, '\0', ID, isParantez, outputFile);
                continue;
            } else if (isalnum(new_char) || new_char == '_') {
                state_change(token, new_char, ID, isParantez, outputFile);
                continue;
            } else if (new_char == '+' || new_char == '-' || new_char == '*' || new_char == '/' || new_char == ':' ||
                       new_char == '=') {
                state_change(token, new_char, OPERATOR, isParantez, outputFile);
                continue;
            } else if (new_char == '{' || new_char == '}') {
                strncat(token, &new_char, 1);
                continue;
            } else if (new_char == '"') {
                state_change(token, new_char, STRING, isParantez, outputFile);
                continue;
            } else if (new_char == '.') {
                state_change(token, new_char, ENDOFLINE, isParantez, outputFile);
                continue;
            } else {
                //state_change(token, '\0', ID, isIdentifier, outputFile);
                printf("Invalid Character(%c)\n", new_char);
                fclose(outputFile);
                exit(5);
                continue;
            }
        }
        else if (active_state == ENDOFLINE) {
            if (new_char == ' ' || new_char == '\n' || new_char == '\0') {
                state_change(token, '\0', ID, isEndOfLine, outputFile);
                continue;
            } else if (isalnum(new_char) || new_char == '_') {
                state_change(token, new_char, ID, isEndOfLine, outputFile);
                continue;
            } else if (new_char == '+' || new_char == '-' || new_char == '*' || new_char == '/' || new_char == ':' ||
                       new_char == '=') {
                state_change(token, new_char, OPERATOR, isEndOfLine, outputFile);
                continue;
            } else if (new_char == '{' || new_char == '}') {
                state_change(token, new_char, PARANTEZ, isEndOfLine, outputFile);
                continue;
            } else if (new_char == '"') {
                state_change(token, new_char, STRING, isEndOfLine, outputFile);
                continue;
            } else if (new_char == '.') {
                state_change(token, new_char, ENDOFLINE, isEndOfLine, outputFile);
                continue;
            } else {
                //state_change(token, '\0', ID, isIdentifier, outputFile);
                printf("Invalid Character(%c)\n", new_char);
                fclose(outputFile);
                exit(5);
                continue;
            }
        }
    }
}

int main() {

    printf("\n");
    char token[1024] = "";

    FILE *inputFile;
    char line[1024];

    // Open the input file
    inputFile = fopen("code.sta", "r");
    if (inputFile == NULL) {
        printf("Error opening the file!\n");
        return 1;
    }

    FILE *outputFile;
    // Open the output file
    outputFile = fopen("code.lex", "w");

    if (outputFile == NULL) {
        printf("Error opening the file!\n");
        return 1;
    }

    while (fgets(line, sizeof(line), inputFile)) {
        //printf("\nNew Line: %s", line);
        handle(line, token, outputFile);
    }


    if (feof(inputFile)) {
        if (active_state == COMMENT || active_state == STRING) {
            printf("Lexical Error, EOF during Comment or String\n");
        }
        printf("END OF FILE\n");
    } else if (ferror(inputFile)) {
        printf("ERROR\n");
    }

    fclose(inputFile);
    fclose(outputFile);
    return 0;
}