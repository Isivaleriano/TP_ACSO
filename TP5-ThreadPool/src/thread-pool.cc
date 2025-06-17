#include "thread-pool.h"
using namespace std;

ThreadPool::ThreadPool(size_t numThreads) : wts(numThreads), done(false), isShuttingDown(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        wts[i].ts = thread([this, i] { worker(i); });
    }
    dt = thread([this] { dispatcher(); });
}

void ThreadPool::schedule(const function<void(void)>& thunk) {
    if (!thunk) {
        throw invalid_argument("Cannot schedule nullptr task.");
    }

    {
        lock_guard<mutex> lock(shutdownLock);
        if (done || isShuttingDown) {
            throw runtime_error("Cannot schedule tasks after shutdown.");
        }
    }

    {
        lock_guard<mutex> lock(queueLock);
        taskQueue.push(thunk);
    }

    {
        lock_guard<mutex> lock(tasksLock);
        tasksInFlight++;
    }

    taskAvailable.signal();
}


void ThreadPool::dispatcher() {
    while (true) {
        taskAvailable.wait(); 
        if (done) break;

        function<void(void)> task;
        {
            lock_guard<mutex> lock(queueLock);
            if (taskQueue.empty()) continue;
            task = taskQueue.front();
            taskQueue.pop();
        }

        while (true) {
            for (auto& w : wts) {
                lock_guard<mutex> guard(w.mutex_worker);
                if (w.available) {
                    w.available = false;
                    w.thunk = task;
                    w.ready.signal();
                    goto assigned;
                }
            }
            this_thread::yield();
        }
    assigned:
        ;
    }
}

void ThreadPool::worker(int id) {
    while (true) {
        wts[id].ready.wait();
        if (done) break;

        wts[id].thunk();

        {
            lock_guard<mutex> guard(wts[id].mutex_worker);
            wts[id].available = true;
        }

        bool notify = false;
        {
            lock_guard<mutex> lock(tasksLock);
            tasksInFlight--;
            if (tasksInFlight == 0) notify = true;
        }

        if (notify) {
            int n_waiters = 0;
            {
                lock_guard<mutex> lock(waitersLock);
                n_waiters = waiters;
            }
            for (int i = 0; i < n_waiters; ++i) {
                allDone.signal();
            }
        }
    }
}


void ThreadPool::wait() {
    {
        lock_guard<mutex> lock(tasksLock);
        if (tasksInFlight == 0) return;
    }

    {
        lock_guard<mutex> lock(waitersLock);
        waiters++;
    }

    allDone.wait();

    {
        lock_guard<mutex> lock(waitersLock);
        waiters--;
    }
}


ThreadPool::~ThreadPool() {
    wait();
    {
        lock_guard<mutex> lock(shutdownLock);
        isShuttingDown = true;
        done = true;
    }

    taskAvailable.signal(); 
    for (auto& w : wts) w.ready.signal(); 

    if (dt.joinable()) dt.join();
    for (auto& w : wts) {
        if (w.ts.joinable()) w.ts.join();
    }
}
