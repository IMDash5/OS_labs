#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <sys/mman.h>

#define BUFFER_SIZE 1024

int random_80_percent() {
    return (rand() % 10) < 8;
}

int main() {
    int pipe1[2], pipe2[2];
    pid_t child1, child2;

    srand(time(NULL));

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("Pipe failed");
        return 1;
    }

    char filename1[BUFFER_SIZE], filename2[BUFFER_SIZE];
    printf("Enter filename for child1: ");
    fgets(filename1, sizeof(filename1), stdin);
    filename1[strcspn(filename1, "\n")] = '\0';

    printf("Enter filename for child2: ");
    fgets(filename2, sizeof(filename2), stdin);
    filename2[strcspn(filename2, "\n")] = '\0';

    int fd1 = open(filename1, O_RDWR | O_CREAT | O_TRUNC, 0644);
    int fd2 = open(filename2, O_RDWR | O_CREAT | O_TRUNC, 0644);

    if (fd1 == -1 || fd2 == -1) {
        perror("Error opening file");
        return 1;
    }

    // Убедитесь, что файлы имеют правильный размер перед отображением
    lseek(fd1, BUFFER_SIZE - 1, SEEK_SET);
    write(fd1, "", 1);
    lseek(fd1, 0, SEEK_SET);

    lseek(fd2, BUFFER_SIZE - 1, SEEK_SET);
    write(fd2, "", 1);
    lseek(fd2, 0, SEEK_SET);

    char *mapped_file1 = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);
    char *mapped_file2 = mmap(NULL, BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd2, 0);

    if (mapped_file1 == MAP_FAILED || mapped_file2 == MAP_FAILED) {
        perror("mmap failed");
        close(fd1);
        close(fd2);
        return 1;
    }

    if ((child1 = fork()) == 0) {
        close(pipe1[1]); 
        dup2(pipe1[0], STDIN_FILENO);

        execl("./child", "child", "1", filename1, NULL);
        perror("execl failed");
        exit(EXIT_FAILURE);
    }

    if ((child2 = fork()) == 0) {
        close(pipe2[1]);
        dup2(pipe2[0], STDIN_FILENO);

        execl("./child", "child", "2", filename2, NULL);
        perror("execl failed");
        exit(EXIT_FAILURE);
    }

    close(pipe1[0]); 
    close(pipe2[0]);  

    char buffer[BUFFER_SIZE];

    printf("Enter strings (type 'exit' to quit):\n");
    while (fgets(buffer, sizeof(buffer), stdin)) {
        if (strcmp(buffer, "exit\n") == 0) break;

        if (random_80_percent()) {
            strncpy(mapped_file1, buffer, BUFFER_SIZE);
            write(pipe1[1], buffer, strlen(buffer));
        } else {
            strncpy(mapped_file2, buffer, BUFFER_SIZE);
            write(pipe2[1], buffer, strlen(buffer)); 
        }
    }

    close(pipe1[1]);
    close(pipe2[1]);

    munmap(mapped_file1, BUFFER_SIZE);
    munmap(mapped_file2, BUFFER_SIZE);

    close(fd1);
    close(fd2);

    printf("Processing complete.\n");
    return 0;
}