#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>

#define SHM_NAME "/myshm_cpp"
#define SEM_WRITER "/sem_writer"
#define SEM_READER "/sem_reader"
#define SHM_SIZE 1024

int main() {
  // 打开共享内存对象
  int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
  if (shm_fd == -1) {
    perror("shm_open");
    return 1;
  }

  // 将共享内存映射到进程地址空间
  char *ptr =
      static_cast<char *>(mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0));
  if (ptr == MAP_FAILED) {
    perror("mmap");
    close(shm_fd);
    return 1;
  }

  // 打开信号量
  sem_t *sem_writer = sem_open(SEM_WRITER, 0); // 只打开，不创建
  sem_t *sem_reader = sem_open(SEM_READER, 0); // 只打开，不创建

  if (sem_writer == SEM_FAILED || sem_reader == SEM_FAILED) {
    perror("sem_open");
    munmap(ptr, SHM_SIZE);
    close(shm_fd);
    return 1;
  }

  // 循环读取数据
  while (true) {
    sem_wait(sem_reader); // 等待读取信号

    std::cout << "Reader: Read message from shared memory: \"" << ptr << "\""
              << std::endl;

    sem_post(sem_writer); // 释放写入信号
    sleep(1);             // 模拟延迟
  }

  // 清理资源
  munmap(ptr, SHM_SIZE);
  close(shm_fd);
  sem_close(sem_writer);
  sem_close(sem_reader);

  return 0;
}
