#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    // Создаем процесс
    pid_t pid = fork();

    if (pid == -1) {
        // Ошибка при вызове fork
        perror("fork");
        return 1;
    } else if (pid == 0) {
        // Дочерний процесс
        printf("Запуск программы sequential_min_max из дочернего процесса\n");

        // Выполняем программу sequential_min_max
        char *args[] = {"./sequential_min_max", "10", "100", NULL};
        execv(args[0], args);

        // Если execv не сработал, выводим ошибку
        perror("execv");
        exit(1);
    } else {
        // Родительский процесс
        printf("Ожидание завершения дочернего процесса\n");
        int status;
        waitpid(pid, &status, 0); // Ожидаем завершение дочернего процесса

        if (WIFEXITED(status)) {
            printf("Дочерний процесс завершился с кодом: %d\n", WEXITSTATUS(status));
        } else {
            printf("Дочерний процесс завершился с ошибкой\n");
        }
    }

    return 0;
}
