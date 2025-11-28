#!/bin/bash

echo "=== Testando todas as funções ==="
echo ""

echo "Função 0: ret \$100"
echo "  Esperado: 100"
./teste test_f0.txt 0 | grep "Resultado:"
echo ""

echo "Função 1: ret p0"
echo "  Com argumento 5 (esperado: 5):"
./teste test_f1.txt 5 | grep "Resultado:"
echo "  Com argumento 10 (esperado: 10):"
./teste test_f1.txt 10 | grep "Resultado:"
echo ""

echo "Função 2: v0 = p0 + \$1; ret v0"
echo "  Com argumento 5 (esperado: 6):"
./teste test_f2.txt 5 | grep "Resultado:"
echo "  Com argumento 10 (esperado: 11):"
./teste test_f2.txt 10 | grep "Resultado:"
echo ""

echo "Função 3: v0 = p0 + \$0; v1 = v0 - \$1; ret v1"
echo "  Com argumento 5 (esperado: 4):"
./teste test_f3.txt 5 | grep "Resultado:"
echo "  Com argumento 10 (esperado: 9):"
./teste test_f3.txt 10 | grep "Resultado:"
echo ""

echo "Função 4: v0 = \$10 + \$5; v1 = v0 - \$3; v2 = v1 * \$2; ret v2"
echo "  Esperado: 24 (independente do argumento)"
./teste test_f4.txt 0 | grep "Resultado:"
echo ""

echo "Função 5: v0 = p0 + \$1; v1 = p0 - \$1; v2 = v0 * v1; ret v2"
echo "  Calcula (p0+1)*(p0-1) = p0²-1"
echo "  Com argumento 5 (esperado: 24):"
./teste test_f5.txt 5 | grep "Resultado:"
echo "  Com argumento 10 (esperado: 99):"
./teste test_f5.txt 10 | grep "Resultado:"
echo ""

echo "Função 6: v0 = p0 * \$3; v1 = v0 + \$10; ret v1"
echo "  Calcula p0*3+10"
echo "  Com argumento 5 (esperado: 25):"
./teste test_f6.txt 5 | grep "Resultado:"
echo "  Com argumento 10 (esperado: 40):"
./teste test_f6.txt 10 | grep "Resultado:"

# Testando o zret
echo ""
echo "=== Testando zret ==="
echo "ARQUIVO 1: zret p0 \$5 (se 0 retorna 5, senão segue e retorna p0)"

echo "  Com argumento 0 (esperado: 5 - entrou no zret):"
./teste test_zret_1.txt 0 | grep "Resultado:"

echo "  Com argumento 5 (esperado: 5 - pulou zret, retornou p0):"
./teste test_zret_1.txt 5 | grep "Resultado:"

echo ""
echo "ARQUIVO 2: zret p0 p0 (se 0 retorna 0, senão segue e retorna 99)"

echo "  Com argumento 0 (esperado: 0 - entrou no zret):"
./teste test_zret_2.txt 0 | grep "Resultado:"

echo "  Com argumento 5 (esperado: 99 - pulou zret, retornou 99):"
./teste test_zret_2.txt 5 | grep "Resultado:"

echo ""
echo "=== Testando CALL (Chamada de Função) ==="

echo "Teste 1: Chamada Simples (Encadeada)"
echo "Arquivo: test_call_1.txt"
echo "Lógica: Chama a função 'inc' duas vezes. f(x) = x + 2"
echo "  Com argumento 10 (esperado: 12):"
./teste test_call_1.txt 10 | grep "Resultado:"
echo "  Com argumento 0 (esperado: 2):"
./teste test_call_1.txt 0 | grep "Resultado:"

echo ""
echo "Teste 2: Recursão e Múltiplas Funções (Soma dos Quadrados)"
echo "Arquivo: test_call_recursivo.txt"
echo "Lógica: Soma dos quadrados de 1 até N"
echo "  Com argumento 3 (esperado: 14 -> 9+4+1):"
./teste test_call_recursivo.txt 3 | grep "Resultado:"
echo "  Com argumento 4 (esperado: 30 -> 16+9+4+1):"
./teste test_call_recursivo.txt 4 | grep "Resultado:"
echo "  Com argumento 5 (esperado: 55):"
./teste test_call_recursivo.txt 5 | grep "Resultado:"