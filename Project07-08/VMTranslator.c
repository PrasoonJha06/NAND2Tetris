#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[30];
    int return_no;
} func;
int func_no = 1;

bool stack_initialized = false;
bool is_dir = false;
bool sys_called = false;
int rel_i = 0;
char func_name[40] = " ";
bool call_failed = false;

void translate(char file[], FILE *translation, func *functions);
char **parser(char buffer[], int *token_count);
void two_op(char operation, FILE *translation);
void rel_op(char jump[], FILE *translation);
void call(char function[], int arg_no, FILE *translation, func *functions);

int main(int argc, char *argv[])
{
    func *functions = malloc(sizeof(functions));

    if (argc != 2) {
        printf("Usage: ./VMTranslator <source>\n");
        return 1;
    }

    // Reading the file(s) and writing into translation file
    int len = strlen(argv[1]);

    if (len >= 3 && strcmp(argv[1] + len - 3, ".vm") == 0) {

        char translation_name[len + 2]; // asm is 1 character longer
                                        // and 1 more for '\0'
        strcpy(translation_name, argv[1]);
        translation_name[len - 3] = '\0';
        strcat(translation_name, ".asm");

        FILE *translation = fopen(translation_name, "w");
        if (translation == NULL) {
            printf("Failed to write .asm file\n");
            return 1;
        }

        translate(argv[1], translation, functions);

        fclose(translation);
        
    } else {                               // It is a folder
        is_dir = true;

        DIR *folder; 
        struct dirent *entry;

        folder = opendir(argv[1]);
        if (folder == NULL) {
            printf("Couldn't open folder\n");
            return 1;
        }

        // Naming the translation
        int dir_len = strlen(argv[1]);
        int j = 0;
        while (argv[1][dir_len - (j + 1)] != '/')
            j++;
        char last_dir[j + 2]; // +2 for '/' and '\0'
        strcpy(last_dir, "/");
        strcat(last_dir, (argv[1] + dir_len - j));
        char translation_name[dir_len + j + 2 + 3]; 
        strcpy(translation_name, argv[1]);
        strcat(translation_name, last_dir);
        strcat(translation_name, ".asm");

        FILE *translation = fopen(translation_name, "w");
        if (translation == NULL) {
            printf("Failed to write .asm file\n");
            return 1;
        }

        char reader_addr[dir_len + 30 + 1];
        strcpy(reader_addr, argv[1]);
        strcat(reader_addr, "/");

        while ((entry = readdir(folder)) != NULL) {
            char file[30];
            strcpy(file, entry->d_name); // d_name contains file's name
            len = strlen(file);
            if (len >= 3 && strcmp(file+len-3, ".vm") == 0) {
                strcat(reader_addr, file);
                printf("%s\n", reader_addr);

                translate(reader_addr, translation, functions);
                strcpy(reader_addr, argv[1]);
                strcat(reader_addr, "/");
            }
        }

        fclose(translation);
        closedir(folder);
    }

    free(functions);
}

