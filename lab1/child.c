#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

void remove_vowels(char *str) {
    char *p = str;
    char *q = str;
    while (*p) {
        if (!strchr("AEIOUYaeiouy", *p)) {
            *q++ = *p;
        }
        p++;
    }
    *q = '\0';
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <process_number> <output_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int process_number = atoi(argv[1]);
    char *filename = argv[2];

    // Открываем файл для записи
    int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Чтение строк из stdin (перенаправлено из pipe)
    while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        remove_vowels(buffer);  // Удаление гласных
        write(file, buffer, strlen(buffer));  // Запись в файл

        // Вывод в стандартный вывод для child1 или child2
        if (process_number == 1) {
            printf("Child1 processed: %s", buffer);
        } else if (process_number == 2) {
            printf("Child2 processed: %s", buffer);
        }
    }

    close(file);
    return 0;
}
