import random

entrada = open('entrada.in', 'w')

for i in range (50):
    nome_arq = 'AxB_' + str(i+1)
    entrada.write(nome_arq + '\n') # Adiciona o nome dos arquivos de cada matriz ao arquivo entrada.in

    arq = open(nome_arq, 'w') # Cria os arquivos com suas respectivas matrizes

    # Cria e escreve a primeira matriz
    for m in range(10):
        for n in range(10):
            num = random.uniform(-10,+10)
            arq.write('{:.2f}'.format(num) + ' ')
        arq.write(str('\n'))

    arq.write(str('\n')) # Linha de separação entre as matrizes

    # Cria e escreve a segunda matriz
    for m in range(10):
        for n in range(10):
            num = random.uniform(-10,+10)
            arq.write('{:.2f}'.format(num) + ' ')
        arq.write(str('\n'))
    arq.close()

entrada.close()
