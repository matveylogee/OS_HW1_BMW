#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#define BUFFER_SIZE 5000
char name_1[] = "one.fifo";
char name_2[] = "two.fifo";

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
    
    (void)umask(0);

    mknod(name_1, S_IFIFO | 0666, 0);
    mknod(name_2, S_IFIFO | 0666, 0);

    // достаем значения
    char *file_read = argv[1];
    char *file_write = argv[2];

    int     fd_write;
    ssize_t write_bytes;

    int result_1, result_2;
    int fd_1, fd_2;
    size_t size, size2;

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

        if((fd_1 = open(name_1, O_WRONLY)) < 0){
            printf("Can\'t open FIFO(writting)\n");
            exit(-1);
        }
        size = write(fd_1, buffer, read_bytes);
        if(size != read_bytes){
            printf("Can\'t write all\n");
            exit(-1);
        }
        if(close(fd_1) < 0) {
            printf("parent: Can\'t close pipe(writing)\n");
            exit(-1);
        }
    } else { // второй
        if((fd_1 = open(name_1, O_RDONLY)) < 0){
            printf("Can\'t open FIFO(reading)\n");
            exit(-1);
        }
        
        char buffer[BUFFER_SIZE];
        
        size = read(fd_1, buffer, BUFFER_SIZE);
        
        if(close(fd_1) < 0){
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
            if((fd_2 = open(name_2, O_WRONLY)) < 0){
                printf("Can\'t open FIFO(writting)\n");
                exit(-1);
            }
            
            size_t old_size = size;
            size = write(fd_2, buffer, old_size);
            if(size != old_size){
                printf("Can\'t write all\n");
                exit(-1);
            }
            
            if(close(fd_2) < 0) {
                printf("parent: Can\'t close pipe(writing)\n");
                exit(-1);
            }

        } else { // третий
            if((fd_2 = open(name_2, O_RDONLY)) < 0){
                printf("Can\'t open FIFO(reading)\n");
                exit(-1);
            }
            
            char buffer[BUFFER_SIZE];
            
            size = read(fd_2, buffer, BUFFER_SIZE);
            
            if(close(fd_2) < 0){
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
