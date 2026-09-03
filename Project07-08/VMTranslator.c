#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **parser(char buffer[], int *token_count);
void two_op(char operation, FILE *translation);
void rel_op(char jump[], FILE *translation, int *rel_i);

int main(int argc, char *argv[]) 
{
    if (argc != 2) {
        printf("Usage: ./VMTranslator <file_name>\n");
        return 1;
    }

    int len = strlen(argv[1]);
    char translation_name[len + 2]; // Replacing vm (2) with asm (3)
    char file_name[len - 1]; 
    if (len >= 3 && strcmp(argv[1] + len - 3, ".vm") == 0) {
        strcpy(translation_name, argv[1]);
        translation_name[len - 2] = '\0';
        int i;
        for (i = (len - 3); i >= 0; --i) {
            if (translation_name[i] == '/')
                break;
        }
        strcpy(file_name, (translation_name + i + 1));
        strcat(translation_name, "asm");
    } else {
        printf("Enter a file ending with .vm\n");
        return 1;
    }

    FILE *reader = fopen(argv[1], "r");
    if (reader == NULL) {
        printf("Cannot open file\n");
        return 1;
    }

    FILE *translation = fopen(translation_name, "w");
    if (translation == NULL) {
        printf("Failed to write .asm file\n");
        return 1;
    }

    bool stack_initialized = false;
    int rel_i = 0;
    char buffer[100]; // Just an arbitrary length
    char func_name[20] = " ";


    while (fgets(buffer, sizeof(buffer), reader) != NULL) {
        // Setting SP to 256 in assembly
        if (!stack_initialized) {
            fputs("\t// Initializing SP\n", translation);

            fputs("\t@256", translation);
            fputs("\n\tD=A\n", translation);
            fputs("\t@SP\n\tM=D\n", translation);

            stack_initialized = true;
        }

        // Skipping empty buffer
        if (buffer[0] == '\n')
            continue;

        // Parsing this buffer
        int token_count = 0;
        char **tokens = parser(buffer, &token_count);

        // Conditional statements
        if (tokens[0][0] == '/') { 
            for (int i = 0; i < token_count; i++) 
                free(tokens[i]);
            free(tokens);
            continue;
        } else if (strcmp(tokens[0], "push") == 0) {
            fprintf(translation, "\n\t// %s\n", buffer);

            if (strcmp(tokens[1], "constant") == 0) {
                // *SP = tokens[2]
                fputs("\t@", translation);
                fputs(tokens[2], translation);
                fputs("\n\tD=A\n\t@SP\n", translation);
                fputs("\tA=M\n\tM=D\n", translation);

                // SP++
                fputs("\t@SP\n\tM=M+1\n", translation);
            } else if (strcmp(tokens[1], "static") == 0) {
                // *SP = file_name.tokens[2]
                fputs("\t@", translation);
                fputs(file_name, translation);
                fputs(tokens[2], translation);
                fputs("\n\tD=M\n\t@SP\n", translation);
                fputs("\tA=M\n\tM=D\n", translation);

                // SP++
                fputs("\t@SP\n\tM=M+1\n", translation);
            } else if (strcmp(tokens[1], "temp") == 0) {
                // addr = 5 + tokens[2]
                fputs("\t@5\n\tD=A\n\t@", translation);
                fputs(tokens[2], translation);
                fputs("\n\tD=D+A\n", translation);

                // *SP = *addr
                fputs("\tA=D\n\tD=M\n", translation);
                fputs("\t@SP\n\tA=M\n", translation);
                fputs("\tM=D\n", translation);

                // SP++
                fputs("\t@SP\n\tM=M+1\n", translation);
            } else if (strcmp(tokens[1], "pointer") == 0) {
                char this_or_that[5];
                if (strcmp(tokens[2], "0") == 0)
                    strcpy(this_or_that, "THIS");
                else if (strcmp(tokens[2], "1") == 0)
                    strcpy(this_or_that, "THAT");

                // *SP = THIS/THAT
                fputs("\t@", translation);
                fputs(this_or_that, translation);
                fputs("\n\tD=M\n\t@SP\n", translation);
                fputs("\tA=M\n\tM=D\n", translation);

                // SP++
                fputs("\t@SP\n\tM=M+1\n", translation);
            } else {
                // local, argument, this, that

                char mem_name[5];
                if (strcmp(tokens[1], "local") == 0) 
                    strcpy(mem_name, "LCL");
                else if (strcmp(tokens[1], "argument") == 0) 
                    strcpy(mem_name, "ARG");
                else if (strcmp(tokens[1], "this") == 0) 
                    strcpy(mem_name, "THIS");
                else if (strcmp(tokens[1], "that") == 0) 
                    strcpy(mem_name, "THAT");

                // addr = mem_name + tokens[2]
                fputs("\t@", translation);
                fputs(mem_name, translation);
                fputs("\n\tD=M\n\t@", translation);
                fputs(tokens[2], translation);
                fputs("\n\tD=D+A\n", translation);

                // *SP = *addr
                fputs("\tA=D\n\tD=M\n\t@SP\n", translation);
                fputs("\tA=M\n\tM=D\n", translation);

                // SP++
                fputs("\t@SP\n\tM=M+1\n", translation);
            }
        } else if (strcmp(tokens[0], "pop") == 0) {
            fprintf(translation, "\n\t// %s\n", buffer);

            if (strcmp(tokens[1], "static") == 0) {
                // SP--
                fputs("\t@SP\n\tM=M-1\n", translation);

                // file_name.tokens[2] = *SP
                fputs("\tA=M\n\tD=M\n\t@", translation);
                fputs(file_name, translation);
                fputs(tokens[2], translation);
                fputs("\n\tM=D\n", translation);
            } else if (strcmp(tokens[1], "temp") == 0) {
                // addr = 5 + i
                fputs("\t@5\n\tD=A\n\t@", translation);
                fputs(tokens[2], translation);
                fputs("\n\tD=D+A\n\t@addr\n\tM=D\n", translation);

                // SP--
                fputs("\t@SP\n\tM=M-1\n", translation);

                // *addr = *SP
                fputs("\tA=M\n\tD=M\n", translation);
                fputs("\t@addr\n\tA=M\n\tM=D\n", translation);
            } else if (strcmp(tokens[1], "pointer") == 0) {
                char this_or_that[5];
                if (strcmp(tokens[2], "0") == 0)
                    strcpy(this_or_that, "THIS");
                else if (strcmp(tokens[2], "1") == 0)
                    strcpy(this_or_that, "THAT");
                    
                // SP--
                fputs("\t@SP\n\tM=M-1\n", translation);

                // THIS/THAT = *SP
                fputs("\tA=M\n\tD=M\n\t@", translation);
                fputs(this_or_that, translation);
                fputs("\n\tM=D\n", translation);
            } else {
                // local, argument, this, that

                char mem_name[5];
                if (strcmp(tokens[1], "local") == 0) 
                    strcpy(mem_name, "LCL");
                else if (strcmp(tokens[1], "argument") == 0) 
                    strcpy(mem_name, "ARG");
                else if (strcmp(tokens[1], "this") == 0) 
                    strcpy(mem_name, "THIS");
                else if (strcmp(tokens[1], "that") == 0) 
                    strcpy(mem_name, "THAT");

                // addr = mem_name + i
                fprintf(translation, "\t@%s\n\tD=M\n", mem_name);
                fprintf(translation, "\t@%s\n\tD=D+A\n", tokens[2]);
                fputs("\t@addr\n\tM=D\n", translation);

                // SP--
                fputs("\t@SP\n\tM=M-1\n", translation);

                // *addr = *SP
                fputs("\tA=M\n\tD=M\n", translation);
                fputs("\t@addr\n\tA=M\n\tM=D\n", translation);
            }
        } else if (strcmp(tokens[0], "add") == 0) {
            fprintf(translation, "\n\t// %s\n", buffer);

            two_op('+', translation);
        } else if (strcmp(tokens[0], "sub") == 0) {
            fprintf(translation, "\n\t// %s\n", buffer);

            two_op('-', translation);
        } else if (strcmp(tokens[0], "and") == 0) {
            fprintf(translation, "\n\t// %s\n", buffer);

            two_op('&', translation);
        } else if (strcmp(tokens[0], "or") == 0) {
            fprintf(translation, "\n\t// %s\n", buffer);

            two_op('|', translation);
        } else if (strcmp(tokens[0], "eq") == 0) {
            fprintf(translation, "\n\t// %s\n", buffer);

            rel_op("JEQ", translation, &rel_i);
        } else if (strcmp(tokens[0], "gt") == 0) {
            fprintf(translation, "\n\t// %s\n", buffer);

            rel_op("JGT", translation, &rel_i);
        } else if (strcmp(tokens[0], "lt") == 0) {
            fprintf(translation, "\n\t// %s\n", buffer);

            rel_op("JLT", translation, &rel_i);
        } else if (strcmp(tokens[0], "neg") == 0) {
            fprintf(translation, "\n\t// %s\n", buffer);

            fputs("\t@SP\n\tA=M-1\n", translation);
            fputs("\tM=-M\n", translation);
        } else if (strcmp(tokens[0], "not") == 0) {
            fprintf(translation, "\n\t// %s\n", buffer);

            fputs("\t@SP\n\tA=M-1\n", translation);
            fputs("\tM=!M\n", translation);
        } else if (strcmp(tokens[0], "label") == 0) {
            fprintf(translation, "\n\t// %s\n", buffer);

            if (strcmp(func_name, " ") == 0)
                fprintf(translation, "(%s)\n", tokens[1]);
            else
                fprintf(translation, "(%s$%s)\n", func_name, tokens[1]);
        } else if (strcmp(tokens[0], "goto") == 0) {
            fprintf(translation, "\n\t// %s\n", buffer);

            if (strcmp(func_name, " ") == 0)
                fprintf(translation, "\t%s\n", tokens[1]);
            else
                fprintf(translation, "\t@%s$%s\n", func_name, tokens[1]);
            fprintf(translation, "\t0;JMP\n");
        } else if (strcmp(tokens[0], "if-goto") == 0) {
            fprintf(translation, "\n\t// %s\n", buffer);

            fprintf(translation, "\t@SP\n\tM=M-1\n");
            fprintf(translation, "\tA=M\n\tD=M\n");
            if (strcmp(func_name, " ") == 0)
                fprintf(translation, "\t@%s\n", tokens[1]);
            else
                fprintf(translation, "\t@%s$%s\n", func_name, tokens[1]);
            fprintf(translation, "\tD;JNE\n");
        } else if (strcmp(tokens[0], "function") == 0) {
            fprintf(translation, "\n\t// %s\n", buffer);

            strcpy(func_name, tokens[1]);
            fprintf(translation, "(%s)\n", func_name);
            int local_var_no = atoi(tokens[2]);
            for (int i = 0; i < local_var_no; i++) {
                // push 0
                fprintf(translation, "\t@0\n\tD=A\n");
                fprintf(translation, "\t@SP\n\tA=M\n\tM=D\n");
                fprintf(translation, "\t@SP\n\tM=M+1\n");
            }
        } else if (strcmp(tokens[0], "call") == 0) {
            // TODO

        } else if (strcmp(tokens[0], "return") == 0) {
            fprintf(translation, "\n\t// %s\n", buffer);

            // frame = LCL
            fprintf(translation, "\t@LCL\n\tD=M\n");
            fprintf(translation, "\t@frame\n\tM=D\n");

            // retAddr = *(frame - 5)
            fprintf(translation, "\t@frame\n\tD=M\n");
            fprintf(translation, "\t@5\n\tD=D-A\n");
            fprintf(translation, "\tA=D\n\tD=M\n");
            fprintf(translation, "\t@retAddr\n\tM=D\n");

            // *ARG = pop()
            fprintf(translation, "\t@SP\n\tA=M-1\n");
            fprintf(translation, "\tD=M\n\t@ARG\n");
            fprintf(translation, "\tA=M\n\tM=D\n");

            // SP = ARG + 1
            fprintf(translation, "\t@ARG\n\tD=M+1\n");
            fprintf(translation, "\t@SP\n\tM=D\n");

            // THAT = *(frame - 1)
            fprintf(translation, "\t@frame\n\tD=M-1\n");
            fprintf(translation, "\tA=D\n\tD=M\n");
            fprintf(translation, "\t@THAT\n\tM=D\n");

            // THIS = *(frame - 2)
            fprintf(translation, "\t@frame\n\tD=M\n");
            fprintf(translation, "\t@2\n\tD=D-A\n");
            fprintf(translation, "\tA=D\n\tD=M\n");
            fprintf(translation, "\t@THIS\n\tM=D\n");

            // ARG = *(frame - 3)
            fprintf(translation, "\t@frame\n\tD=M\n");
            fprintf(translation, "\t@3\n\tD=D-A\n");
            fprintf(translation, "\tA=D\n\tD=M\n");
            fprintf(translation, "\t@ARG\n\tM=D\n");

            // LCL = *(frame - 4)
            fprintf(translation, "\t@frame\n\tD=M\n");
            fprintf(translation, "\t@4\n\tD=D-A\n");
            fprintf(translation, "\tA=D\n\tD=M\n");
            fprintf(translation, "\t@LCL\n\tM=D\n");

            // goto retAddr
            fprintf(translation, "\t@retAddr\n\tA=M\n\t0;JMP\n");
        }

        // Freeing up tokens
        for (int i = 0; i < token_count; i++)
            free(tokens[i]);
        free(tokens);
    }

    fclose(reader);
    fclose(translation);

    return 0;
}

