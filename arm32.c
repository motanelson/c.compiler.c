#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_FUNCOES 100
#define MAX_LINHAS 1000
#define MAX_NOME 64
#define MAX_LINHA 512

typedef struct {
    char nome[MAX_NOME];
    char linhas[MAX_LINHAS][MAX_LINHA];
    int linha_count;
    char vars[MAX_LINHAS][MAX_LINHA];
    int var_count;
} Subrotina;

Subrotina funcoes[MAX_FUNCOES];
int func_count = 0;
char chamadas_indefinidas[MAX_FUNCOES][MAX_NOME];
int chamadas_count = 0;

int if_count = 0;
int while_count = 0;

void trim(char* s) {
    char* p = s;
    int l = strlen(p);
    while (l > 0 && isspace(p[l - 1])) p[--l] = 0;
    while (*p && isspace(*p)) ++p, --l;
    memmove(s, p, l + 1);
}

void adicionar_chamada_indefinida(const char* nome) {
    for (int i = 0; i < func_count; i++)
        if (strcmp(funcoes[i].nome, nome) == 0) return;

    for (int i = 0; i < chamadas_count; i++)
        if (strcmp(chamadas_indefinidas[i], nome) == 0) return;

    strcpy(chamadas_indefinidas[chamadas_count++], nome);
}

void processar_linha(Subrotina* f, const char* linha) {
    char l[MAX_LINHA];
    strcpy(l, linha);
    trim(l);

    if (strncmp(l, "call ", 5) == 0) {
        char* destino = l + 5;
        trim(destino);
        sprintf(f->linhas[f->linha_count++], "bl %s", destino);
        adicionar_chamada_indefinida(destino);

    } else if (strncmp(l, "int ", 4) == 0) {
        char* nome = strtok(l + 4, "=");
        char* valor = strtok(NULL, ";");
        if (nome && valor) {
            trim(nome);
            trim(valor);
            sprintf(f->vars[f->var_count++], "%s_%s: .word %s", nome, f->nome, valor);
        }

    } else if (strstr(l, "char*") || strstr(l, "char *")) {
        char* nome = strtok(strstr(l, "*") + 1, "=");
        char* valor = strtok(NULL, ";");
        if (nome && valor) {
            trim(nome);
            trim(valor);
            if (valor[0] == '"') valor++;
            char* end = strchr(valor, '"');
            if (end) *end = '\0';
            sprintf(f->vars[f->var_count++], "%s_%s: .asciz \"%s\"", nome, f->nome, valor);
        }

    } else if (strncmp(l, "return ", 7) == 0) {
        char* val = l + 7;
        trim(val);
        sprintf(f->linhas[f->linha_count++], "ldr r0, =%s", val);
        sprintf(f->linhas[f->linha_count++], "bx lr");

    } else if (strncmp(l, "if (", 4) == 0) {
        char var[32], op[3], val[32];
        sscanf(l, "if (%31[^=<>!] %2[=!<>] %31[^)]", var, op, val);
        char label[32];
        sprintf(label, "if_end_%d", if_count++);
        trim(var); trim(val);

        sprintf(f->linhas[f->linha_count++], "ldr r1, =%s_%s", var, f->nome);
        sprintf(f->linhas[f->linha_count++], "ldr r2, [r1]");
        sprintf(f->linhas[f->linha_count++], "mov r3, #%s", val);
        sprintf(f->linhas[f->linha_count++], "cmp r2, r3");

        if (strstr(l, "==")) sprintf(f->linhas[f->linha_count++], "bne %s", label);
        else if (strstr(l, "!=")) sprintf(f->linhas[f->linha_count++], "beq %s", label);
        else if (strstr(l, ">=")) sprintf(f->linhas[f->linha_count++], "blt %s", label);
        else if (strstr(l, "<=")) sprintf(f->linhas[f->linha_count++], "bgt %s", label);
        else if (strstr(l, ">")) sprintf(f->linhas[f->linha_count++], "ble %s", label);
        else if (strstr(l, "<")) sprintf(f->linhas[f->linha_count++], "bge %s", label);

        sprintf(f->linhas[f->linha_count++], "; ... (bloco do if)");
        sprintf(f->linhas[f->linha_count++], "%s:", label);

    } else if (strncmp(l, "while (", 7) == 0) {
        char var[32], op[3], val[32];
        sscanf(l, "while (%31[^=<>!] %2[=!<>] %31[^)]", var, op, val);
        char label_topo[32], label_fim[32];
        sprintf(label_topo, "loop_%d", while_count);
        sprintf(label_fim, "endloop_%d", while_count);
        while_count++;
        trim(var); trim(val);

        sprintf(f->linhas[f->linha_count++], "%s:", label_topo);
        sprintf(f->linhas[f->linha_count++], "ldr r1, =%s_%s", var, f->nome);
        sprintf(f->linhas[f->linha_count++], "ldr r2, [r1]");
        sprintf(f->linhas[f->linha_count++], "mov r3, #%s", val);
        sprintf(f->linhas[f->linha_count++], "cmp r2, r3");

        if (strstr(l, "==")) sprintf(f->linhas[f->linha_count++], "bne %s", label_fim);
        else if (strstr(l, "!=")) sprintf(f->linhas[f->linha_count++], "beq %s", label_fim);
        else if (strstr(l, ">=")) sprintf(f->linhas[f->linha_count++], "blt %s", label_fim);
        else if (strstr(l, "<=")) sprintf(f->linhas[f->linha_count++], "bgt %s", label_fim);
        else if (strstr(l, ">")) sprintf(f->linhas[f->linha_count++], "ble %s", label_fim);
        else if (strstr(l, "<")) sprintf(f->linhas[f->linha_count++], "bge %s", label_fim);

        sprintf(f->linhas[f->linha_count++], "; ... (bloco do while)");
        sprintf(f->linhas[f->linha_count++], "b %s", label_topo);
        sprintf(f->linhas[f->linha_count++], "%s:", label_fim);

    } else {
        sprintf(f->linhas[f->linha_count++], "; ignorado: %s", l);
    }
}

