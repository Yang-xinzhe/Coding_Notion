## share memory in C
* first create the share memory allocation then do the rest execution
```
gcc write.c -o write -lrt -lpthread
gcc read.c -o read -lrt -lpthread
```
```
./write # First execute
./read  
```
## share memory in C++
```
g++ writer.cpp -o writer -lrt -lpthread
g++ reader.cpp -o reader -lrt -lpthread
```
* ```-lrt``` stand for **Real-Time Library** 
* ```-lpthread``` stand for **Pthread Library** 
