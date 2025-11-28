/* Saulo Canto 2320940 3WB */
/* Hugo Freires 2321223 3WA */

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
            char var_cond[10];
            char var_ret[10];
            
            // Lê: zret condicao valor_retorno
            if (sscanf(linha, "zret %s %s", var_cond, var_ret) == 2) {
                printf("DEBUG: zret %s %s\n", var_cond, var_ret);

                // Carrega o valor da condição (var_cond) para %ecx
                if (strcmp(var_cond, "p0") == 0) {
                    code[code_idx++] = 0x8b; code[code_idx++] = 0xcf; // mov %edi, %ecx
                } else if (var_cond[0] == '$') {
                    int val = atoi(&var_cond[1]);
                    code[code_idx++] = 0xb9; // mov $val, %ecx
                    code[code_idx++] = val & 0xFF;
                    code[code_idx++] = (val >> 8) & 0xFF;
                    code[code_idx++] = (val >> 16) & 0xFF;
                    code[code_idx++] = (val >> 24) & 0xFF;
                } else { // é variável local vX
                    int var_num = var_cond[1] - '0';
                    int offset = 8 * (var_num + 1);
                    code[code_idx++] = 0x8b; code[code_idx++] = 0x4d; // mov -off(%rbp), %ecx
                    code[code_idx++] = (unsigned char)(256 - offset);
                }

                // cmpl $0, %ecx -> 83 f9 00
                code[code_idx++] = 0x83;
                code[code_idx++] = 0xf9;
                code[code_idx++] = 0x00;

                // Vamos pular o código de retorno. Precisamos saber quantos bytes pular.
                // O código de retorno vai ter: MOVs (var_ret->eax) + LEAVE + RET.
                // Deixar um placeholder para o offset do JNE e preencher depois.
                code[code_idx++] = 0x75; // JNE (short jump)
                int idx_do_pulo = code_idx; // Guarda onde escrever o tamanho do pulo
                code[code_idx++] = 0x00; // Placeholder

                // Move var_ret para %eax
                if (strcmp(var_ret, "p0") == 0) {
                    code[code_idx++] = 0x89; code[code_idx++] = 0xf8; // mov %edi, %eax
                } else if (var_ret[0] == '$') {
                    int val = atoi(&var_ret[1]);
                    code[code_idx++] = 0xb8; // mov $val, %eax
                    code[code_idx++] = val & 0xFF;
                    code[code_idx++] = (val >> 8) & 0xFF;
                    code[code_idx++] = (val >> 16) & 0xFF;
                    code[code_idx++] = (val >> 24) & 0xFF;
                } else { // vX
                    int var_num = var_ret[1] - '0';
                    int offset = 8 * (var_num + 1);
                    code[code_idx++] = 0x8b; code[code_idx++] = 0x45; // mov -off(%rbp), %eax
                    code[code_idx++] = (unsigned char)(256 - offset);
                }

                // Epílogo manual (leave + ret)
                code[code_idx++] = 0xc9; // leave
                code[code_idx++] = 0xc3; // ret

                // O pulo deve ser: (indice atual) - (indice logo após a instrução JNE)
                // indice logo após JNE = idx_do_pulo + 1
                code[idx_do_pulo] = code_idx - (idx_do_pulo + 1);
            }
        }
        else if (cmd == 'v') { 
            int var_dest;
            char token1[20], token2[20], token3[20]; // Strings para ler os pedaços
            
            // Lê o formato genérico: v0 = token1 token2 token3
            // Exemplo Soma: v0 = v1 + v2   -> token1="v1", token2="+", token3="v2"
            // Exemplo Call: v0 = call 1 p0 -> token1="call", token2="1", token3="p0"
            
            if (sscanf(linha, "v%d = %s %s %s", &var_dest, token1, token2, token3) == 4) {
                
                // ==========================================================
                // CASO 1: É UMA CHAMADA DE FUNÇÃO (CALL)
                // ==========================================================
                if (strcmp(token1, "call") == 0) {
                    int id_func = atoi(token2); // Qual função chamar (0, 1, 2...)
                    char *arg = token3;         // O argumento (p0, v0, $10...)

                    printf("DEBUG: CALL detectado. Func: %d, Arg: %s\n", id_func, arg);

                    if (arg[0] == '$') { // Constante
                        int val = atoi(&arg[1]);
                        code[code_idx++] = 0xbf; // mov $val, %edi
                        code[code_idx++] = val & 0xFF;
                        code[code_idx++] = (val >> 8) & 0xFF;
                        code[code_idx++] = (val >> 16) & 0xFF;
                        code[code_idx++] = (val >> 24) & 0xFF;
                    } 
                    else if (arg[0] == 'v') { // Variável Local
                        int vn = arg[1] - '0';
                        int offset = 8 * (vn + 1);
                        // mov -offset(%rbp), %edi
                        code[code_idx++] = 0x8b; 
                        code[code_idx++] = 0x7d; 
                        code[code_idx++] = (unsigned char)(256 - offset);
                    }

                    // 2. Gerar a instrução CALL
                    // Opcode E8 (Call Relative)
                    int end_instrucao_call = code_idx; 
                    int target_addr = endereco_funcs[id_func]; // Endereço onde a função alvo começa
                    
                    // Offset = Destino - (Endereço_Onde_Estou + 5 bytes da instrução)
                    int call_offset = target_addr - (end_instrucao_call + 5);

                    code[code_idx++] = 0xe8;
                    code[code_idx++] = call_offset & 0xFF;
                    code[code_idx++] = (call_offset >> 8) & 0xFF;
                    code[code_idx++] = (call_offset >> 16) & 0xFF;
                    code[code_idx++] = (call_offset >> 24) & 0xFF;

                    // 3. Pegar o retorno (%eax) e salvar na variável destino
                    // mov %eax, -offset_destino(%rbp)
                    int off_dest = 8 * (var_dest + 1);
                    code[code_idx++] = 0x89; 
                    code[code_idx++] = 0x45; 
                    code[code_idx++] = (unsigned char)(256 - off_dest);
                }
                
                // ==========================================================
                // CASO 2: É UMA OPERAÇÃO ARITMÉTICA (CÓDIGO ANTIGO ADAPTADO)
                // ==========================================================
                else {
                    // token1 é o operando 1 (ex: "v1")
                    // token2 é o operador (ex: "+")
                    // token3 é o operando 2 (ex: "v2")
                    
                    char *op1 = token1;
                    char operador = token2[0];
                    char *op2 = token3;
                    
                    // 1. Carregar op1 em %ecx
                    if (strcmp(op1, "p0") == 0) {
                        code[code_idx++] = 0x89; code[code_idx++] = 0xf9; // mov %edi, %ecx
                    } else if (op1[0] == 'v') {
                        int vn = op1[1] - '0';
                        int off = 8 * (vn + 1);
                        code[code_idx++] = 0x8b; code[code_idx++] = 0x4d; code[code_idx++] = 256-off;
                    } else if (op1[0] == '$') {
                        int val = atoi(&op1[1]);
                        code[code_idx++] = 0xb9; 
                        code[code_idx++] = val & 0xFF; code[code_idx++] = (val>>8)&0xFF; code[code_idx++] = (val>>16)&0xFF; code[code_idx++] = (val>>24)&0xFF;
                    }

                    // 2. Operar com op2
                    // Precisamos carregar op2 ou usar imediato
                    if (op2[0] == '$') {
                        int val = atoi(&op2[1]);
                        if (operador == '+') {
                            if (val >= -128 && val <= 127) { code[code_idx++] = 0x83; code[code_idx++] = 0xc1; code[code_idx++] = val&0xFF; }
                            else { code[code_idx++] = 0x81; code[code_idx++] = 0xc1; code[code_idx++] = val&0xFF; code[code_idx++] = (val>>8)&0xFF; code[code_idx++] = (val>>16)&0xFF; code[code_idx++] = (val>>24)&0xFF; }
                        } else if (operador == '-') {
                            if (val >= -128 && val <= 127) { code[code_idx++] = 0x83; code[code_idx++] = 0xe9; code[code_idx++] = val&0xFF; }
                            else { code[code_idx++] = 0x81; code[code_idx++] = 0xe9; code[code_idx++] = val&0xFF; code[code_idx++] = (val>>8)&0xFF; code[code_idx++] = (val>>16)&0xFF; code[code_idx++] = (val>>24)&0xFF; }
                        } else if (operador == '*') {
                            code[code_idx++] = 0x69; code[code_idx++] = 0xc9; 
                            code[code_idx++] = val&0xFF; code[code_idx++] = (val>>8)&0xFF; code[code_idx++] = (val>>16)&0xFF; code[code_idx++] = (val>>24)&0xFF;
                        }
                    } else {
                        // op2 é variável ou p0. Carrega em %edx
                        if (strcmp(op2, "p0") == 0) { code[code_idx++] = 0x89; code[code_idx++] = 0xfa; }
                        else if (op2[0] == 'v') {
                            int vn = op2[1] - '0'; int off = 8*(vn+1);
                            code[code_idx++] = 0x8b; code[code_idx++] = 0x55; code[code_idx++] = 256-off;
                        }
                        
                        if (operador == '+') { code[code_idx++] = 0x01; code[code_idx++] = 0xd1; } // add %edx, %ecx
                        else if (operador == '-') { code[code_idx++] = 0x29; code[code_idx++] = 0xd1; } // sub %edx, %ecx
                        else if (operador == '*') { code[code_idx++] = 0x0f; code[code_idx++] = 0xaf; code[code_idx++] = 0xca; } // imul %edx, %ecx
                    }

                    // 3. Salvar resultado (que está em %ecx) na variavel destino
                    int off_dest = 8 * (var_dest + 1);
                    code[code_idx++] = 0x89; code[code_idx++] = 0x4d; code[code_idx++] = 256-off_dest;
                }
            }
        }
    }
    
    if (num_funcs > 0) {
        *entry = (funcp)&code[endereco_funcs[num_funcs - 1]];
        printf("DEBUG: Entry point definido para função %d no endereço %p\n", num_funcs - 1, (void*)*entry);
    } else {
        *entry = NULL;
    }

}
