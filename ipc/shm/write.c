#include <bits/pthreadtypes.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <semaphore.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#define SHM_NAME "/myshm_c"
#define SEM_WRITER "/sem_writer"
#define SEM_READER "/sem_reader"
#define SHM_SIZE 1024

void generate_random_string(char *str, size_t size) {
  const char charset[] =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  for (size_t i = 0; i < size; ++i) {
    int key = rand() % (sizeof(charset) - 1);
    str[i] = charset[key];
  }
  str[size - 1] = '\0';
}

int main() {
  srand(time(NULL));

  int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(1);
  }

  // set shared memory size
  if (ftruncate(shm_fd, SHM_SIZE) == -1) {
    perror("ftruncate");
    close(shm_fd);
    shm_unlink(SHM_NAME);
    exit(1);
  }

  char *ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (ptr == MAP_FAILED) {
    perror("mmap");
    close(shm_fd);
    shm_unlink(SHM_NAME);
    exit(1);
  }

  // Create semaphore
  sem_t *sem_writer = sem_open(SEM_WRITER, O_CREAT, 0666, 1);
  sem_t *sem_reader = sem_open(SEM_READER, O_CREAT, 0666, 0);

  if (sem_writer == SEM_FAILED || sem_reader == SEM_FAILED) {
    perror("sem_open");
    munmap(ptr, SHM_SIZE);
    close(shm_fd);
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_WRITER);
    sem_unlink(SEM_READER);
    exit(1);
  }
  while (1) {
    sem_wait(sem_writer);

    char message[20];
    generate_random_string(message, sizeof(message));
    strncpy(ptr, message, SHM_SIZE);
    printf("Writer: %s\n", message);

    sem_post(sem_reader);
    sleep(1);
  }

  munmap(ptr, SHM_SIZE);
  close(shm_fd);
  sem_close(sem_writer);
  sem_close(sem_reader);

  return 0;
}
