#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <time.h>

#define BUFFER_SIZE 1024

int random_80_percent() {
    return (rand() % 10) < 8;
}

int main() {
    int pipe1[2], pipe2[2];
    pid_t child1, child2;

    srand(time(NULL));

    // Создаем два pipe
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("Pipe failed");
        return 1;
    }

    // Получаем имена файлов от пользователя
    char filename1[BUFFER_SIZE], filename2[BUFFER_SIZE];
    printf("Enter filename for child1: ");
    fgets(filename1, sizeof(filename1), stdin);
    filename1[strcspn(filename1, "\n")] = '\0';  // Удаляем символ новой строки

    printf("Enter filename for child2: ");
    fgets(filename2, sizeof(filename2), stdin);
    filename2[strcspn(filename2, "\n")] = '\0';  // Удаляем символ новой строки

    // Создание первого дочернего процесса
    if ((child1 = fork()) == 0) {
        // Перенаправляем каналы на дочерний процесс
        close(pipe1[1]);  // Закрываем конец записи pipe1 в дочернем процессе
        dup2(pipe1[0], STDIN_FILENO);  // Перенаправляем pipe1 в стандартный ввод

        // Запускаем дочерний процесс 1
        execl("./child", "child", "1", filename1, NULL);
        perror("execl failed");
        exit(EXIT_FAILURE);
    }

    // Создание второго дочернего процесса
    if ((child2 = fork()) == 0) {
        // Перенаправляем каналы на дочерний процесс
        close(pipe2[1]);  // Закрываем конец записи pipe2 в дочернем процессе
        dup2(pipe2[0], STDIN_FILENO);  // Перенаправляем pipe2 в стандартный ввод

        // Запускаем дочерний процесс 2
        execl("./child", "child", "2", filename2, NULL);
        perror("execl failed");
        exit(EXIT_FAILURE);
    }

    // Родительский процесс
    close(pipe1[0]);  // Закрываем конец чтения pipe1
    close(pipe2[0]);  // Закрываем конец чтения pipe2

    char buffer[BUFFER_SIZE];

    // Чтение строк от пользователя и отправка в соответствующий pipe
    printf("Enter strings (type 'exit' to quit):\n");
    while (fgets(buffer, sizeof(buffer), stdin)) {
        if (strcmp(buffer, "exit\n") == 0) break;  // Завершение программы

        // Отправляем строку в pipe1 с вероятностью 80%, иначе в pipe2
        if (random_80_percent()) {
            write(pipe1[1], buffer, strlen(buffer));  // В pipe1
        } else {
            write(pipe2[1], buffer, strlen(buffer));  // В pipe2
        }
    }

    // Закрываем концы записи в pipe
    close(pipe1[1]);
    close(pipe2[1]);

    printf("Processing complete.\n");
    return 0;
}
