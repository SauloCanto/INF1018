/* Saulo Canto 2320940 3WB */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gera_codigo.h"

static void gera_prologo(unsigned char code[], int *code_idx) {
    code[(*code_idx)++] = 0x55;        // push %rbp
    code[(*code_idx)++] = 0x48;        // mov %rsp,%rbp
    code[(*code_idx)++] = 0x89;
    code[(*code_idx)++] = 0xe5;
    code[(*code_idx)++] = 0x48;        // subq $32, %rsp
    code[(*code_idx)++] = 0x83;
    code[(*code_idx)++] = 0xec;
    code[(*code_idx)++] = 0x20;
}

static void gera_epilogo(unsigned char code[], int *code_idx) {
    code[(*code_idx)++] = 0x48;        // addq $32, %rsp
    code[(*code_idx)++] = 0x83;
    code[(*code_idx)++] = 0xc4;
    code[(*code_idx)++] = 0x20;
    code[(*code_idx)++] = 0xc9;        // leave
    code[(*code_idx)++] = 0xc3;        // ret
}

void gera_codigo (FILE *f, unsigned char code[], funcp *entry){
    char linha[100];
    int code_idx = 0;
    int num_funcs = 0;
    int endereco_funcs[10];
    
    while (fgets(linha, sizeof(linha), f) != NULL) {
        linha[strcspn(linha, "\n")] = 0; // Remove /n da linha lida
        
        if (strlen(linha) == 0) continue; // Segue se a linha está vazia
        
        printf("DEBUG: Linha lida: '%s'\n", linha);
        
        char cmd = linha[0];
        
        if (cmd == 'f') {  // function
            printf("DEBUG: Início da função %d no índice %d\n", num_funcs, code_idx);
            endereco_funcs[num_funcs] = code_idx;
            gera_prologo(code, &code_idx);
            num_funcs++;
        }
        else if (cmd == 'e') {  // end
            printf("DEBUG: Fim de função\n");
            gera_epilogo(code, &code_idx);
        }
        else if (cmd == 'r') {  // ret
            int valor;
            char var[10];
    
            if (sscanf(linha, "ret $%d", &valor) == 1) { // retorna constante
                printf("DEBUG: ret $%d (constante)\n", valor);
                code[code_idx++] = 0xb8;    // mov $valor, %eax
                code[code_idx++] = (valor & 0xFF);
                code[code_idx++] = ((valor >> 8) & 0xFF);
                code[code_idx++] = ((valor >> 16) & 0xFF);
                code[code_idx++] = ((valor >> 24) & 0xFF);
            }
            else if (sscanf(linha, "ret %s", var) == 1) { // retorna variável/parâmetro
                printf("DEBUG: ret %s\n", var);
                if (strcmp(var, "p0") == 0) {
                    code[code_idx++] = 0x89;    //movl %edi, %eax
                    code[code_idx++] = 0xf8;
                    printf("DEBUG: ret p0 (movl %%edi, %%eax)\n");
                } else {
                    int var_num = var[1] - '0';  // pega o número de v'x'
                    int offset = 8 * (var_num + 1);  // v0=-8, v1=-16, v2=-24, v3=-32, v4=-40
                    unsigned char off_byte = 256 - offset;  // complemento a 2
                    
                    code[code_idx++] = 0x8b;    // movl
                    code[code_idx++] = 0x45;    // (%rbp)
                    code[code_idx++] = off_byte; // -offset(%rbp)
                    printf("DEBUG: ret %s (movl -%d(%%rbp), %%eax)\n", var, offset);
                }
            }
        }
        else if (cmd == 'z') {  // zret
            printf("DEBUG: zret detectado\n");
        }
        else if (cmd == 'v') {  // atribuição (v0 = ...)
            printf("DEBUG: atribuição detectada\n");
            
            int var_dest;
            char op1[10], op2[10];
            char operador;
            
            if (sscanf(linha, "v%d = %s %c %s", &var_dest, op1, &operador, op2) == 4) {
                printf("DEBUG: v%d = %s %c %s\n", var_dest, op1, operador, op2);
                
                if (strcmp(op1, "p0") == 0) {
                    code[code_idx++] = 0x89;     // movl %edi, %ecx
                    code[code_idx++] = 0xf9;
                } else if (op1[0] == 'v') {
                    int var_num = op1[1] - '0'; 
                    int offset = 8 * (var_num + 1);
                    unsigned char off_byte = 256 - offset;
                    code[code_idx++] = 0x8b;    // movl -offset(%rbp), %ecx
                    code[code_idx++] = 0x4d;
                    code[code_idx++] = off_byte;
                } else if (op1[0] == '$') {
                    int const_val = atoi(&op1[1]);
                    code[code_idx++] = 0xb9;    // movl $const, %ecx
                    code[code_idx++] = (const_val & 0xFF);
                    code[code_idx++] = ((const_val >> 8) & 0xFF);
                    code[code_idx++] = ((const_val >> 16) & 0xFF);
                    code[code_idx++] = ((const_val >> 24) & 0xFF);
                }
                
                if (op2[0] == '$') {
                    int const_val = atoi(&op2[1]);
                    
                    if (operador == '+') {
                        // addl $const, %ecx
                        if (const_val >= -128 && const_val <= 127) {
                            code[code_idx++] = 0x83;
                            code[code_idx++] = 0xc1;
                            code[code_idx++] = (const_val & 0xFF);
                        } else {
                            code[code_idx++] = 0x81;
                            code[code_idx++] = 0xc1;
                            code[code_idx++] = (const_val & 0xFF);
                            code[code_idx++] = ((const_val >> 8) & 0xFF);
                            code[code_idx++] = ((const_val >> 16) & 0xFF);
                            code[code_idx++] = ((const_val >> 24) & 0xFF);
                        }
                    } else if (operador == '-') {
                        // subl $const, %ecx
                        if (const_val >= -128 && const_val <= 127) {
                            code[code_idx++] = 0x83;
                            code[code_idx++] = 0xe9;
                            code[code_idx++] = (const_val & 0xFF);
                        } else {
                            code[code_idx++] = 0x81;
                            code[code_idx++] = 0xe9;
                            code[code_idx++] = (const_val & 0xFF);
                            code[code_idx++] = ((const_val >> 8) & 0xFF);
                            code[code_idx++] = ((const_val >> 16) & 0xFF);
                            code[code_idx++] = ((const_val >> 24) & 0xFF);
                        }
                    } else if (operador == '*') {
                        // imull $const, %ecx, %ecx
                        code[code_idx++] = 0x69;
                        code[code_idx++] = 0xc9;
                        code[code_idx++] = (const_val & 0xFF);
                        code[code_idx++] = ((const_val >> 8) & 0xFF);
                        code[code_idx++] = ((const_val >> 16) & 0xFF);
                        code[code_idx++] = ((const_val >> 24) & 0xFF);
                    }
                } else {
                    // Operação com registrador - carregar operando2 em %edx primeiro
                    if (strcmp(op2, "p0") == 0) {
                        // movl %edi, %edx
                        code[code_idx++] = 0x89;
                        code[code_idx++] = 0xfa;
                    } else if (op2[0] == 'v') {
                        // movl -offset(%rbp), %edx
                        int var_num = op2[1] - '0';
                        int offset = 8 * (var_num + 1);
                        unsigned char off_byte = 256 - offset;
                        code[code_idx++] = 0x8b;
                        code[code_idx++] = 0x55;
                        code[code_idx++] = off_byte;
                    }
                    
                    // Aplicar operação
                    if (operador == '+') {
                        // addl %edx, %ecx
                        code[code_idx++] = 0x01;
                        code[code_idx++] = 0xd1;
                    } else if (operador == '-') {
                        // subl %edx, %ecx
                        code[code_idx++] = 0x29;
                        code[code_idx++] = 0xd1;
                    } else if (operador == '*') {
                        // imull %edx, %ecx
                        code[code_idx++] = 0x0f;
                        code[code_idx++] = 0xaf;
                        code[code_idx++] = 0xca;
                    }
                }
                
                // 3. Salvar resultado em var_dest
                int offset = 8 * (var_dest + 1);
                unsigned char off_byte = 256 - offset;
                // movl %ecx, -offset(%rbp)
                code[code_idx++] = 0x89;
                code[code_idx++] = 0x4d;
                code[code_idx++] = off_byte;
                
                printf("DEBUG: Atribuição v%d gerada\n", var_dest);
            }

        }
        else {
            printf("DEBUG: Comando desconhecido: %s\n", linha);
        }
    }
    
    if (num_funcs > 0) {
        *entry = (funcp)&code[endereco_funcs[num_funcs - 1]];
        printf("DEBUG: Entry point definido para função %d no endereço %p\n", num_funcs - 1, (void*)*entry);
    } else {
        *entry = NULL;
    }

}
