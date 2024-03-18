#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ctype.h>

#define BUFFER_SIZE 5000

void replaceLowercaseVowels(char *buffer) {
    for (int i = 0; buffer[i] != '\0'; i++) {
        if (buffer[i] == 'a' || buffer[i] == 'e' || buffer[i] == 'i' || buffer[i] == 'o' || buffer[i] == 'u' || buffer[i] == 'y') {
            buffer[i] = toupper(buffer[i]);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }

    // достаем значения
    char *file_read = argv[1];
    char *file_write = argv[2];
    
    int fd_write;
    ssize_t write_bytes;

    int result_1, result_2;
    int fd_1[2], fd_2[2];
    size_t size, size2;

    if (pipe(fd_1) == -1 || pipe(fd_2) == -1) {
        perror("Pipe");
        return 1;
    }

    result_1 = fork();
    if (result_1 == -1) {
        perror("Fork");
        return 1;
    }
    
    if (result_1 > 0) { // первый
        char    buffer[BUFFER_SIZE];
        int     fd_read;
        ssize_t read_bytes;

        // Пытаемся открыть файл для чтения, иначе печатаем ошибку
        if((fd_read = open(file_read, O_RDONLY)) < 0){
            printf("Can\'t open for reading\n");
            exit(-1);
        }

        read_bytes = read(fd_read, buffer, BUFFER_SIZE);
        if(read_bytes == -1) {
            printf("Can\'t read\n");
            exit(-1);
        }

        if(close(fd_1[0]) < 0){
            printf("parent: Can\'t close pipe(reading)\n");
            exit(-1);
        }
        size = write(fd_1[1], buffer, read_bytes);
        if(size != read_bytes){
            printf("Can\'t write all\n");
            exit(-1);
        }
        if(close(fd_1[1]) < 0) {
            printf("parent: Can\'t close pipe(writing)\n");
            exit(-1);
        }
    } else if (result_1 == 0) { // второй
        if(close(fd_1[1]) < 0){
            printf("child: Can\'t close pipe(writing)\n");
            exit(-1);
        }
        
        char buffer[BUFFER_SIZE];
        
        size = read(fd_1[0], buffer, BUFFER_SIZE);
        
        if(close(fd_1[0]) < 0){
            printf("child: Can\'t close pipe(reading)\n");
            exit(-1);
        }
        
        replaceLowercaseVowels(buffer);
        
        result_2 = fork();
        if(result_2 < 0) {
            perror("Fork");
            return 1;
        } 
        
        if (result_2 != 0) { // второй
            if(close(fd_2[0]) < 0){
                printf("parent: Can\'t close pipe(reading)\n");
                exit(-1);
            }
            
            size_t old_size = size;
            size = write(fd_2[1], buffer, old_size);
            if(size != old_size){
                printf("Can\'t write all\n");
                exit(-1);
            }
            
            if(close(fd_2[1]) < 0) {
                printf("parent: Can\'t close pipe(writing)\n");
                exit(-1);
            }
            
        } else { // третий
            if (close(fd_2[1]) < 0){
                printf("child: Can\'t close pipe(writing)\n");
                exit(-1);
            }
            
            char buffer[BUFFER_SIZE];
            
            size = read(fd_2[0], buffer, BUFFER_SIZE);
            
            if(close(fd_2[0]) < 0){
                printf("child: Can\'t close pipe(reading)\n");
                exit(-1);
            }
            
            // Пытаемся открыть файл для записи, иначе печатаем ошибку
            if((fd_write = open(file_write, O_WRONLY | O_CREAT | O_TRUNC, 0666)) < 0){
                printf("Can\'t open for writing\n");
                exit(-1);
            }
            
            write_bytes = write(fd_write, buffer, size);
            if(write_bytes == -1) {
                printf("Can\'t write\n");
                exit(-1);
            }
        }
    }
    
    return 0;
}
