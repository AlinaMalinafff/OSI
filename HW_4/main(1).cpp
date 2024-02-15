#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BUFF_SIZE 1024

int main(int argc, char *argv[]) {
  // проверка написали ли все параметры для корректного запуска программы
  if (argc != 3) {
    printf("неправильное количество аргументов запуска кода\n");
    return 1;
  }

  // argv[0] - название запускаемого файла
  // argv[1] - имя файла для чтения
  // argv[2] - имя файла для записи
  char *input_file = argv[1];
  char *output_file = argv[2];

  // открытие файла для чтения
  int input_fd = open(input_file, O_RDONLY);
  // проверка, что он открылся
  if (input_fd == -1) {
    printf("ошибка detected:");
    perror("ошибка в открытии файла для чтения");
    return 1;
  }

  char buffer[BUFF_SIZE];
  int bytes_read;
  int bytes_written;

  // открытие или создание файла для записи
  int output_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (output_fd == -1) {
    printf("ошибка detected:");
    perror("ошибка в открытии файла для записи");
    return 1;
  }

  // чтение данных из файла и запись в другой файл
  while ((bytes_read = read(input_fd, buffer, BUFF_SIZE)) > 0) {
    bytes_written = write(output_fd, buffer, bytes_read);
    if (bytes_read == -1) {
      printf("ошибка detected:");
      perror("ошибка в чтении данных из файла");
      return 1;
    }
    if (bytes_written != bytes_read) {
      printf("ошибка detected:");
      perror("ошибка в записи в файл");
      return 1;
    }
    if (bytes_written == -1) {
      printf("ошибка detected:");
      perror("ошибка в записи данных в файл");
      return 1;
    }
  }

  // закрытие файлов
  close(input_fd);
  close(output_fd);

  printf("спасибо за выбор нашей программы для переноса данных из файла в файл :)\n");

  return 0;
}