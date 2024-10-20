#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

#define BUFFER_SIZE 1024

bool is_vowel(char c) {
    return strchr("AEIOUYaeiouy", c) != NULL;
}

void remove_vowels(char *str) {
    int write_index = 0;

    for (int i = 0; str[i] != '\0'; i++) {
        if (!is_vowel(str[i])) {
            str[write_index++] = str[i];
        }
    }
    
    str[write_index] = '\0';
}

int main(int argc, char *argv[]) {

    int process_number = atoi(argv[1]);
    char *filename = argv[2];

    int file = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file == -1) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    while ((bytes_read = read(STDIN_FILENO, buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0';
        remove_vowels(buffer);
        write(file, buffer, strlen(buffer));

        if (process_number == 1) {
            printf("Child1 processed: %s", buffer);
        } else if (process_number == 2) {
            printf("Child2 processed: %s", buffer);
        }
    }

    close(file);
    return 0;
}
