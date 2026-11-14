#ifndef _WIN32
#define _POSIX_C_SOURCE 200809L  // for nanosleep on Linux and macOS
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef _WIN32
#define strcasecmp _stricmp
#else
#include <strings.h>
#endif
#include "processor.c"


#define MAX_LINES 1024          // the instruction segment is addresses 0 to 1023
#define MAX_LINE_LENGTH 256


char* get_opcode_binary(const char* mnemonic) {
    // Use static buffer to return string
    static char* binary = NULL;

    if (strcasecmp(mnemonic, "ADD") == 0) binary = "0000";
    else if (strcasecmp(mnemonic, "SUB") == 0) binary = "0001";
    else if (strcasecmp(mnemonic, "MUL") == 0) binary = "0010";
    else if (strcasecmp(mnemonic, "MOVI") == 0) binary = "0011";
    else if (strcasecmp(mnemonic, "JEQ") == 0) binary = "0100";
    else if (strcasecmp(mnemonic, "AND") == 0) binary = "0101";
    else if (strcasecmp(mnemonic, "XORI") == 0) binary = "0110";
    else if (strcasecmp(mnemonic, "JMP") == 0) binary = "0111";
    else if (strcasecmp(mnemonic, "LSL") == 0) binary = "1000";
    else if (strcasecmp(mnemonic, "LSR") == 0) binary = "1001";
    else if (strcasecmp(mnemonic, "MOVR") == 0) binary = "1010";
    else if (strcasecmp(mnemonic, "MOVM") == 0) binary = "1011";
    else binary = NULL;

    return binary;
}

char* get_register_binary(const char* reg) {
    if (reg[0] != 'R' || reg[1] == '\0' || strlen(reg) > 3) return NULL;
    for (const char* c = reg + 1; *c; c++) {
        if (!isdigit((unsigned char)*c)) return NULL;
    }

    int num = atoi(&reg[1]);
    if (num < 0 || num > 31) return NULL;

    char* binary = malloc(6);  // 5 bits + '\0'
    for (int i = 4; i >= 0; i--) {
        binary[4 - i] = ((num >> i) & 1) + '0';
    }
    binary[5] = '\0';
    return binary;
}


char* get_13bit_binary(const char* str) {
    static char binary[14];  // 13 bits + null terminator

    int num = atoi(str);  // convert string to int

    // handle negative values as 2's complement (optional)
    if (num < 0) {
        num = (1 << 13) + num;  // 2's complement for 13 bits
    }

    for (int i = 12; i >= 0; i--) {
        binary[12 - i] = ((num >> i) & 1) + '0';
    }

    binary[13] = '\0';  // null-terminate
    return binary;
}


char* get_18bit_binary(const char* str) {
    static char binary[19];  // 18 bits + null terminator

    int num = atoi(str);  // convert string to int

    // Handle negative values using 2's complement (optional)
    if (num < 0) {
        num = (1 << 18) + num;  // Apply 2's complement for 18 bits
    }

    for (int i = 17; i >= 0; i--) {
        binary[17 - i] = ((num >> i) & 1) + '0';
    }

    binary[18] = '\0';  // Null-terminate
    return binary;
}


char* get_28bit_binary(const char* str) {
    static char binary[29];  // 28 bits + null terminator

    int num = atoi(str);  // Convert string to int

    // Handle negative values using 2's complement for 28 bits
    if (num < 0) {
        num = (1 << 28) + num;
    }

    for (int i = 27; i >= 0; i--) {
        binary[27 - i] = ((num >> i) & 1) + '0';
    }

    binary[28] = '\0';  // Null-terminate
    return binary;
}



// Checks that a number fits in the given range; prints an error with the line number if not.
int check_number(const char* str, long min, long max, int line_no, const char* what) {
    char* end;
    long value = strtol(str, &end, 10);
    if (*str == '\0' || *end != '\0') {
        printf("Error on line %d: \"%s\" is not a number (%s)\n", line_no, str, what);
        return 0;
    }
    if (value < min || value > max) {
        printf("Error on line %d: %s %ld is out of range (%ld to %ld)\n", line_no, what, value, min, max);
        return 0;
    }
    return 1;
}

// Checks the operands of one instruction before it is translated.
int validate(char** words, int word_count, int line_no) {
    const char* m = words[0];
    int expected;
    if (get_opcode_binary(m) == NULL) {
        printf("Error on line %d: unknown instruction \"%s\"\n", line_no, m);
        return 0;
    }
    if (strcmp(m, "JMP") == 0) expected = 2;
    else if (strcmp(m, "MOVI") == 0) expected = 3;
    else expected = 4;
    if (word_count != expected) {
        printf("Error on line %d: %s takes %d operands, found %d\n", line_no, m, expected - 1, word_count - 1);
        return 0;
    }
    if (strcmp(m, "JMP") == 0)
        return check_number(words[1], 0, (1L << 28) - 1, line_no, "jump address");

    // Every other instruction starts with registers
    int registers = (strcmp(m, "MOVI") == 0) ? 1 : (strcmp(m, "ADD") == 0 || strcmp(m, "SUB") == 0 || strcmp(m, "MUL") == 0 || strcmp(m, "AND") == 0) ? 3 : 2;
    for (int r = 1; r <= registers; r++) {
        char* bits = get_register_binary(words[r]);
        if (bits == NULL) {
            printf("Error on line %d: \"%s\" is not a register (R0 to R31)\n", line_no, words[r]);
            return 0;
        }
        free(bits);
    }
    if (registers == 3) return 1;
    if (strcmp(m, "LSL") == 0 || strcmp(m, "LSR") == 0)
        return check_number(words[3], 0, (1L << 13) - 1, line_no, "shift amount");
    return check_number(words[registers + 1], -(1L << 17), (1L << 17) - 1, line_no, "immediate");
}

