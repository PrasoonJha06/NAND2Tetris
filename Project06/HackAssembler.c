#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// typedef struct {
//     char name[20];
//     int address;
// } symbol;

char *tobinary(int num);
bool isnum(char *str);

int main(int argc, char *argv[])
{
    /*
    We for now are making a an assembler that 
    works if there are no symbols in the assembly
    file
    */

    // Checking there is only 1 input
    if (argc != 2) {
        printf("Please enter only 1 filename\n");
        return 1;
    }

    // Checking extension and generating translation's name
    char *ext_ptr = strstr(argv[1], ".asm\0");
    if (ext_ptr == NULL) {
        printf("Enter file ending with extension .asm\n");
        return 1;
    }
    int ext_index = ext_ptr - argv[1];
    char translation_name[ext_index + 6];
    for (int i = 0; i < ext_index; ++i)
        translation_name[i] = argv[1][i];
    translation_name[ext_index] = '\0';
    strcat(translation_name, ".hack");

    // Opening the assembly file in read mode
    FILE *program = fopen(argv[1], "r");
    if (program == NULL) {
        printf("Couldn't open file\n");
        return 1;
    }
    
    // Opening the binary translation in write mode
    FILE *translation = fopen(translation_name, "w");
    if (translation == NULL) {
        printf("Couldn't write .hack file\n");
        return 1;
    }

    // Time to start reading the program

    int line_length = 42; // For it is the answer
    char buffer[line_length]; 
    int line_no = 0;

    while(fgets(buffer, sizeof(buffer), program) != NULL) {
        bool text_there = false;
        int text_index;
        for (int i = 0; i < line_length; ++i) {
            if (buffer[i] != ' ') {
                text_there = true;
                text_index = i;
                break;
            }
        }

        // If there is no text, goto next iteration
        if (!text_there)
            continue;

        if (buffer[text_index] == '/') {
            if (buffer[text_index + 1] == '/')
                continue;
        }
        else if (buffer[text_index] == '(')
            continue;
        // A Instruction
        else if (buffer[text_index] == '@') {

            int address;

            // For symbols, we will use isnum()
            // Even then there will be two cases: 
            // Predefined symbol and Variable
            for (int i = text_index + 1; i < line_length; ++i) {
                if (buffer[i] == '\n')
                    buffer[i] = '\0';
            }
            char a_instruction[39]; // Arbitrary length
            // strncpy() copies until length is reached or it it hits '\0'
            strncpy(a_instruction, buffer + text_index + 1, 38);

            address = atoi(a_instruction);

            // Here on it's the same for both
            char *binary_address = tobinary(address);
            fputs(binary_address, translation);
            fputc('\n', translation);

            // free()ing the dynamically allocated string
            free(binary_address);
            
            ++line_no;
        }
        // C Instruction
        else {
            // TODO

            ++line_no;
        }
    }

    // Closing the files
    fclose(program);
    fclose(translation);

    return 0;
}

char *tobinary(int num)
{
    char *bus = malloc(17 * sizeof(char));
    for (int i = 0; i < 16; ++i)
        bus[i] = '0';
    bus[16] = '\0';

    if (num == 0)
        goto end;

    int just_larger;
    int i = 0;
    float divisor = 1.0;
    bool flag = true;
    while (flag) {

        if (num / divisor <= 1) {
            just_larger = divisor;
            flag = false;
        }
        ++i;
        divisor *= 2;
    }

    if (num % just_larger == 0) // 0 would have stumped this test
        bus[16 - i] = '1';
    else {
        /*
        If power of 2 smaller than just_larger gives 0
        remainder then, make its bit 1. Replace num with
        divided value and do it again for smaller value.
        i must decrement as well. It must continue until
        i becomes 0 (index becomes 16)
        */
        int n = just_larger / 2;
        while (i >= 0) {
            --i;

            if ((float) num / n > 1) {
                bus[16 - i] = '1';
                num %= n;
            }
            else if (num / n == 1) {
                bus[16 - i] = '1';
                break;
            }

            n /= 2;
        }
    }

    end:
        return bus;
}

// a_instruction not buffer will go inside it
bool isnum(char *str)
{
    int i = 0;
    while (str[i] != '\0') {
        if (!isdigit(str[i]))
            return false;
        ++i;
    }
    return true;
}