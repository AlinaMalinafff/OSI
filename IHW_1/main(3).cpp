#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BS 5000

// Проверка, является ли строка палиндромом
int is_palindrome(char *str) {
  int len = strlen(str);
  for (int i = 0; i < len / 2; i++) {
    if (str[i] != str[len - i - 1]) {
      return 0;
    }
  }
  return 1;
}

int main(int argc, char *argv[]) {
  if (argc != 3) { // чек ввода
    fprintf(stderr, "некорректный формат командной строки", argv[0]);
    return 1;
  }

  // неименованный канал
  int fd[2];
  if (pipe(fd) == -1) {
    perror("ошибка создания канала");
    return 1;
  }

  // форк для дочернего процесса
  pid_t pid = fork();
  if (pid == -1) {
    perror("ошибка форка дочернего процесса");
    return 1;
  }

  // если процесс родительский
  if (pid > 0) {
    // конец записи канала
    close(fd[1]); 

    // открытие входного файла
    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
      perror("щшибка открытия входного файла");
      return 1;
    }

    // данные из файла
    char buffer[BS];
    size_t bytes_read = fread(buffer, 1, BS, fp);
    if (bytes_read == 0) {
      perror("ошибка чтения из файла");
      return 1;
    }

    // данные по каналу
    if (write(fd[0], buffer, bytes_read) == -1) {
      perror("ошибка записи в канал");
      return 1;
    }

    // закрытие входного файла 
    fclose(fp); 

    // ждём до конца дочернего процесса
    wait(NULL);

    // результат из канала
    int is_pal;
    if (read(fd[0], &is_pal, sizeof(int)) == -1) {
      perror("ошибка чтения из канала");
      return 1;
    }

    // закрытие канала
    close(fd[0]);

    // открытие выходного файла
    FILE *fp_out = fopen(argv[2], "w");
    if (fp_out == NULL) {
      perror("ошибка открытия выходного файла");
      return 1;
    }

    // запись результата в файл
    if (is_pal) {
      fprintf(fp_out, "палиндром\n");
    } else {
      fprintf(fp_out, "не палиндром\n");
    }

    // закрытие выходного файла
    fclose(fp_out);
  } 
  // процесс дочерний_1
  else {
    // конец чтения канала
    close(fd[0]);

    // данные из канала
    char buffer[BS];
    size_t bytes_read = read(fd[1], buffer, BS);
    if (bytes_read == -1) {
      perror("лшибка чтения из канала");
      return 1;
    }

    // обработка данных
    int is_pal = is_palindrome(buffer);

    // результат по каналу
    if (write(fd[1], &is_pal, sizeof(int)) == -1) {
      perror("ошибка записи в канал");
      return 1;
    }

    // закрытие канала
    close(fd[1]);
  }

  return 0;
}