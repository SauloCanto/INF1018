  /* Hugo Freires 2321223 3WA */
  /* Saulo Canto 2320940 3WB */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "bigint.h"

/* ==== utilitários de teste ==== */

// imprime um BigInt como bytes little-endian
static void dump_hex(const char *tag, BigInt x) {
    printf("%s = {", tag);
    for (int i = 0; i < (int)sizeof(BigInt); i++) {
        printf("0x%02X%s", x[i], (i == (int)sizeof(BigInt)-1) ? "" : ",");
    }
    printf("}\n");
}

// compara dois BigInt; em caso de erro, mostra o diff e aborta
static void expect_equal(const char *msg, BigInt got, BigInt exp) {
    if (memcmp(got, exp, sizeof(BigInt)) != 0) {
        printf("FAIL: %s\n", msg);
        dump_hex(" got", got);
        dump_hex(" exp", exp);
        assert(!"BigInt mismatch");
    } else {
        printf("OK  : %s\n", msg);
    }
}

// cria BigInt a partir de long usando big_val
static void from_long(BigInt out, long v) {
    big_val(out, v);
}

/* ==== testes unitários por função ==== */

static void test_big_val(void) {
    BigInt x, e;

    from_long(e, 0);
    big_val(x, 0);
    expect_equal("big_val(0)", x, e);

    // exemplo do enunciado: 1
    {
        unsigned char e1[16] = {0x01,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
        big_val(x, 1);
        memcpy(e, e1, 16);
        expect_equal("big_val(1)", x, e);
    }

    // exemplo do enunciado: -2
    {
        unsigned char em2[16] = {0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
        big_val(x, -2);
        memcpy(e, em2, 16);
        expect_equal("big_val(-2)", x, e);
    }

    // -1: todos 0xFF
    big_val(x, -1);
    for (int i=0;i<16;i++){
        e[i]=0xFF;
    }
    expect_equal("big_val(-1)", x, e);
}

static void test_big_comp2(void) {
    BigInt a, r, e;

    //positivo para negativo
    from_long(a, 6);
    from_long(e, -6);
    big_comp2(r, a);
    expect_equal("comp2(6) == -6", r, e);

    //negativo para positivo
    from_long(a, -6);
    from_long(e, 6);
    big_comp2(r, a);
    expect_equal("comp2(-6) == 6", r, e);

    //zero
    from_long(a, 0);
    from_long(e, 0);
    big_comp2(r, a);
    expect_equal("comp2(0) == 0", r, e);

    // teste -(-x) == x para alguns valores aleatórios
    long samples[] = {1, -1, 12345, -99999, 0x7FFFFFFF, (long)0x8000000000000000};
    for (int k=0;k<(int)(sizeof(samples)/sizeof(samples[0]));k++){
        from_long(a, samples[k]);
        big_comp2(r, a);
        big_comp2(r, r);
        from_long(e, samples[k]);
        expect_equal("comp2 twice == identity", r, e);
    }
}

static void test_big_sum(void) {
    BigInt a,b,r,e;

    //soma de positivos
    from_long(a, 20);
    from_long(b, 22);
    from_long(e, 42);
    big_sum(r, a, b);
    expect_equal("sum(20,22)==42", r, e);

    //soma em outra ordem
    big_sum(r, b, a);
    expect_equal("sum(22,20)==42", r, e);

    //soma de números negativos
    from_long(a, -7);
    from_long(b, -8);
    from_long(e, -15);
    big_sum(r, a, b);
    expect_equal("sum(-7,-8)==-15", r, e);

    //soma de números negativos e positivos
    from_long(a, -5);
    from_long(b, 12);
    from_long(e, 7);
    big_sum(r, a, b);
    expect_equal("sum(-5,12)==7", r, e);
    big_sum(r, b, a);
    expect_equal("sum(12,-5)==7", r, e);

}

static void test_big_sub(void) {
    BigInt a,b,r,e,zero={0};

    //subtração de um número positivo resultando em zero
    from_long(a, 123456);
    big_sub(r, a, a);
    expect_equal("sub(a,a)==0", r, zero);

    // sum como a + (-b) deve bater com big_sub
    from_long(a, 7777);
    from_long(b, -999);
    BigInt nb, sum_res;
    big_comp2(nb, b);
    big_sum(sum_res, a, nb);
    big_sub(r, a, b);
    expect_equal("sub vs comp2+sum", r, sum_res);

    //subtração de um número positivo
    from_long(a, 20);
    from_long(b, 5);
    from_long(e, 15);
    big_sub(r, a, b);
    expect_equal("sub(20,5)==15", r, e);

    //subtração de numeros negativos
    from_long(a, -5);
    from_long(b, -7);
    from_long(e, 2);
    big_sub(r, a, b);
    expect_equal("sub(-5,-7)==2", r, e);

    //subtração de numeros negativos por numeros positivos
    from_long(a, -3);
    from_long(b, 6);
    from_long(e, -9);
    big_sub(r, a, b);
    expect_equal("sub(-3,6)==-9", r, e);

    //subtração em ordem diferente
    from_long(a, 10);
    from_long(b, 3);
    from_long(e, 7);
    big_sub(r, a, b);
    expect_equal("sub(10,3)==7", r, e);
    from_long(e, -7);
    big_sub(r, b, a);
    expect_equal("sub(3,10)==-7", r, e);
}

static void test_shl(void) {
    BigInt a,r,e,zero={0};

    // Usar big_shl convencionalmente
    from_long(a, 1);
    big_shl(r, a, 8);
    for (int i=0; i<16; i++){ e[i] = 0; }
    e[1] = 0x01;
    expect_equal("shl(1,8)==0x0100", r, e);

    // Usar big_shl com parâmetros nulos
    from_long(a, 1);
    big_shl(r, a, 0);
    expect_equal("shl n=0 copia", r, a);

    // Usar big_shl com parâmetros maiores que o número de bits
    from_long(a, 1);
    big_shl(r, a, 200);
    expect_equal("shl n>=128 == 0", r, zero);
}

static void test_shr(void) {
    BigInt a,r,e,zero={0};

    // Usar big_shr convencionalmente
    from_long(a, 0x0100);
    big_shr(r, a, 4);
    for (int i=0; i<16; i++){ e[i] = 0; }
    e[0] = 0x10;
    expect_equal("shr(0x0100,4)==0x0010", r, e);

    // Usar big_shr com parâmetros nulos
    from_long(a, 0x1234);
    big_shr(r, a, 0);
    expect_equal("shr n=0 copia", r, a);

    // Usar big_shr com parâmetros maiores que o número de bits
    from_long(a, 0x1234);
    big_shr(r, a, 130);
    expect_equal("shr n>=128 == 0", r, zero);
}

static void test_sar(void) {
    BigInt a,r,e;

    // Usar big_sar convencionalmente
    from_long(a, -1);
    big_sar(r, a, 37);
    from_long(e, -1);
    expect_equal("sar(-1,37)==-1", r, e);

    // Usar big_sar para comparar comportamento entre shr e sar para negativos
    from_long(a, -256);
    big_shr(r, a, 4);
    BigInt sr; memcpy(sr, r, 16);
    big_sar(r, a, 4);
    BigInt ar; memcpy(ar, r, 16);
    if (memcmp(sr, ar, 16) == 0) {
        dump_hex("shr(-256,4)", sr);
        dump_hex("sar(-256,4)", ar);
        assert(!"shr vs sar (negativo) deveriam diferir");
    } else {
        printf("OK  : shr vs sar divergem para negativo (como esperado)\n");
    }

    // Usar big_sar com parâmetros maiores que o número de bits
    from_long(a, -42);
    big_sar(r, a, 200);
    for (int i=0;i<16;i++) e[i]=0xFF;
    expect_equal("sar n>=128 preenche com sinal", r, e);

    // Usar big_sar com parâmetros nulos
    from_long(a, -1234);
    big_sar(r, a, 0);
    expect_equal("sar n=0 copia", r, a);
}

static void test_mul(void) {
    BigInt a,b,r,e;

    // Usar big_mul para valores positivos
    from_long(a, 7); from_long(b, 6); from_long(e, 42);
    big_mul(r, a, b);
    expect_equal("mul(7,6)==42", r, e);

    // Usar big_mul para um valor positivo e um negativo
    from_long(a, -3); from_long(b, 11); from_long(e, -33);
    big_mul(r, a, b);
    expect_equal("mul(-3,11)==-33", r, e);

    // Usar big_mul para valores negativos
    from_long(a, -5); from_long(b, -7); from_long(e, 35);
    big_mul(r, a, b);
    expect_equal("mul(-5,-7)==35", r, e);

    // Usar big_mul para 0
    from_long(a, 0); from_long(b, 123456); from_long(e, 0);
    big_mul(r, a, b);
    expect_equal("mul(0,123456)==0", r, e);

    // Usar big_mul para 0 (ordem invertida)
    from_long(a, -999); from_long(b, 0); from_long(e, 0);
    big_mul(r, a, b);
    expect_equal("mul(-999,0)==0", r, e);
}

/* ==== MAIN: executa todos os testes ==== */
int main(void) {
    printf("=== Testes BigInt ===\n");
    test_big_val();
    test_big_comp2();
    test_big_sum();
    test_big_sub();
    test_mul();
    test_shl();
    test_shr();
    test_sar();
    printf("=== Todos os testes concluidos! ===\n");
    return 0;
}
