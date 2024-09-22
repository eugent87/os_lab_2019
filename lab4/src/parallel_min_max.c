#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int *child_pids = NULL;  // Для хранения PID-ов дочерних процессов
int pnum = -1;           // Для использования в обработчике сигнала

void alarm_handler(int signum) {
  printf("Timeout expired. Killing child processes...\n");
  for (int i = 0; i < pnum; i++) {
    kill(child_pids[i], SIGKILL);  // Отправляем сигнал SIGKILL дочерним процессам
  }
}

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  pnum = -1;  
  bool with_files = false;
  int timeout = 0;  // Таймаут в миллисекундах

  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {"timeout", required_argument, 0, 0},  // Таймаут в мс
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            if (seed <= 0) {
              printf("Seed should be a positive number!\n");
              return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            if (array_size <= 0) {
              printf("Array size should be a positive number!\n");
              return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            if (pnum <= 0 || pnum > array_size) {
              printf("Pnum should be a positive number and less than or equal to array size!\n");
              return 1;
            }
            break;
          case 3:
            with_files = true;
            break;
          case 4:
            timeout = atoi(optarg);  // Таймаут в мс
            if (timeout < 0) {
              printf("Timeout should be a non-negative number!\n");
              return 1;
            }
            break;
          default:
            printf("Unknown option index %d\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;
      case '?':
        break;
      default:
        printf("getopt returned unknown character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Unexpected argument after options!\n");
    return 1;
  }

  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout \"num\"]\n", argv[0]);
    return 1;
  }

  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);

  child_pids = malloc(sizeof(int) * pnum);  // Выделяем память для хранения PID-ов

  int pipefd[2];
  if (!with_files) {
    if (pipe(pipefd) == -1) {
      printf("Pipe creation failed!\n");
      return 1;
    }
  }

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  // Устанавливаем таймер для обработки таймаута
  if (timeout > 0) {
    signal(SIGALRM, alarm_handler);
    struct itimerval timer;
    timer.it_value.tv_sec = timeout / 1000;
    timer.it_value.tv_usec = (timeout % 1000) * 1000;  // Преобразование в микросекунды
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);  // Устанавливаем таймер
  }for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    if (child_pid >= 0) {
      child_pids[i] = child_pid;  // Сохраняем PID дочернего процесса
      if (child_pid == 0) {
        unsigned int begin = i * array_size / pnum;
        unsigned int end = (i == pnum - 1) ? array_size : (i + 1) * array_size / pnum;

        struct MinMax local_min_max = GetMinMax(array, begin, end);

        if (with_files) {
          char filename[256];
          snprintf(filename, sizeof(filename), "output_%d.txt", i);
          FILE *file = fopen(filename, "w");
          if (file == NULL) {
            printf("Error opening file for writing!\n");
            return 1;
          }
          fprintf(file, "%d %d\n", local_min_max.min, local_min_max.max);
          fclose(file);
        } else {
          close(pipefd[0]);  // Закрываем чтение
          write(pipefd[1], &local_min_max.min, sizeof(int));
          write(pipefd[1], &local_min_max.max, sizeof(int));
          close(pipefd[1]);
        }
        free(array);
        return 0;
      }
    } else {
      printf("Fork failed!\n");
      return 1;
    }
  }

  int active_child_processes = pnum;
  int status;
  while (active_child_processes > 0) {
    pid_t pid = waitpid(-1, &status, WNOHANG);  // Проверяем статус дочерних процессов без блокировки
    if (pid > 0) {
      active_child_processes--;  // Если процесс завершился, уменьшаем счетчик
    }

    usleep(10000);  // Даем небольшую задержку для таймера (10 мс)

    if (timeout > 0 && active_child_processes > 0) {
      // После срабатывания таймера обработчик сигналов сам завершит процессы
    }
  }

  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;
    if (with_files) {
      char filename[256];
      snprintf(filename, sizeof(filename), "output_%d.txt", i);
      FILE *file = fopen(filename, "r");
      if (file == NULL) {
        printf("Error reading file!\n");
        return 1;
      }
      fscanf(file, "%d %d", &min, &max);
      fclose(file);
    } else {
      close(pipefd[1]);  // Закрываем запись
      read(pipefd[0], &min, sizeof(int));
      read(pipefd[0], &max, sizeof(int));
    }

    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  free(array);
  free(child_pids);  // Освобождаем память для PID-ов

  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);

  return 0;
}