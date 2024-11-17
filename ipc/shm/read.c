#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/myshm_c"
#define SEM_WRITER "/sem_writer"
#define SEM_READER "/sem_reader"
#define SHM_SIZE 1024

int main() {
  int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
  if (shm_fd == -1) {
    perror("shm_open");
    exit(1);
  }

  char *ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
  if (ptr == MAP_FAILED) {
    perror("mmap");
    close(shm_fd);
    exit(1);
  }

  sem_t *sem_writer = sem_open(SEM_WRITER, 0); // Only open Not Create
  sem_t *sem_reader = sem_open(SEM_READER, 0); // Only open Not Create

  if (sem_writer == SEM_FAILED || sem_reader == SEM_FAILED) {
    perror("sem_open");
    munmap(ptr, SHM_SIZE);
    close(shm_fd);
    exit(1);
  }

  while (1) {
    sem_wait(sem_reader);

    printf("Reader %s\n", ptr);

    sem_post(sem_writer);
    sleep(3);
  }

  munmap(ptr, SHM_SIZE);
  close(shm_fd);
  sem_close(sem_writer);
  sem_close(sem_reader);

  return 0;
}
