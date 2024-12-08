#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

using namespace std;

class ThreadPool {
public:
    ThreadPool(size_t numThreads) : stop(false) {// Constructor
        for(size_t i = 0 ; i < numThreads ; ++i) {
            workers.emplace_back(&ThreadPool::worker, this);
        }
    } 
    ~ThreadPool() { // Deconstructor
        stop.store(true); // atomic_bool set True Stop all the Threads
        cv.notify_all(); // condition_variable notify all Threads
        for(auto& worker : workers) {
            if(worker.joinable()) {
                worker.join();
            }
        }
    } 
    
    template <class F>
    void enqueue(F&& f) {
        {
            lock_guard<mutex> lock(mtx);
            tasks.push(forward<F>(f)); // add to queue
        }
        cv.notify_one(); // one thread execute work
    }


private:
    void worker() { // Keep loading work from work_queue
        while(!stop.load()) {
            function<void()> task;
            {
                unique_lock<mutex> lock(mtx); // Lock work_queue
                cv.wait(lock, [this] { return !tasks.empty() || stop.load(); }); // wait work or stop signal
                if (stop.load() && tasks.empty()) {
                    return ; // exit when the queue is empty and thread pool is stop
                }
                task = move(tasks.front()); // get task from queue
                tasks.pop(); // remove task from queue
            }
            cout << "Thread " << this_thread::get_id() << "processing task..." << endl;
            task(); // execute task
        }
    }

    vector<thread> workers; // Threads Container
    queue<function<void()>> tasks; // Working Queue
    mutex mtx; // mutex lock
    condition_variable cv; 
    atomic_bool stop; // Stop flag for Threads Pool
};

// void exampleTask(int id) {
//     cout << "Task" << id << " is being processed by thread " << this_thread::get_id() << endl;
// }

void exampleTask(int id) {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::cout << "Task " << id << " started at " << std::ctime(&time)
              << " by thread " << std::this_thread::get_id() << std::endl;
    // 模拟任务执行时间
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    now = std::chrono::system_clock::now();
    time = std::chrono::system_clock::to_time_t(now);
    std::cout << "Task " << id << " finished at " << std::ctime(&time)
              << " by thread " << std::this_thread::get_id() << std::endl;
}

int main(int argc, char const *argv[])
{
    ThreadPool pool(4);
    for (double i = 0 ; i < 10 ; ++i) {
        pool.enqueue([i] { exampleTask(i); });
    }
    this_thread::sleep_for(chrono::seconds(2));
    return 0;
}

