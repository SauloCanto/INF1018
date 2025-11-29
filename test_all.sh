#!/bin/bash


echo "=== Testando todos os arquivos .txt do diretório ==="
for arq in *.txt; do
	if [ "$arq" = "trabalho.txt" ]; then
		continue
	fi
	echo "Arquivo: $arq"
	echo "  Argumento 0:"
	./main "$arq" 0 | grep "Resultado:"
	echo "  Argumento 5:"
	./main "$arq" 5 | grep "Resultado:"
	echo "  Argumento 10:"
	./main "$arq" 10 | grep "Resultado:"
	echo ""
done