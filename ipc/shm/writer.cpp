#include <iostream>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <ctime>

#define SHM_NAME "/myshm_cpp"
#define SEM_WRITER "/sem_writer"
#define SEM_READER "/sem_reader"
#define SHM_SIZE 1024

// 生成随机字符串
void generateRandomString(char *str, size_t size) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    for (size_t i = 0; i < size - 1; i++) {
        int key = rand() % (sizeof(charset) - 1);
        str[i] = charset[key];
    }
    str[size - 1] = '\0';
}

int main() {
    srand(time(NULL)); // 初始化随机数种子

    // 创建或打开共享内存对象
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    // 设置共享内存大小
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return 1;
    }

    // 将共享内存映射到进程地址空间
    char *ptr = static_cast<char *>(mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
    if (ptr == MAP_FAILED) {
        perror("mmap");
        close(shm_fd);
        shm_unlink(SHM_NAME);
        return 1;
    }

    // 创建信号量
    sem_t *sem_writer = sem_open(SEM_WRITER, O_CREAT, 0666, 1); // 写者信号量，初始值为1
    sem_t *sem_reader = sem_open(SEM_READER, O_CREAT, 0666, 0); // 读者信号量，初始值为0

    if (sem_writer == SEM_FAILED || sem_reader == SEM_FAILED) {
        perror("sem_open");
        munmap(ptr, SHM_SIZE);
        close(shm_fd);
        shm_unlink(SHM_NAME);
        sem_unlink(SEM_WRITER);
        sem_unlink(SEM_READER);
        return 1;
    }

    // 循环写入数据
    while (true) {
        sem_wait(sem_writer); // 等待写入信号

        // 生成随机字符串
        char message[100];
        generateRandomString(message, sizeof(message));
        strncpy(ptr, message, SHM_SIZE);
        std::cout << "Writer: Wrote message to shared memory: \"" << message << "\"" << std::endl;

        sem_post(sem_reader); // 发送读取信号
        sleep(1); // 模拟延迟
    }

    // 清理资源
    munmap(ptr, SHM_SIZE);
    close(shm_fd);
    sem_close(sem_writer);
    sem_close(sem_reader);

    return 0;
}
