#include <stdio.h>
#include <stdlib.h>
#include "gera_codigo.h"

#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>

#define PAGE_START(P) ((intptr_t)(P)&~(pagesize-1))
#define PAGE_END(P) (((intptr_t)(P)+pagesize-1)&~(pagesize-1))

int execpage(void *ptr, size_t len) {
	int ret;

	const long pagesize = sysconf(_SC_PAGE_SIZE);
	if (pagesize == -1)
		return -1;

	ret = mprotect((void *)PAGE_START(ptr),
		 PAGE_END((intptr_t)ptr + len) - PAGE_START(ptr),
		 PROT_READ | PROT_WRITE | PROT_EXEC);
	if (ret == -1)
		return -1;

	return 0;
}

#undef PAGE_START
#undef PAGE_END

int main(int argc, char *argv[]) {
    FILE *fp;
    funcp funcLBS;
    unsigned char code[300];
    int res;

    execpage(code, sizeof(code));

    if (argc < 2) {
        printf("Uso: %s <arquivo.txt> [argumento]\n", argv[0]);
        return 1;
    }

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

    int arg = 5;
    if (argc >= 3) {
        arg = atoi(argv[2]);
    }
    
    printf("\nChamando função com argumento %d...\n", arg);
    res = (*funcLBS)(arg);
    printf("Resultado: %d\n", res);
    
    return 0;
}
