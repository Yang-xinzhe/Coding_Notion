#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>


#define GPIO_BASE_ADDR 0xF0200000 // GPIO0 控制器基地址
#define GPIO_REG_SIZE 0x1000      // 映射大小 4KB

// GPIO寄存器偏移
#define GPIO_MODE0_OFFSET 0x08     // GPIO模式寄存器
#define GPIO_OVAL_OFFSET 0x10      // GPIO输出寄存器
#define GPIO_BIT_SET_OFFSET 0x48   // GPIO置位寄存器
#define GPIO_BIT_RESET_OFFSET 0x4C // GPIO清零寄存器

#define GPIO_PIN1_MASK (1 << 1) // GPIO Pin1 对应的位掩码

int main() {
  int fd;
  volatile unsigned int *gpio_base; // GPIO 基地址映射指针

  // 打开 /dev/mem 文件，获取物理内存访问权限
  fd = open("/dev/mem", O_RDWR | O_SYNC);
  if (fd < 0) {
    perror("Failed to open /dev/mem");
    return -1;
  }

  // 映射 GPIO 控制器寄存器
  gpio_base =
      (volatile unsigned int *)mmap(NULL, GPIO_REG_SIZE, PROT_READ | PROT_WRITE,
                                    MAP_SHARED, fd, GPIO_BASE_ADDR);
  if (gpio_base == MAP_FAILED) {
    perror("Failed to mmap");
    close(fd);
    return -1;
  }

  // 设置 GPIO Pin1 为输出模式
  gpio_base[GPIO_MODE0_OFFSET / 4] |= GPIO_PIN1_MASK;

  // 将 GPIO Pin1 设置为高电平
  gpio_base[GPIO_BIT_SET_OFFSET / 4] = GPIO_PIN1_MASK;
  printf("GPIO Pin1 set to HIGH.\n");

  sleep(1); // 延时 1 秒

  // 将 GPIO Pin1 设置为低电平
  gpio_base[GPIO_BIT_RESET_OFFSET / 4] = GPIO_PIN1_MASK;
  printf("GPIO Pin1 set to LOW.\n");

  // 解除映射并关闭文件
  if (munmap((void *)gpio_base, GPIO_REG_SIZE) == -1) {
    perror("Failed to munmap");
  }
  close(fd);

  return 0;
}
