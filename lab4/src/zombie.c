#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
//ps aux | grep Z
int main() {
    pid_t pid = fork();

    if (pid > 0) {
        // Родительский процесс
        printf("Родительский процесс. PID: %d\n", getpid());
        sleep(5); // Ждем, чтобы увидеть зомби процесс
    } else if (pid == 0) {
        // Дочерний процесс
        printf("Дочерний процесс. PID: %d\n", getpid());
        exit(0); // Завершаем дочерний процесс, создавая зомби
    } else {
        // Ошибка fork()
        perror("fork");
        return 1;
    }

    // Родительский процесс ждет завершения дочернего
    wait(NULL);
    return 0;
}