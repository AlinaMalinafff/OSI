#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// как изменяются числа при вводе и прогонке через функцию фибоначчи
// 0 -> 0
// 1 -> 1
// 2 -> 1
// 3 -> 2
// 4 -> 3
// 5 -> 5 и т.д.

// функция, считающая числа фибоначчи
unsigned long long fibonacci(unsigned long long n) {
  if (n <= 1) {
    return n;
  }
  unsigned long long fib_1 = 0, fib_2 = 1, sum;
  for (unsigned long long i = 2; i <= n; i++) {
    if (fib_1 + fib_2 < sum) {
      printf("the num of fibonacci is too big, the last possible fibonacci to "
             "calculate will be printed\n");
      break;
    }
    sum = fib_1 + fib_2;
    fib_1 = fib_2;
    fib_2 = sum;
  }
  return fib_2;
}

// функция для подсчёта факториала числа
unsigned long long factorial(unsigned long long n) {
  if (n == 0 || n == 1) {
    return 1;
  } else {
    unsigned long long factorial = 1;
    for (unsigned long long i = 2; i <= n; ++i) {
      if (factorial * i < factorial) {
        printf("the num of factorial is too big, the last possible factorial "
               "to calculate will be printed\n");
        break;
      }
      factorial *= i;
    }
    return factorial;
  }
}

// мейн
int main(int argc, char *argv[]) {
// как именно выводится через командную строку и проверка на количество аргументов в командной строке
  if (argc != 2) {
    printf("Usage: ./program <number>\n");
    return 1;
  }

// проверяем получаемое число
  int n = atoi(argv[1]);
  if (n < 0) {
    printf("Invalid input. Please enter a non-negative integer the next "
           "time.Goodbye\n");
    return 0;
  }

// создаём процессного ребёнка
  pid_t chpid = fork();

// если у процесса число меньше нуля, то всё плохо
  if (chpid < 0) {
    printf("fork gone wrong\n");
    return 1;
// если 0, то это ребёночий процесс
  } else if (chpid == 0) { // child process
    printf("info about child process: pid = %d, ppid = %d\ncalculatiing "
           "factorial\n",
           getpid(), getppid());
    unsigned long long result = factorial(n);
    printf("child process with pid %d calculated factorial of %d as %llu\n",
           getpid(), n, result);
    printf("\n");
// если большее нуля число, то это родительский процесс
  } else { // parent process
    printf("info about parent process: pid = %d, ppid = %d\ncalculatiing "
           "fibonacci\n",
           getpid(), getppid());
    unsigned long long result = fibonacci(n);
    printf("parent process with pid %d calculated fibonacci of %d as %llu\n",
           getpid(), n, result);
  }

  return 0;
}