void translate(char file[], FILE *translation, func *functions)
{
    FILE *reader = fopen(file, "r");
    if (reader == NULL) {
        printf("Cannot open %s\n", file);
        return;
    }

    // foo is file's name without its directory and extension
    char foo[strlen(file) + 1];
    char *ptr = strstr(file, ".vm");
    *ptr = '\0';
    while (*ptr != '/') {
        if (file - ptr == 0)
            break;

        ptr--;
    }
    strcpy(foo, (ptr + 1));

    char buffer[100]; // Just an arbitrary length

    while (fgets(buffer, sizeof(buffer), reader) != NULL) {
        // Setting SP to 256 in assembly
        if (!stack_initialized) {
            fputs("\t// Initializing SP\n", translation);

            fputs("\t@256", translation);
            fputs("\n\tD=A\n", translation);
            fputs("\t@SP\n\tM=D\n", translation);

            stack_initialized = true;
        }

        // Calling Sys.init()
        if (is_dir && !sys_called) {
            call("Sys.init", 0, translation, functions);
            if (call_failed) {
                fclose(reader);
                return;
            }
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
                // *SP = foo.tokens[2]
                fputs("\t@", translation);
                fputs(foo, translation);
                fputc('.', translation);
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

                // foo.tokens[2] = *SP
                fputs("\tA=M\n\tD=M\n\t@", translation);
                fputs(foo, translation);
                fputc('.', translation);
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

            rel_op("JEQ", translation);

        } else if (strcmp(tokens[0], "gt") == 0) {
            fprintf(translation, "\n\t// %s\n", buffer);

            rel_op("JGT", translation);

        } else if (strcmp(tokens[0], "lt") == 0) {
            fprintf(translation, "\n\t// %s\n", buffer);

            rel_op("JLT", translation);

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

            fprintf(translation, "(%s)\n", func_name);
            int local_var_no = atoi(tokens[2]);
            for (int i = 0; i < local_var_no; i++) {
                // push 0
                fprintf(translation, "\t@SP\n\tA=M\n\tM=0\n");
                fprintf(translation, "\t@SP\n\tM=M+1\n");
            }

        } else if (strcmp(tokens[0], "call") == 0) {
            int arg_no = atoi(tokens[2]);
            call(tokens[1], arg_no, translation, functions);
            if (call_failed) {
                fclose(reader);
                return;
            }

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
}

void rel_op(char jump[], FILE *translation)
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

    rel_i++;
}

void call(char function[], int arg_no, FILE *translation, func *functions)
{
    strcpy(func_name, function);
    bool is_stored_already = false;
    int i;
    for (i = 0; i < func_no; i++) {
        if (strcmp(func_name, functions[i].name) == 0) {
            is_stored_already = true;
            functions[i].return_no++;
            break;
        }
    }

    if (!is_stored_already) {
        // Add func_name to functions[i].name
        strcpy(functions[i].name, func_name);
        // Set functions[i].return_no = 0
        functions[i].return_no = 0;
        // Expand functions array
        func *tmp = realloc(function, (func_no * sizeof(func)));
        if (tmp == NULL) {
            printf("Couldn't call %s\n", func_name);
            free(functions);
            call_failed = true;
            return;
        }
        functions = tmp;
        func_no++;
    }

    // push func_name$ret.functions[i].return_no
    fprintf(translation, "\t@%s$ret.%d\n", func_name, functions[i].return_no);
    fprintf(translation, "\tD=A\n\t@SP\n");
    fprintf(translation, "\tA=M\n\tM=D\n");
    fprintf(translation, "\t@SP\n\tM=M+1\n");

    // push LCL
    fprintf(translation, "\t@LCL\n\tD=M\n\t@SP\n");
    fprintf(translation, "\tA=M\n\tM=D\n");
    fprintf(translation, "\t@SP\n\tM=M+1\n");

    // push THIS
    fprintf(translation, "\t@THIS\n\tD=M\n\t@SP\n");
    fprintf(translation, "\tA=M\n\tM=D\n");
    fprintf(translation, "\t@SP\n\tM=M+1\n");

    // push THAT
    fprintf(translation, "\t@THAT\n\tD=M\n\t@SP\n");
    fprintf(translation, "\tA=M\n\tM=D\n");
    fprintf(translation, "\t@SP\n\tM=M+1\n");

    // ARG = SP - 5 - arg_no
    fprintf(translation, "\t@SP\n\tD=M\n");
    fprintf(translation, "\t@5\n\tD=D-A\n");
    fprintf(translation, "\t@%s\n\tD=D-A\n", arg_no);
    fprintf(translation, "\t@ARG\n\tM=D\n");

    // LCL = SP
    fprintf(translation, "\t@SP\n\tD=M\n");
    fprintf(translation, "\t@LCL\n\tM=D\n");

    // goto func_name
    fprintf(translation, "\t@%s\n\tA=M\n", func_name);
    fprintf(translation, "0;JMP");

    // (func_name$ret.functions[i].return_no)
    fprintf(translation, "(%s$ret.%d)\n", func_name, functions[i].return_no);
}