char **parser(char buffer[], int *token_count)
{
    int max_tokens = 3;

    int start = 0;
    while (buffer[start] == ' ' || buffer[start] == '\t')
        start++;
    int front = start, rear = start;

    // Takingh each string as an array of 21 chars
    char **tokens = malloc(max_tokens * sizeof(char *));
    for (int i = 0; i < max_tokens; i++) {

        // This condition signifies end of line
        if (buffer[rear] == '\n' || buffer[rear] == '\0')
            return tokens;

        // Every step, front moves to the prior rear
        front = rear;

        // And then rear moves away
        while (buffer[rear] != ' ' &&
               buffer[rear] != '\n' &&
               buffer[rear] != '\0') 
            rear++;
        
        int length = rear - front;
        // malloc() was giving wrong output as
        // it does not set the allocated memory to '\0'
        tokens[i] = calloc(21, sizeof(char));
        (*token_count)++;
        memcpy(tokens[i], (buffer + front), length);
        tokens[i][length] = '\0';

        // front should not end up in whitespace the next
        // time but also shouldn't skip over '\n' or '\0'
        if (buffer[rear] == ' ')
            rear++;
    }
    
    return tokens;
}

void two_op(char operation, FILE *translation)
{
    fputs("\t@SP\n\tA=M-1\n\tA=A-1\n", translation);
    fputs("\tD=M\n\tA=A+1\n\tD=D", translation);
    fputc(operation, translation);
    fputs("M\n\tA=A-1\n\tM=D\n", translation);
    fputs("\t@SP\n\tM=M-1\n", translation);

    return;
}

void rel_op(char jump[], FILE *translation, int *rel_i)
{
    fputs("\t@SP\n\tA=M-1\n", translation);
    fputs("\tA=A-1\n", translation);
    fputs("\tD=M\n\tA=A+1\n", translation);
    fputs("\tD=D-M\n\tA=A-1\n\tM=-1\n", translation);
    fputs("\t@REL", translation);
    fprintf(translation, "%i\n", rel_i);
    fputs("\tD;", translation);
    fputs(jump, translation);
    fputs("\n\t@SP\n\tA=M-1\n", translation);
    fputs("\tA=A-1\n\tM=0\n(REL", translation);
    fprintf(translation, "%i)\n", rel_i);
    fputs("\t@SP\n\tM=M-1\n", translation);

    (*rel_i)++;

    return;
}