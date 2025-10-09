  /* Hugo Freires 2321223 3WA */
  /* Saulo Canto 2320940 3WB */

#include "bigint.h"
#include <stdio.h>
#include "string.h"

// res = val (extensão de sinal para 128 bits)
void big_val (BigInt res, long val){

    for (int i = 0; i < (int)sizeof(BigInt); i++) { //zera os 16 bytes do resultado (evita lixo nos bytes altos) */
        res[i] = 0;
    }

    int neg = (val < 0);    // 1 se o número é negativo, 0 se é positivo

    unsigned long temp = (unsigned long)val; // copia o valor passado como parâmetro para temp
    unsigned char parte = 0;                  // guarda o byte menos significativo

    for (int i = 0; i < (int)sizeof(long); i++) {   //salva os bytes no modelo little-endian
        parte = (unsigned char)(temp & 0xFF);  // pega o byte menos significativo
        res[i] = parte;                        // grava o byte menos significativo no BigInt
        temp = temp >> 8;                      // avança para o próximo byte
    }

    if (neg == 0){  //se for positivo, a extensão de sinal é 0x00, que acontece quando o vetor resultado é zerado no inicio do código
        return;
    }

    for (int i = (int)sizeof(long); i < (int)sizeof(BigInt); i++) { //se negativo, extensão de sinal com 0xFF nos bytes mais significativos 
        res[i] = 0xFF;
    }
}

// res = -a  (complemento de 2: ~a + 1)
void big_comp2(BigInt res, BigInt a){

    unsigned int carry = 1; // inicia em 1 por causa do "+1" do complemento de 2

    for (int i = 0; i < (int)sizeof(BigInt); i++) {
        unsigned int inv = (unsigned int)(~a[i]) & 0xFF;  // inverte os bits do byte atual
        unsigned int soma = inv + carry;                  // soma +1 (e possíveis vai-uns)
        res[i] = (unsigned char)(soma & 0xFF);            // guarda só o byte atual
        carry = soma >> 8;                                // carry (vai-um) para o próximo byte o que não foi salvo
    }
}

// res = a + b
void big_sum (BigInt res, BigInt a, BigInt b) {

    unsigned int carry = 0; 

    for (int i = 0; i < (int)sizeof(BigInt); i++) {
        unsigned int soma = (unsigned int)a[i] + (unsigned int)b[i] + carry; // soma byte a byte + carry anterior
        res[i] = (unsigned char)(soma & 0xFF); // guarda só o byte menos significativo da soma
        carry = soma >> 8;                     // transbordo da soma vira carry pro próximo byte
    }
}

// res = a - b
void big_sub (BigInt res, BigInt a, BigInt b) {

    unsigned int prox = 0; //sinalizar "pegar emprestado" do próximo byte

    for (int i = 0; i < (int)sizeof(BigInt); i++) {
        int sub = (int)a[i] - (int)b[i] - (int)prox;

        if (sub < 0) {
            sub += 256; // empresta 1 byte (2^8)
            prox = 1;   // sinaliza borrow pro próximo
        } else {
            prox = 0;
        }

        res[i] = (unsigned char)(sub & 0xFF);
    }
}

// res = a * b
void big_mul (BigInt res, BigInt a, BigInt b) {
    BigInt acc; // acumulador do resultado parcial
    for (int i = 0; i < (int)sizeof(BigInt); i++) acc[i] = 0; 

    BigInt sh;  // "versão deslocada" de a, que anda 1 bit a cada passo
    memcpy(sh, a, sizeof(BigInt));

    // percorre cada um dos 128 bits de b (varrendo por bytes, e dentro de cada byte, por bits)
    for (int byte = 0; byte < (int)sizeof(BigInt); byte++) {
        unsigned char bj = b[byte]; // byte atual de b (8 bits)
        for (int bit = 0; bit < 8; bit++) {
            if (bj & 1) {          // se o bit atual de b é 1
                BigInt temp;       // soma o parcial: acc += sh
                big_sum(temp, acc, sh);
                memcpy(acc, temp, sizeof(BigInt));
            }
            BigInt temp2;
            big_shl(temp2, sh, 1); // sh <<= 1 (prepara para o próximo bit)
            memcpy(sh, temp2, sizeof(BigInt));
            bj >>= 1;              // avança para o próximo bit do byte bj
        }
    }

    memcpy(res, acc, sizeof(BigInt)); // resultado final
}

