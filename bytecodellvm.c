// Compilador C para gerar LLVM IR (Intermediate Representation)
// Suporta: int, i=i+1, i=i-1, while, return
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

void gerar_llvm(const char *input_filename, const char *output_filename) {
    FILE *in = fopen(input_filename, "r");
    FILE *out = fopen(output_filename, "w");
    if (!in || !out) {
        printf("Erro ao abrir ficheiros.\n");
        return;
    }

    fprintf(out,
        "define i32 @main() {\n"
        "entry:\n"
        "  %%i = alloca i32\n"
    );

    char line[256];
    int label = 0;
    while (fgets(line, sizeof(line), in)) {
        remove_spaces(line);

        if (strncmp(line, "int", 3) == 0) {
            int val;
            if (sscanf(line, "inti=%d;", &val) == 1) {
                fprintf(out, "  store i32 %d, i32* %%i\n", val);
            }
        }
        else if (strstr(line, "i=i+1") || strstr(line, "i+=1")) {
            fprintf(out,
                "  %%tmp%d = load i32, i32* %%i\n"
                "  %%tmp%da = add i32 %%tmp%d, 1\n"
                "  store i32 %%tmp%da, i32* %%i\n",
                label, label, label, label);
            label++;
        }
        else if (strstr(line, "i=i-1") || strstr(line, "i-=1") || strstr(line, "i--")) {
            fprintf(out,
                "  %%tmp%d = load i32, i32* %%i\n"
                "  %%tmp%ds = sub i32 %%tmp%d, 1\n"
                "  store i32 %%tmp%ds, i32* %%i\n",
                label, label, label, label);
            label++;
        }
        else if (strncmp(line, "while(", 6) == 0) {
            int val;
            sscanf(line, "while(i<%d)", &val);
            fprintf(out,
                "  br label \%%d\n"
                "L%d:\n"
                "  %%tmp%dc = load i32, i32* %%i\n"
                "  %%cmp%d = icmp slt i32 %%tmp%dc, %d\n"
                "  br i1 %%cmp%d, label %%L%dbody, label %%L%done\n"
                "L%dbody:\n",
                label, label, label, label, label, val, label, label, label);
        }
        else if (strncmp(line, "}", 1) == 0) {
            fprintf(out,
                "  br label %%L%d\n"
                "L%done:\n",
                label, label);
            label++;
        }
        else if (strncmp(line, "return", 6) == 0) {
            fprintf(out, "  ret i32 0\n");
        }
    }

    fprintf(out, "}\n");
    fclose(in);
    fclose(out);
}

int main() {
    char input[256], output[256];
    printf("\033c\033[43;30m\n");
    printf("Ficheiro C de entrada: ");
    scanf("%s", input);
    sprintf(output, "%.*s.ll", (int)(strlen(input) - 2), input);
    gerar_llvm(input, output);
    printf("Código LLVM IR gerado em %s\n", output);
    return 0;
}