void processar_codigo(const char* codigo) {
    const char* p = codigo;
    while ((p = strstr(p, "void ")) != NULL) {
        p += 5;
        char nome[MAX_NOME] = {0};
        sscanf(p, "%s", nome);
        char* abre = strchr(p, '{');
        if (!abre) break;
        abre++;
        char* fecha = strchr(abre, '}');
        if (!fecha) break;

        Subrotina* f = &funcoes[func_count++];
        strcpy(f->nome, nome);
        f->linha_count = 0;
        f->var_count = 0;

        char buffer[MAX_LINHA];
        const char* linha = abre;
        while (linha < fecha) {
            const char* prox = strchr(linha, '\n');
            int len = prox ? (prox - linha) : strlen(linha);
            strncpy(buffer, linha, len);
            buffer[len] = '\0';
            processar_linha(f, buffer);
            if (!prox) break;
            linha = prox + 1;
        }
        p = fecha + 1;
    }
}

void gravar_saida(const char* nome_saida) {
    FILE* f = fopen(nome_saida, "w");
    if (!f) {
        perror("Erro ao gravar");
        return;
    }

    fprintf(f, ".text\n\n");

    for (int i = 0; i < func_count; i++) {
        fprintf(f, ".global %s\n%s:\n", funcoes[i].nome, funcoes[i].nome);
        for (int j = 0; j < funcoes[i].linha_count; j++)
            fprintf(f, "    %s\n", funcoes[i].linhas[j]);
        fprintf(f, "\n");
    }

    fprintf(f, ".data\n\n");
    for (int i = 0; i < func_count; i++)
        for (int j = 0; j < funcoes[i].var_count; j++)
            fprintf(f, "%s\n", funcoes[i].vars[j]);

    for (int i = 0; i < chamadas_count; i++) {
        int definida = 0;
        for (int j = 0; j < func_count; j++)
            if (strcmp(funcoes[j].nome, chamadas_indefinidas[i]) == 0)
                definida = 1;
        if (!definida)
            fprintf(f, ".global %s\n%s:\n    bx lr\n\n", chamadas_indefinidas[i], chamadas_indefinidas[i]);
    }

    fclose(f);
    printf("Compilação concluída: %s\n", nome_saida);
}

int main() {
    char nome[256];
    printf("\033c\033[43;30m\nNome do ficheiro C (.c): ");
    scanf("%s", nome);

    FILE* f = fopen(nome, "r");
    if (!f) {
        perror("Erro ao abrir o ficheiro");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char* buffer = (char*)malloc(size + 1);
    fread(buffer, 1, size, f);
    buffer[size] = '\0';
    fclose(f);

    processar_codigo(buffer);

    char saida[256];
    strcpy(saida, nome);
    char* ponto = strrchr(saida, '.');
    if (ponto) strcpy(ponto, ".s");
    else strcat(saida, ".s");

    gravar_saida(saida);
    free(buffer);
    return 0;
}
