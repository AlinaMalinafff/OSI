#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h> // Include fcntl.h for O_CREAT and O_EXCL
#include <time.h> // Include time.h for time tracking

#define HONEY_LIMIT 30
#define HONEY_THRESHOLD (HONEY_LIMIT / 2)
#define PROGRESS_CHECK_INTERVAL 5 // Check honey progress every 5 seconds

// Структура для разделяемой памяти
typedef struct {
    int honey;
    int bee_count;
    sem_t *honey_sem;
    sem_t *bee_sem;
} SharedMemory;

// Функция для пчелы
void *bee_routine(void *arg) {
    int bee_id = *(int *)arg;
    SharedMemory *shm_ptr = (SharedMemory *)arg; // Access shared memory

    while (1) {
        // Получение семафора для меда
        sem_wait(shm_ptr->honey_sem); // Use shm_ptr to access honey_sem

        // Проверка, есть ли другие пчелы в улье
        if (shm_ptr->bee_count <= 1) {
            // Если пчела одна, она уходит из улья
            sem_post(shm_ptr->bee_sem);
            continue;
        }

        // Собирает мед
        shm_ptr->honey++;
        printf("Bee %d collected honey. Total honey: %d\n", bee_id, shm_ptr->honey);

        // Отпускает семафор для меда
        sem_post(shm_ptr->honey_sem);

        // Отпускает семафор для пчел
        sem_post(shm_ptr->bee_sem);
    }

    return NULL;
}

// Функция для медведя
void *bear_routine(void *arg) {
    SharedMemory *shm_ptr = (SharedMemory *)arg; // Access shared memory

    while (1) {
        // Получение семафора для меда
        sem_wait(shm_ptr->honey_sem); // Use shm_ptr to access honey_sem

        // Проверка, достаточно ли меда
        if (shm_ptr->honey < HONEY_THRESHOLD) {
            // Медведя не будит, отпускает семафор
            sem_post(shm_ptr->honey_sem);
            continue;
        }

        // Проверка, сколько пчел в улье
        if (shm_ptr->bee_count < 3) {
            // Медведь забирает весь мед
            shm_ptr->honey = 0;
            printf("Bear took all honey!\n");

            // Отпускает семафор для меда
            sem_post(shm_ptr->honey_sem);
            sleep(1); // Медведь съедает мед
        } else {
            // Медведя кусают пчелы
            printf("Bear was stung by bees!\n");

            // Отпускает семафор для меда
            sem_post(shm_ptr->honey_sem);
            sleep(3); // Медведь лечит укус
        }
    }

    return NULL;
}

// Функция для печати прогресса
void print_progress(SharedMemory *shm_ptr) {
    time_t current_time;
    struct tm *time_info;

    current_time = time(NULL);
    time_info = localtime(&current_time);

    printf("Current Honey Progress: %d (at %02d:%02d:%02d)\n", shm_ptr->honey, time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
}

// Обработчик сигнала прерывания
void sigint_handler(int sig) {
    printf("Exiting...\n");
    exit(0);
}

int main(int argc, char *argv[]) {
    int num_bees, shm_id;
    SharedMemory *shm_ptr;
    pthread_t *bees;
    pthread_t bear_thread;
    int *bee_ids;

    // Обработка сигнала прерывания
    signal(SIGINT, sigint_handler);

    // Получение количества пчел
    if (argc != 2) {
        printf("Usage: %s <number_of_bees>\n", argv[0]);
        return 1;
    }
    num_bees = atoi(argv[1]);

    // Создание разделяемой памяти
    shm_id = shmget(IPC_PRIVATE, sizeof(SharedMemory), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget");
        return 1;
    }
    shm_ptr = (SharedMemory *)shmat(shm_id, NULL, 0);
    if (shm_ptr == (void *)-1) {
        perror("shmat");
        return 1;
    }

    // Инициализация разделяемой памяти
    shm_ptr->honey = 0;
    shm_ptr->bee_count = 0;

    // Создание семафоров (POSIX)
    shm_ptr->honey_sem = sem_open("/my_honey_sem", O_CREAT | O_EXCL, 0666, 1); // Use unique names
    if (shm_ptr->honey_sem == SEM_FAILED) {
        perror("sem_open honey");
        return 1;
    }
    shm_ptr->bee_sem = sem_open("/my_bee_sem", O_CREAT | O_EXCL, 0666, 1); // Use unique names
    if (shm_ptr->bee_sem == SEM_FAILED) {
        perror("sem_open bee");
        return 1;
    }

    // Создание массива пчел
    bees = (pthread_t *)malloc(num_bees * sizeof(pthread_t));
    bee_ids = (int *)malloc(num_bees * sizeof(int));

    // Создание потоков для пчел
    for (int i = 0; i < num_bees; i++) {
        bee_ids[i] = i + 1;
        if (pthread_create(&bees[i], NULL, bee_routine, (void *)shm_ptr) != 0) { // Pass shm_ptr to bee_routine
            perror("pthread_create");
            return 1;
        }
        shm_ptr->bee_count++;
    }

    // Создание потока для медведя
    if (pthread_create(&bear_thread, NULL, bear_routine, (void *)shm_ptr) != 0) { // Pass shm_ptr to bear_routine
        perror("pthread_create");
        return 1;
    }

    // Печать прогресса каждые PROGRESS_CHECK_INTERVAL секунд
    while (1) {
        sleep(PROGRESS_CHECK_INTERVAL);
        print_progress(shm_ptr);
    }

    // Ожидание завершения потоков
    for (int i = 0; i < num_bees; i++) {
        if (pthread_join(bees[i], NULL) != 0) {
            perror("pthread_join");
            return 1;
        }
    }
    if (pthread_join(bear_thread, NULL) != 0) {
        perror("pthread_join");
        return 1;
    }

    // Удаление семафоров
    sem_unlink("/my_honey_sem");
    sem_unlink("/my_bee_sem");

    // Удаление разделяемой памяти
    shmdt(shm_ptr);
    shmctl(shm_id, IPC_RMID, NULL);

    free(bees);
    free(bee_ids);

    return 0;
}