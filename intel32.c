// Compilador simples C para Assembly x86 32-bit
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int label_id = 0;

void remove_spaces(char *str) {
    int i, x = 0;
    for (i = 0; str[i]; i++)
        if (!isspace(str[i]) || (i > 0 && str[i-1] != ' '))
            str[x++] = str[i];
    str[x] = 0;
}

void gerar_asm_x86(const char *input_filename, const char *output_filename) {
    FILE *in = fopen(input_filename, "r");
    FILE *out = fopen(output_filename, "w");
    if (!in || !out) {
        printf("Erro ao abrir ficheiro.\n");
        return;
    }

    fprintf(out, "section .text\n    global _start\n\n");

    char line[1024], current_func[128] = "";
    FILE *data = tmpfile();
    char funcs[100][100]; int func_count = 0;

    while (fgets(line, sizeof(line), in)) {
        remove_spaces(line);
        if (strncmp(line, "void", 4) == 0) {
            sscanf(line, "void%[^()]()", current_func);
            fprintf(out, "%s:\n", current_func);
            strcpy(funcs[func_count++], current_func);
        }
        else if (strstr(line, "int") == line) {
            char var[64]; int val;
            sscanf(line, "int%[^=]=%d;", var, &val);
            fprintf(data, "%s_%s: dd %d\n", var, current_func, val);
        }
        else if (strstr(line, "char*") == line) {
            char var[64], text[512];
            sscanf(line, "char*%[^=]=\"%[^\"]", var, text);
            fprintf(data, "%s_%s: db \"%s\", 0\n", var, current_func, text);
        }
        else if (strstr(line, "call") != NULL) {
            char fname[64];
            sscanf(line, "call%s();", fname);
            fprintf(out, "    call %s\n", fname);
        }
        else if (strstr(line, "return") == line) {
            int val;
            if (sscanf(line, "return%d;", &val) == 1)
                fprintf(out, "    mov eax, %d\n", val);
            fprintf(out, "    ret\n");
        }
        else if (strstr(line, "if(") == line) {
            char var[64], op[3]; int val;
            sscanf(line, "if(%[^<>=!]=%d)", var, &val);
            char *p = strstr(line, var);
            p += strlen(var);
            strncpy(op, p, 2); op[2] = 0;
            int n = label_id++;
            fprintf(out, "    mov eax, [%s_%s]\n", var, current_func);
            fprintf(out, "    cmp eax, %d\n", val);
            if (strcmp(op, "==") == 0) fprintf(out, "    jne endif_%d\n", n);
            else if (strcmp(op, "!=") == 0) fprintf(out, "    je endif_%d\n", n);
            else if (strcmp(op, "<") == 0) fprintf(out, "    jge endif_%d\n", n);
            else if (strcmp(op, ">") == 0) fprintf(out, "    jle endif_%d\n", n);
            else if (strcmp(op, "<=") == 0) fprintf(out, "    jg endif_%d\n", n);
            else if (strcmp(op, ">=") == 0) fprintf(out, "    jl endif_%d\n", n);
            fprintf(out, "endif_%d:\n", n);
        }
        else if (strstr(line, "while(") == line) {
            char var[64], op[3]; int val;
            sscanf(line, "while(%[^<>=!]=%d)", var, &val);
            char *p = strstr(line, var);
            p += strlen(var);
            strncpy(op, p, 2); op[2] = 0;
            int n = label_id++;
            fprintf(out, "loop_%d:\n", n);
            fprintf(out, "    mov eax, [%s_%s]\n", var, current_func);
            fprintf(out, "    cmp eax, %d\n", val);
            if (strcmp(op, "==") == 0) fprintf(out, "    jne endloop_%d\n", n);
            else if (strcmp(op, "!=") == 0) fprintf(out, "    je endloop_%d\n", n);
            else if (strcmp(op, "<") == 0) fprintf(out, "    jge endloop_%d\n", n);
            else if (strcmp(op, ">") == 0) fprintf(out, "    jle endloop_%d\n", n);
            else if (strcmp(op, "<=") == 0) fprintf(out, "    jg endloop_%d\n", n);
            else if (strcmp(op, ">=") == 0) fprintf(out, "    jl endloop_%d\n", n);
            fprintf(out, "    jmp loop_%d\n", n);
            fprintf(out, "endloop_%d:\n", n);
        }
    }

    for (int i = 0; i < func_count; i++) {
        int found = 0;
        rewind(in);
        while (fgets(line, sizeof(line), in))
            if (strstr(line, funcs[i])) { found = 1; break; }
        if (!found)
            fprintf(out, "%s:\n    ret\n", funcs[i]);
    }

    fprintf(out, "\nsection .data\n");
    rewind(data);
    while (fgets(line, sizeof(line), data))
        fputs(line, out);

    fclose(in); fclose(out); fclose(data);
}

int main() {
    char input[256], output[256];
    printf("\033c\033[43;30m\n");
    printf("Ficheiro C de entrada: ");
    scanf("%s", input);
    sprintf(output, "%.*s.S", (int)(strlen(input)-2), input);
    gerar_asm_x86(input, output);
    printf("Assembly x86 gerado em %s\n", output);
    return 0;
}