// res = a << n
void big_shl (BigInt res, BigInt a, int n) {
    if (n <= 0) {   //caso base, se n<0, não há deslocamento, copia 'a' para 'res'*/
        if (res != a) memcpy(res, a, sizeof(BigInt));
        return;
    }
    if (n >= 8 * (int)sizeof(BigInt)) { // se deslocamento é maior que o numero de bits, o número é todo zerado
        for (int i = 0; i < (int)sizeof(BigInt); i++) res[i] = 0;
        return;
    }

    int byte_shift = n / 8; //deslocamento de bytes
    int bit_shift  = n % 8; //deslocamento de bits

    BigInt tmp;

    for (int i = 0; i < (int)sizeof(BigInt); i++) {
        int src = i - byte_shift;
        tmp[i] = (src >= 0 && src < (int)sizeof(BigInt)) ? a[src] : 0x00; //tmp[i] recebe a[i - byte_shift], mas se a[src] esta fora do range, recebe 0
    }

    if (bit_shift != 0) {
        unsigned int carry = 0;
        for (int i = 0; i < (int)sizeof(BigInt); i++) {
            unsigned int v = ((unsigned int)tmp[i] << bit_shift) | carry; //desloca temp[i] o número necessários de bits
            tmp[i] = (unsigned char)(v & 0xFF); //pega apeans o byte menos significativo
            carry  = v >> 8;  //transborda para o próximo byte
        }
    }

    memcpy(res, tmp, sizeof(BigInt)); //copia 'temp' para 'res'
}


// res = a >> n
void big_shr (BigInt res, BigInt a, int n) {
    if (n <= 0) {
        memcpy(res, a, sizeof(BigInt));
        return;
    }
    if (n >= 8 * (int)sizeof(BigInt)) {
        for (int i = 0; i < (int)sizeof(BigInt); i++) res[i] = 0;
        return;
    }

    int byte_shift = n / 8;   // quantos bytes inteiros mover 
    int bit_shift  = n % 8;   // quantos bits dentro do byte 

    BigInt tmp;

    for (int i = 0; i < (int)sizeof(BigInt); i++) { // desloca por BYTES para a direita: byte i de tmp vem do byte (i + byte_shift) de a se estourar, entra 0 (lógico) 
        int src = i + byte_shift;
        tmp[i] = (src < (int)sizeof(BigInt)) ? a[src] : 0x00;
    }

    if (bit_shift != 0) { // desloca por BITS dentro dos bytes: varre do topo (MSB) para a base (LSB), carregando para o byte menor os bits que cairam do byte maior
        unsigned int carry = 0; // bits baixos do byte anterior
        for (int i = (int)sizeof(BigInt) - 1; i >= 0; i--) {
            unsigned int v = ((unsigned int)tmp[i] >> bit_shift) | (carry << (8 - bit_shift));
            carry = tmp[i] & ((1u << bit_shift) - 1u); // guarda os bits que sobraram 
            tmp[i] = (unsigned char)(v & 0xFF);
        }
    }

    memcpy(res, tmp, sizeof(BigInt));
}


// res = a >> n
void big_sar (BigInt res, BigInt a, int n) {
    // logica geral muito similar a de big_shr
    if (n <= 0) {
        memcpy(res, a, sizeof(BigInt));
        return;
    }

    unsigned char sign = (a[(int)sizeof(BigInt) - 1] & 0x80) ? 0xFF : 0x00; // descobre o bit de sinal original

    if (n >= 8 * (int)sizeof(BigInt)) {
        for (int i = 0; i < (int)sizeof(BigInt); i++) res[i] = sign;
        return;
    }

    int byte_shift = n / 8;
    int bit_shift  = n % 8;

    BigInt tmp;

    for (int i = 0; i < (int)sizeof(BigInt); i++) {
        int src = i + byte_shift;
        tmp[i] = (src < (int)sizeof(BigInt)) ? a[src] : sign;
    }

    if (bit_shift != 0) {
        unsigned int carry = (sign == 0xFF) ? ((1u << bit_shift) - 1u) : 0u;
        for (int i = (int)sizeof(BigInt) - 1; i >= 0; i--) {
            unsigned int v = ((unsigned int)tmp[i] >> bit_shift) | (carry << (8 - bit_shift));
            carry = tmp[i] & ((1u << bit_shift) - 1u);
            tmp[i] = (unsigned char)(v & 0xFF);
        }
    }

    memcpy(res, tmp, sizeof(BigInt));
}

