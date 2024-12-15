#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
  int fd = open("./mmap.txt", O_RDWR, 0666);
  if (fd < 0) {
    printf("file open failed!\n");
    return -1;
  }
  struct stat st;
  if(fstat(fd, &st) < 0) {
    printf("failed to get file size.\n");
    return -1;
  }

  size_t file_size = st.st_size;
  printf("file size is %ld\n", file_size);

  // size_t page_size = getpagesize();
  // printf("page_size is %ld\n", page_size);

  void* map = mmap(0, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if(map == MAP_FAILED){
    perror("map is failed");
    close(fd);
    return -1;
  }
  printf("mmap is %s\n", (char*)map);

  char* str = (char *)map;
  // str[5] = '+';
  strcpy(str + 6, "Universe");
  printf("After modified: %s\n", (char *)map);
  ftruncate(fd, file_size + 3);
  msync(map, file_size + 3, MS_SYNC);

  munmap(map, file_size + 3);
  return 0;
}
