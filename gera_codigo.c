/* Saulo Canto 2320940 3WB */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gera_codigo.h"

void gera_codigo (FILE *f, unsigned char code[], funcp *entry){
    char linha[100];
    int code_idx = 0;
    int num_funcs = 0;
    int endereco_funcs[10];
    
    while (fgets(linha, sizeof(linha), f) != NULL) {
        linha[strcspn(linha, "\n")] = 0; // Remove /n da linha lida
        
        if (strlen(linha) == 0) continue; // Segue se a linha nao está vazia
        
        printf("DEBUG: Linha lida: '%s'\n", linha);
        
        char cmd = linha[0];
        
        if (cmd == 'f') {  // function
            printf("DEBUG: Início da função %d no índice %d\n", num_funcs, code_idx);
            endereco_funcs[num_funcs] = code_idx;
            num_funcs++;
        }
        else if (cmd == 'e') {  // end
            printf("DEBUG: Fim de função\n");
        }
        else if (cmd == 'r') {  // ret
            int valor;
            char var[10];
    
            if (sscanf(linha, "ret $%d", &valor) == 1) { // retorna constante
                printf("DEBUG: ret $%d (constante)\n", valor);
                code[code_idx++] = 0xb8;
                code[code_idx++] = (valor & 0xFF);
                code[code_idx++] = ((valor >> 8) & 0xFF);
                code[code_idx++] = ((valor >> 16) & 0xFF);
                code[code_idx++] = ((valor >> 24) & 0xFF);
                code[code_idx++] = 0xc3;
            }
            else if (sscanf(linha, "ret %s", var) == 1) { //retorna 
                printf("DEBUG: ret %s (variável/parâmetro)\n", var);
            }
        }
        else if (cmd == 'z') {  // zret
            printf("DEBUG: zret detectado\n");
        }
        else if (cmd == 'v') {  // atribuição (v0 = ...)
            printf("DEBUG: atribuição detectada\n");
        }
        else {
            printf("DEBUG: Comando desconhecido: %s\n", linha);
        }
    }
    
    if (num_funcs > 0) {
        *entry = (funcp)(&code[endereco_funcs[num_funcs - 1]]);
        printf("DEBUG: Entry point definido para função %d no endereço %p\n", num_funcs - 1, (void*)*entry);
    } else {
        *entry = NULL;
    }

}

int main(int argc, char *argv[]) {
    FILE *fp;
    funcp funcLBS;
    unsigned char code[300];
    int res;

    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Erro ao abrir arquivo %s\n", argv[1]);
        return 1;
    }
    
    printf("Gerando código a partir de %s...\n", argv[1]);
    
    gera_codigo(fp, code, &funcLBS);
    
    fclose(fp);
    
    if (funcLBS == NULL) {
        printf("Erro na geração de código\n");
        return 1;
    }
    
    printf("\nChamando função com argumento 5...\n");
    res = (*funcLBS)(5);
    printf("Resultado: %d\n", res);
    
    return 0;
}