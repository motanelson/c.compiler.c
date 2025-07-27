#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_FUNCOES 128
#define MAX_LINHAS 128
#define MAX_NOME 64
#define MAX_CODIGO 128

typedef struct {
    char nome[MAX_NOME];
    char linhas[MAX_LINHAS][MAX_CODIGO];
    int total;
} Funcao;

Funcao funcoes[MAX_FUNCOES];
int total_funcoes = 0;

char* trim(char* str) {
    while (isspace(*str)) str++;
    if (*str == 0) return str;
    char* end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    *(end + 1) = 0;
    return str;
}

Funcao* encontra_ou_cria_funcao(const char* nome) {
    for (int i = 0; i < total_funcoes; i++) {
        if (strcmp(funcoes[i].nome, nome) == 0) return &funcoes[i];
    }
    strcpy(funcoes[total_funcoes].nome, nome);
    funcoes[total_funcoes].total = 0;
    return &funcoes[total_funcoes++];
}

void adiciona_codigo(const char* nome, const char* linha) {
    Funcao* f = encontra_ou_cria_funcao(nome);
    if (f->total < MAX_LINHAS) {
        strncpy(f->linhas[f->total++], linha, MAX_CODIGO - 1);
    }
}

int eh_declaracao_global(char* linha, char* nome_var) {
    if (strncmp(linha, "int ", 4) == 0 && strchr(linha, '=')) {
        sscanf(linha, "int %s", nome_var);
        char* igual = strchr(nome_var, '=');
        if (igual) *igual = '\0';
        char* p = nome_var;
        while (*p) {
            if (*p == ';') *p = '\0';
            p++;
        }
        return 1;
    }
    return 0;
}

void processa_linha(char* funcao, char* linha) {
    linha = trim(linha);

    if (strstr(linha, "a = a + b")) {
        adiciona_codigo(funcao, "    mov ax, [a]");
        adiciona_codigo(funcao, "    add ax, [b]");
        adiciona_codigo(funcao, "    mov [a], ax");
    }
    else if (strstr(linha, "a--")) {
        adiciona_codigo(funcao, "    mov ax, [a]");
        adiciona_codigo(funcao, "    dec ax");
        adiciona_codigo(funcao, "    mov [a], ax");
    }
    else if (strstr(linha, "return a")) {
        adiciona_codigo(funcao, "    mov ax, [a]");
        adiciona_codigo(funcao, "    ret");
    }
    else if (strstr(linha, "return")) {
        adiciona_codigo(funcao, "    ret");
    }
    else if (strchr(linha, '(') && strchr(linha, ')') && strstr(linha, ";")) {
        char chamada[64] = {0};
        sscanf(linha, "%[^(); \t]", chamada);
        char* p = chamada;
        while (*p && !isalpha(*p)) p++;
        if (*p) {
            char instrucao[128];
            snprintf(instrucao, sizeof(instrucao), "    call %s", p);
            adiciona_codigo(funcao, instrucao);
        }
    }
}

int main() {
    char nome_entrada[128], nome_saida[128];
    printf("Nome do ficheiro C: ");
    fgets(nome_entrada, 128, stdin);
    nome_entrada[strcspn(nome_entrada, "\n")] = 0;

    FILE* in = fopen(nome_entrada, "r");
    if (!in) {
        perror("Erro ao abrir");
        return 1;
    }

    snprintf(nome_saida, sizeof(nome_saida), "%s", nome_entrada);
    char* ponto = strrchr(nome_saida, '.');
    if (ponto) strcpy(ponto, ".S");
    else strcat(nome_saida, ".S");

    char linha[256];
    char contexto[MAX_NOME] = "global";
    char var_nome[64];
    char globais[128][64];
    int total_globais = 0;

    while (fgets(linha, sizeof(linha), in)) {
        if (eh_declaracao_global(linha, var_nome)) {
            strcpy(globais[total_globais++], var_nome);
            continue;
        }

        if (strstr(linha, "void ") || strstr(linha, "int ")) {
            if (strchr(linha, '(') && strchr(linha, ')') && strchr(linha, '{')) {
                sscanf(linha, "%*s %[^ (]", contexto);
                continue;
            }
        }

        if (strchr(linha, '{') || strchr(linha, '}')) continue;
        processa_linha(contexto, linha);
    }

    fclose(in);

    FILE* out = fopen(nome_saida, "w");
    if (!out) {
        perror("Erro ao criar saída");
        return 1;
    }

    fprintf(out, "[global main]\n\nsection .data\n");
    for (int i = 0; i < total_globais; i++) {
        fprintf(out, "%s dw 0\n", globais[i]);
    }

    fprintf(out, "\nsection .text\n");
    for (int i = 0; i < total_funcoes; i++) {
        fprintf(out, "%s:\n", funcoes[i].nome);
        for (int j = 0; j < funcoes[i].total; j++) {
            fprintf(out, "%s\n", funcoes[i].linhas[j]);
        }
        fprintf(out, "\n");
    }

    fclose(out);
    printf("Compilado para: %s\n", nome_saida);
    return 0;
}
