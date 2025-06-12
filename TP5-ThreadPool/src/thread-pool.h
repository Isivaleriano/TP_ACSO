#ifndef _thread_pool_
#define _thread_pool_

#include <cstddef>
#include <functional>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include "Semaphore.h"

using namespace std;

typedef struct worker {
    thread ts;                        
    function<void(void)> thunk;         
    Semaphore ready{0};               
    bool available = true;             
    mutex mutex_worker;               
} worker_t;

class ThreadPool {
public:
    ThreadPool(size_t numThreads);             
    void schedule(const function<void(void)>& thunk);   
    void wait();                                       
    ~ThreadPool();                                     

    ThreadPool(const ThreadPool& original) = delete;
    ThreadPool& operator=(const ThreadPool& rhs) = delete;

private:
    void worker(int id);         
    void dispatcher();         

    thread dt;                  
    vector<worker_t> wts;         

    queue<function<void(void)>> taskQueue; 
    mutex queueLock;            
    Semaphore taskAvailable{0};   

    bool done;                   

    int tasksInFlight = 0;      
    mutex tasksLock;            
    Semaphore allDone{0};        
};

#endif