/* Usage: processor [program file] [--delay ms]
   The program file defaults to instructions.txt and the delay between cycles to 500 ms. */
int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);  // show the emojis in the Windows console
#endif
    const char* path = "instructions.txt";
    for (int a = 1; a < argc; a++) {
        if (strcmp(argv[a], "--delay") == 0 && a + 1 < argc) cycle_delay_ms = atoi(argv[++a]);
        else path = argv[a];
    }

    init();
    FILE* file = fopen(path, "r");
    if (file == NULL) {
        printf("Error: could not open %s\n", path);
        return 1;
    }

    char* instructions[MAX_LINES];
    int line_numbers[MAX_LINES];
    char line[MAX_LINE_LENGTH];
    int count = 0;
    int line_no = 0;

    while (fgets(line, sizeof(line), file) != NULL) {
        line_no++;
        // Remove comments (# or ;) and the end of the line (\n, or \r\n from Windows files)
        line[strcspn(line, "#;\r\n")] = '\0';

        // Treat non-breaking spaces (UTF-8 bytes C2 A0, often pasted from documents) as normal spaces
        for (char* c = line; *c; c++) {
            if ((unsigned char)c[0] == 0xC2 && (unsigned char)c[1] == 0xA0) { c[0] = ' '; c[1] = ' '; }
        }

        // Skip lines that are empty or only spaces
        int blank = 1;
        for (char* c = line; *c; c++) {
            if (!isspace((unsigned char)*c)) blank = 0;
            *c = (char)toupper((unsigned char)*c);
        }
        if (blank) continue;

        if (count == MAX_LINES) {
            printf("Error: the program has more than %d instructions\n", MAX_LINES);
            return 1;
        }

        // Allocate memory and copy the line
        instructions[count] = malloc(strlen(line) + 1);
        strcpy(instructions[count], line);
        line_numbers[count] = line_no;
        count++;
    }

    fclose(file);

    // ✅ Parse each instruction line into words
for (int i = 0; i < count; i++) {
    printf("\nInstruction %d: %s\n", i + 1, instructions[i]);

    // Create a copy of the line (because strtok modifies the string)
    char line_copy[MAX_LINE_LENGTH];
    strcpy(line_copy, instructions[i]);

    // Array of word pointers
    char* words[10];  // Assume max 10 words per instruction
    int word_count = 0;

    // Tokenize the line by spaces, tabs and commas
    char* token = strtok(line_copy, " \t,");
    while (token != NULL && word_count < 10) {
        words[word_count++] = token;
        token = strtok(NULL, " \t,");
    }

    // Print the words
    printf("Words: [");
    for (int j = 0; j < word_count; j++) {
        printf("%s", words[j]);
        if (j < word_count - 1) printf(", ");
    }
    printf("]\n");
    if (!validate(words, word_count, line_numbers[i])) {
        return 1;
    }
    char res[33];  // 32 bits + '\0'
    if(strcmp(words[0], "ADD") == 0 || strcmp(words[0], "SUB") == 0 || strcmp(words[0], "MUL") == 0 || strcmp(words[0], "AND") == 0){
      char* a = get_opcode_binary(words[0]);
      char* b = get_register_binary(words[1]);
      char* c = get_register_binary(words[2]);
      char* d = get_register_binary(words[3]);
      strcpy(res, a);
      strcat(res,b);
      strcat(res,c);
      strcat(res,d);
      strcat(res,"0000000000000");
    }
    if(strcmp(words[0], "MOVI") == 0){
      char* e = get_opcode_binary(words[0]);
      char* f = get_register_binary(words[1]);
      char* g = get_18bit_binary(words[2]);
      strcpy(res, e); 
      strcat(res,f);
      strcat(res,"00000");
      strcat(res,g);

    }
    if(strcmp(words[0], "JEQ") == 0 || strcmp(words[0], "XORI") == 0 || strcmp(words[0], "MOVR") == 0 || strcmp(words[0], "MOVM") == 0){
      char* h = get_opcode_binary(words[0]);
      char* i = get_register_binary(words[1]);
      char* j = get_register_binary(words[2]);
      char* k = get_18bit_binary(words[3]);

      strcpy(res, h); 
      strcat(res,i);
      strcat(res,j);
      strcat(res,k);
    }
    if(strcmp(words[0], "JMP") == 0){
      char* l = get_opcode_binary(words[0]);
      char* m = get_28bit_binary(words[1]);
      strcpy(res, l); 
      strcat(res,m);
    }
    if(strcmp(words[0], "LSL") == 0 || strcmp(words[0], "LSR") == 0){
      char* n = get_opcode_binary(words[0]);
      char* o = get_register_binary(words[1]);
      char* p = get_register_binary(words[2]);
      char* q = get_13bit_binary(words[3]);
      strcpy(res, n); 
      strcat(res,o);
      strcat(res,p);
      strcat(res,"00000");
      strcat(res,q);
      
    }
    printf("Result: %s\n",res);
    int zz = convert_from_binary_string_to_int(res);

    write_int_into_register(zz,ram[i]);

}

    // ✅ Free allocated memory
    for (int i = 0; i < count; i++) {
        free(instructions[i]);
    }
    imain(count);

    return 0;
}
