// Compilador C simples para gerar bytecode JVM (formato Jasmin)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void remove_spaces(char *str) {
    int i, x = 0;
    for (i = 0; str[i]; i++)
        if (!isspace(str[i]) || (i > 0 && str[i - 1] != ' '))
            str[x++] = str[i];
    str[x] = 0;
}

void gerar_jasmin(const char *input_filename, const char *output_filename) {
    FILE *in = fopen(input_filename, "r");
    FILE *out = fopen(output_filename, "w");
    if (!in || !out) {
        printf("Erro ao abrir ficheiros.\n");
        return;
    }

    fprintf(out,
        ".class public Main\n"
        ".super java/lang/Object\n\n"
        ".method public static main([Ljava/lang/String;)V\n"
        "    .limit stack 10\n"
        "    .limit locals 10\n\n"
    );

    char line[256];
    int label = 0;
    int loop_label = 0;
    while (fgets(line, sizeof(line), in)) {
        remove_spaces(line);

        if (strncmp(line, "int", 3) == 0) {
            char var[32]; int val;
            if (sscanf(line, "int%s=%d;", var, &val) == 2) {
                if (val >= 0 && val <= 5)
                    fprintf(out, "    iconst_%d\n", val);
                else
                    fprintf(out, "    bipush %d\n", val);
                fprintf(out, "    istore_1\n");
            }
        }
        else if (strncmp(line, "while(", 6) == 0) {
            char var[32], op[3]; int val;
            sscanf(line, "while(%[^<>=!]=%d)", var, &val);
            char *p = strstr(line, var);
            p += strlen(var);
            strncpy(op, p, 2); op[2] = 0;

            fprintf(out, "L%d:\n", loop_label);
            fprintf(out, "    iload_1\n");
            if (val >= 0 && val <= 5)
                fprintf(out, "    iconst_%d\n", val);
            else
                fprintf(out, "    bipush %d\n", val);

            if (strcmp(op, "<") == 0)
                fprintf(out, "    if_icmpge L%d_end\n", loop_label);
            else if (strcmp(op, ">") == 0)
                fprintf(out, "    if_icmple L%d_end\n", loop_label);
            else if (strcmp(op, "==") == 0)
                fprintf(out, "    if_icmpne L%d_end\n", loop_label);
            else if (strcmp(op, "!=") == 0)
                fprintf(out, "    if_icmpeq L%d_end\n", loop_label);
            else if (strcmp(op, "<=") == 0)
                fprintf(out, "    if_icmpgt L%d_end\n", loop_label);
            else if (strcmp(op, ">=") == 0)
                fprintf(out, "    if_icmplt L%d_end\n", loop_label);
        }
        else if (strstr(line, "=") && strstr(line, "+")) {
            fprintf(out, "    iload_1\n");
            fprintf(out, "    iconst_1\n");
            fprintf(out, "    iadd\n");
            fprintf(out, "    istore_1\n");
        }
        else if (strncmp(line, "}", 1) == 0) {
            fprintf(out, "    goto L%d\n", loop_label);
            fprintf(out, "L%d_end:\n", loop_label);
            loop_label++;
        }
        else if (strncmp(line, "return", 6) == 0) {
            fprintf(out, "    return\n");
        }
    }

    fprintf(out, ".end method\n");
    fclose(in);
    fclose(out);
}

int main() {
    char input[256], output[256];
    printf("Ficheiro C de entrada: ");
    scanf("%s", input);
    sprintf(output, "%.*s.j", (int)(strlen(input) - 2), input);
    gerar_jasmin(input, output);
    printf("Código JVM gerado em %s\n", output);
    return 0;
}
