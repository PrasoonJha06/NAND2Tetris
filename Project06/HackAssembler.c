#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COMP_TABLE_SIZE 28
#define LINE_LENGTH 100

// typedef struct {
//     char name[20];
//     int address;
// } symbol;

typedef struct {
    char code[4]; // With '\0' 
    char bits[8]; // 1 a bit and 6 c bits
} comp_bin;

char *tobinary(int num);
bool isnum(char *str);

int main(int argc, char *argv[])
{
    /*
    We for now are making a an assembler that 
    works if there are no symbols in the assembly
    file
    */

    comp_bin comp_table[COMP_TABLE_SIZE] = {
        {"0", "0101010"}, {"1", "0111111"},
        {"-1", "0111010"}, {"D", "0001100"},
        {"A", "0110000"}, {"!D", "0001101"},
        {"!A", "0110001"}, {"-D", "0001111"},
        {"-A", "0110011"}, {"D+1", "0011111"},
        {"A+1", "0110111"}, {"D-1", "0001110"},
        {"A-1", "0110010"}, {"D+A", "0000010"},
        {"D-A", "0010011"}, {"A-D", "0000111"},
        {"D&A", "0000000"}, {"D|A", "0010101"},
        {"M", "1110000"}, {"!M", "1110001"},
        {"-M", "1110011"}, {"M+1", "1110111"},
        {"M-1", "1110010"}, {"D+M", "1000010"},
        {"D-M", "1010011"}, {"M-D", "1000111"},
        {"D&M", "1000000"}, {"D|M", "1010101"}
    };

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

    char buffer[LINE_LENGTH]; 
    int line_no = 0;

    while(fgets(buffer, sizeof(buffer), program) != NULL) {
        if (buffer[0] == '\n') {
            continue;
        }

        int text_index;
        for (int i = 0; i < LINE_LENGTH; ++i) {
            if (buffer[i] != ' ') {
                text_index = i;
                break;
            }
        }

        // Comment
        if (buffer[text_index] == '/') {
            continue;
        }
        // Label
        else if (buffer[text_index] == '(') {
            continue;
        }
        // A Instruction
        else if (buffer[text_index] == '@') {
            int address;

            // For symbols, we will use isnum()
            // Even then there will be two cases: 0
            // Predefined symbol and Variable
            for (int i = text_index + 1; i < LINE_LENGTH; ++i) {
                if (buffer[i] == '\n')
                    buffer[i] = '\0';
            }
            char a_instruction[38]; // Arbitrary length
            // strncpy() copies until length is reached or it it hits '\0'
            strncpy(a_instruction, buffer + text_index + 1, 37);

            address = atoi(a_instruction);

            // Here on it's the same for both
            char *binary_address = tobinary(address);
            fputs(binary_address, translation);
            fputc('\n', translation);

            // free()ing the dynamically allocated string
            free(binary_address);
        }
        // C Instruction
        else {
            // Let's parse the instruction
            char dest[5] = "";
            char comp[5] = "";
            char jump[5] = "";
            char *equal_ptr = strchr(buffer, '=');
            char *scolon_ptr = strchr(buffer, ';');
            char *newl_ptr = strchr(buffer, '\n');

            if ((equal_ptr == NULL) && (scolon_ptr == NULL)) {
                strcpy(dest, "null");
                strcpy(jump, "null");

                *newl_ptr = '\0'; // Replace '\n' with '\0'
                // strncpy() terminates if it hits '\0'
                strncpy(comp, buffer + text_index, 3);
            }
            else if ((equal_ptr != NULL) && (scolon_ptr == NULL)) {
                strcpy(jump, "null");

                int equal_index = equal_ptr - buffer;
                *equal_ptr = '\0';
                *newl_ptr = '\0';
                strncpy(dest, buffer + text_index, 3);
                strncpy(comp, buffer + equal_index + 1, 3);
            }
            else if ((equal_ptr == NULL) && (scolon_ptr != NULL)) {
                strcpy(dest, "null");

                int scolon_index = scolon_ptr - buffer;
                *scolon_ptr = '\0';
                *newl_ptr = '\0';
                strncpy(comp, buffer + text_index, 3);
                strncpy(jump, buffer + scolon_index + 1, 3);
            }
            else {
                int equal_index = equal_ptr - buffer;
                int scolon_index = scolon_ptr - buffer;
                *equal_ptr = '\0';
                *scolon_ptr = '\0';
                *newl_ptr = '\0';
                strncpy(dest, buffer + text_index, 3);
                strncpy(comp, buffer + equal_index + 1, 3);
                strncpy(jump, buffer + scolon_index + 1, 3);
            }

            // C-instruction in binary
            char instruction[17];
            for (int i = 0; i < 3; ++i)
                instruction[i] = '1';
            for (int i = 3; i < 16; ++i)
                instruction[i] = '0';
            instruction[16] = '\0';

            // Dest bits
            if (strchr(dest, 'A') != NULL)
                instruction[10] = '1';
            if (strchr(dest, 'D') != NULL)
                instruction[11] = '1';
            if (strchr(dest, 'M') != NULL)
                instruction[12] = '1';

            // Jump bits
            if (strchr(jump, 'N') != NULL) {
                instruction[13] = '1';
                instruction[15] = '1';
            }
            else if (strchr(jump, 'P') != NULL) {
                instruction[13] = '1';
                instruction[14] = '1';
                instruction[15] = '1';
            }
            else {
                if (strchr(jump, 'L') != NULL)
                    instruction[13] = '1';
                if (strchr(jump, 'E') != NULL)
                    instruction[14] = '1';
                if (strchr(jump, 'G') != NULL)
                    instruction[15] = '1';
            }

            // Comp bits
            for (int i = 0; i < COMP_TABLE_SIZE; ++i) {
                if (strcmp(comp, comp_table[i].code) == 0) {
                    strncpy((instruction + 3), comp_table[i].bits, 7);
                    continue;
                }
            }

            // Writing on the file
            fputs(instruction, translation);
            fputc('\n', translation);
        }
    }

    // Closing the files
    fclose(program);
    fclose(translation);

    return 0;
}

char *tobinary(int num)
{
    char *bus = malloc(17);
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