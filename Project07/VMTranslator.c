#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_LENGTH 100

int *find_indices(char buffer[]);
void two_op(char operation, FILE *translation);

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Usage: ./VMTranslator <filename>\n");
        return 1;
    }

    int len = strlen(argv[1]);
    char translation_name[(len - 2) + 3]; // Replacing vm (2) with asm (3)
    char foo[len - 1]; 
    if (len >= 3 && strcmp(argv[1] + len - 3, ".vm") == 0) {
        strcpy(translation_name, argv[1]);
        translation_name[len - 2] = '\0';
        strcpy(foo, translation_name); // Will be used in static memory segment
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
    char buffer[BUFFER_LENGTH];
    while (fgets(buffer, sizeof(buffer), reader) != NULL) {
        // Setting SP to 256 in assembly
        if (!stack_initialized) {
            fputs("\t// Initializing SP\n", translation);

            fputs("\t@256", translation);
            fputs("\n\tD=A\n", translation);
            fputs("\t@SP\n\tM=D\n", translation);

            stack_initialized = true;
        }

        // This assumes no indentation in VM Code

        // Empty line
        if (buffer[0] == '\n')
            continue;
        // Comment
        else if (buffer[0] == '/')
            continue;
        // VM Code
        else {
            int i = 0;
            while(true) {
                if (buffer[i] == '\n') {
                    break;
                } else if (buffer[i] == '\0') {
                    buffer[i] = '\n';
                    break;
                }
                ++i;
            }
            fputs("\t// ", translation);
            fputs(buffer, translation);

            // Stack Operations
            if (strstr(buffer, "push") != NULL) {
                int *indices = find_indices(buffer);
                char mem_seg[9];
                char seg_index[6];

                strncpy(mem_seg, buffer + indices[1], 9);
                strncpy(seg_index, buffer + indices[2], 6);

                if (strcmp(mem_seg, "constant") == 0) {
                    // *SP = seg_index
                    fputs("\t@", translation);
                    fputs(seg_index, translation);
                    fputs("\n\tD=A\n\t@SP\n", translation);
                    fputs("\tA=M\n\tM=D\n", translation);

                    // SP++
                    fputs("\t@SP\n\tM=M+1\n", translation);
                } else if (strcmp(mem_seg, "static") == 0) {
                    // *SP = foo.seg_index
                    fputs("\t@", translation);
                    fputs(foo, translation);
                    fputs(seg_index, translation);
                    fputs("\n\tD=M\n\t@SP\n", translation);
                    fputs("\tA=M\n\tM=D\n", translation);

                    // SP++
                    fputs("\t@SP\n\tM=M+1\n", translation);
                } else if (strcmp(mem_seg, "temp") == 0) {
                    // addr = 5 + seg_index
                    fputs("\t@5\n\tD=A\n\t@", translation);
                    fputs(seg_index, translation);
                    fputs("\n\tD=D+A\n", translation);

                    // *SP = *addr
                    fputs("\tA=D\n\tD=M\n", translation);
                    fputs("\t@SP\n\tA=M\n", translation);
                    fputs("\tM=D\n", translation);

                    // SP++
                    fputs("\t@SP\t\nM=M+1\n", translation);
                } else if (strcmp(mem_seg, "pointer") == 0) {
                    char this_or_that[5];
                    if (strcmp(seg_index, "0") == 0)
                        strcpy(this_or_that, "THIS");
                    else if (strcmp(seg_index, "1") == 0)
                        strcpy(this_or_that, "THAT");

                    // *SP = THIS/THAT
                    fputs("\t@", translation);
                    fputs(this_or_that, translation);
                    fputs("\n\tD=M\n\t@SP\n", translation);
                    fputs("\tA=M\n\tM=D\n", translation);

                    // SP++
                    fputs("\t@SP\t\nM=M+1\n", translation);
                } else {
                    // local, argument, this, that

                    char mem_name[5];
                    if (strcmp(mem_seg, "local") == 0) 
                        strcpy(mem_name, "LCL");
                    else if (strcmp(mem_seg, "argument") == 0) 
                        strcpy(mem_name, "ARG");
                    else if (strcmp(mem_seg, "this") == 0) 
                        strcpy(mem_name, "THIS");
                    else if (strcmp(mem_seg, "that") == 0) 
                        strcpy(mem_name, "THAT");

                    // addr = mem_name + seg_index
                    fputs("\t@", translation);
                    fputs(mem_name, translation);
                    fputs("\n\tD=M\n\t@", translation);
                    fputs(seg_index, translation);
                    fputs("\n\tD=D+A\n", translation);

                    // *SP = *addr
                    fputs("\tA=D\n\tD=M\n\t@SP\n", translation);
                    fputs("\tA=M\n\tM=D\n", translation);

                    // SP++
                    fputs("\t@SP\n\tM=M+1\n", translation);
                }
            } else if (strstr(buffer, "pop") != NULL) {
                int *indices = find_indices(buffer);
                char mem_seg[9];
                char seg_index[6];

                strncpy(mem_seg, buffer + indices[1], 9);
                strncpy(seg_index, buffer + indices[2], 6);

                if (strcmp(mem_seg, "static") == 0) {
                    // TODO
                } else if (strcmp(mem_seg, "temp") == 0) {
                    // TODO
                } else if (strcmp(mem_seg, "pointer") == 0) {
                    // TODO
                } else {
                    // local, argument, this, that

                    char mem_name[5];
                    if (strcmp(mem_seg, "local") == 0) 
                        strcpy(mem_name, "LCL");
                    else if (strcmp(mem_seg, "argument") == 0) 
                        strcpy(mem_name, "ARG");
                    else if (strcmp(mem_seg, "this") == 0) 
                        strcpy(mem_name, "THIS");
                    else if (strcmp(mem_seg, "that") == 0) 
                        strcpy(mem_name, "THAT");

                    // TODO
                }
            }
            // Two Operand Operations
            else if (strstr(buffer, "add") != NULL) {
                two_op('+', translation);
            } else if (strstr(buffer, "sub") != NULL) {
                two_op('-', translation);
            } else if (strstr(buffer, "and") != NULL) {
                two_op('&', translation);
            } else if (strstr(buffer, "or") != NULL) {
                two_op('|', translation);
            } else if (strstr(buffer, "eq") != NULL) {
                // TODO
            } else if (strstr(buffer, "gt") != NULL) {
                // TODO
            } else if (strstr(buffer, "lt") != NULL) {
                // TODO
            }
            // One Operand Operations
            else if (strstr(buffer, "neg") != NULL) {
                fputs("\t@SP\n\tA=M-1\n", translation);
                fputs("\tM=-M\n", translation);
            } else if (strstr(buffer, "not") != NULL) {
                fputs("\t@SP\n\tA=M-1\n", translation);
                fputs("\tM=!M\n", translation);
            }
        }
    }

    fclose(reader);
    fclose(translation);

    return 0;
}

int *find_indices(char buffer[])
{
    // Stores indices of all three words
    static int indices[3] = {0}; // First address is already 0
    int i = 0, arr_i = 1; // i is for buffer, arr_i is for indices[]
    while ((buffer[i] != '\n') && (buffer[i] != '\0')) {
        if (buffer[i] == ' ') {
            buffer[i] = '\0';
            indices[arr_i] = i + 1; // index after space is where next word starts
            ++arr_i; 
        }

        ++i;
    }
    buffer[i] = '\0';

    return indices;
}

void two_op(char operation, FILE *translation)
{
    fputs("\t@SP\n\tA=M-1\n\tD=M\n", translation);
    fputs("\tA=A-1\n\tM=D", translation);
    fputc(operation, translation);
    fputs("M\n\t@SP\n\tM=M-1\n", translation);

    return;
}