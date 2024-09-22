CC = gcc
CFLAGS = -Wall

all: parallel_min_max process_memory

# Компиляция программы parallel_min_max без передачи параметров
parallel_min_max : utils.o find_min_max.o utils.h find_min_max.h
	$(CC) -o parallel_min_max utils.o find_min_max.o parallel_min_max.c $(CFLAGS)

utils.o : utils.h
	$(CC) -o utils.o -c utils.c $(CFLAGS)

find_min_max.o : utils.h find_min_max.h
	$(CC) -o find_min_max.o -c find_min_max.c $(CFLAGS)

# Компиляция программы process_memory
process_memory: process_memory.c
	$(CC) $(CFLAGS) -o process_memory process_memory.c

# Очистка
clean:
	rm -f utils.o find_min_max.o parallel_min_max process_memory

# Добавляем правило для запуска программы parallel_min_max с параметрами
run_parallel_min_max: parallel_min_max
	./parallel_min_max --seed 4 --array_size 10000 --pnum 20 --timeout 1
