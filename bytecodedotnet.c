// Compilador C para gerar bytecode .NET CIL (Common Intermediate Language)
// Suporta: int, i=i+1, i=i-1, if, while, return (simples)
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

void gerar_cil(const char *input_filename, const char *output_filename) {
    FILE *in = fopen(input_filename, "r");
    FILE *out = fopen(output_filename, "w");
    if (!in || !out) {
        printf("Erro ao abrir ficheiros.\n");
        return;
    }

    fprintf(out,
        ".assembly extern mscorlib {}\n"
        ".assembly programa {}\n"
        ".method static void Main() cil managed\n"
        "{\n"
        "  .entrypoint\n"
        "  .maxstack 10\n"
        "  .locals init ([0] int32 i)\n"
    );

    char line[256];
    int label = 0;
    while (fgets(line, sizeof(line), in)) {
        remove_spaces(line);

        if (strncmp(line, "int", 3) == 0) {
            int val;
            if (sscanf(line, "inti=%d;", &val) == 1) {
                fprintf(out, "  ldc.i4.%d\n  stloc.0\n", val);
            }
        }
        else if (strstr(line, "i=i+1") || strstr(line, "i+=1")) {
            fprintf(out, "  ldloc.0\n  ldc.i4.1\n  add\n  stloc.0\n");
        }
        else if (strstr(line, "i=i-1") || strstr(line, "i-=1") || strstr(line, "i--")) {
            fprintf(out, "  ldloc.0\n  ldc.i4.1\n  sub\n  stloc.0\n");
        }
        else if (strncmp(line, "while(", 6) == 0) {
            int val;
            sscanf(line, "while(i<%d)", &val);
            fprintf(out, "L%d:\n", label);
            fprintf(out, "  ldloc.0\n  ldc.i4.%d\n  bge L%d_end\n", val, label);
        }
        else if (strncmp(line, "if(", 3) == 0) {
            int val;
            sscanf(line, "if(i<%d)", &val);
            fprintf(out, "  ldloc.0\n  ldc.i4.%d\n  bge L%d_end\n", val, label);
        }
        else if (strncmp(line, "break;", 6) == 0) {
            fprintf(out, "  br L%d_end\n", label);
        }
        else if (strncmp(line, "}", 1) == 0) {
            fprintf(out, "  br L%d\n", label);
            fprintf(out, "L%d_end:\n", label);
            label++;
        }
        else if (strncmp(line, "return", 6) == 0) {
            fprintf(out, "  ret\n");
        }
    }

    fprintf(out, "  ret\n}\n");
    fclose(in);
    fclose(out);
}

int main() {
    char input[256], output[256];
    printf("\033c\033[43;30m\n");
    printf("Ficheiro C de entrada: ");
    scanf("%s", input);
    sprintf(output, "%.*s.il", (int)(strlen(input) - 2), input);
    gerar_cil(input, output);
    printf("Código CIL gerado em %s\n", output);
    return 0;
}
