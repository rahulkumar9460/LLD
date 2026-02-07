#include<iostream>
#include<chrono>
#include<functional>
#include<queue>
#include<memory>
#include<condition_variable>
#include<atomic>
#include<vector>
#include<thread>

using namespace std;

enum class JobStatus {
    SCHEDULED,
    RUNNING,
    FAILED,
    COMPLETED
};

class Job {
public:
    int jobId;
    chrono::steady_clock::time_point createdAt;
    chrono::steady_clock::time_point executedAt;
    atomic<JobStatus> status;
    function<void()> task;
    uint16_t maxRetry;
    uint16_t retryCount;


    Job(int id, function<void()> task, uint16_t maxRetry, chrono::steady_clock::time_point executedAt)
        : jobId(id), task(task), maxRetry(maxRetry), retryCount(0), executedAt(executedAt) {
        createdAt = chrono::steady_clock::now();
        status = JobStatus::SCHEDULED;
    }
};

class JobComparator {
public:
    bool operator() (const shared_ptr<Job>& job1, const shared_ptr<Job>& job2) {
        return job1->executedAt > job2->executedAt;
    }
};

class JobScheduler {
    priority_queue<shared_ptr<Job>, vector<shared_ptr<Job>>, JobComparator> scheduleQueue;
    mutex scheduleMux;
    queue<shared_ptr<Job>> readyQueue;
    mutex readyMux;

    condition_variable notEmptyCv;
    condition_variable readyCv;
    uint16_t maxWorkerThreads;
    atomic<bool> shutDownFlag;

    thread schedulerThread;
    vector<thread> workerThreads;

public:
    JobScheduler(uint16_t workerThreads) : maxWorkerThreads(workerThreads), shutDownFlag(false) {

    }
    void insert(shared_ptr<Job> job) {
        lock_guard<mutex> lock(scheduleMux);
        scheduleQueue.push(job);
        notEmptyCv.notify_one();
    }

    void start() {
        this->schedulerThread = thread(&JobScheduler::scheduler, this);

        for(uint16_t i=0; i<maxWorkerThreads; i++) {
            this->workerThreads.push_back(thread(&JobScheduler::worker, this));
        }
    }

    void stop() {
        shutDownFlag = true;

        // notify all waiting threads
        notEmptyCv.notify_all();
        readyCv.notify_all();

        if(schedulerThread.joinable()) {
            schedulerThread.join();
        }

        for(uint16_t i=0; i<maxWorkerThreads; i++) {
            if(workerThreads[i].joinable())
                workerThreads[i].join();
        }

        workerThreads.clear();
    }
private:
    void scheduler() {
        while(true) {
            unique_lock<mutex> lock(scheduleMux);
            notEmptyCv.wait(lock, [&]{return shutDownFlag || !scheduleQueue.empty();});

            if(shutDownFlag && scheduleQueue.empty()) return; // Gracefull shutdown

            shared_ptr<Job> job = scheduleQueue.top();
            auto timeNow = chrono::steady_clock::now();
            if(job->executedAt > timeNow) {
                notEmptyCv.wait_until(lock, job->executedAt);
                continue;
            }

            scheduleQueue.pop();
            lock.unlock();

            {
                unique_lock<mutex> lock(readyMux);
                readyQueue.push(job);
            }

            readyCv.notify_one();

        }
    }

    void worker() {
        while(true) {
            unique_lock<mutex> lock(readyMux);
            readyCv.wait(lock, [&]{return shutDownFlag || !readyQueue.empty();});

            if(shutDownFlag && readyQueue.empty()) return;

            shared_ptr<Job> job = readyQueue.front();
            readyQueue.pop();
            lock.unlock();

            try {
                job->status = JobStatus::RUNNING;
                job->task();
                job->status = JobStatus::COMPLETED;
            }
            catch (...) {
                // Retry logic
                if(job->retryCount + 1 <= job->maxRetry) {
                    int backoff = 1 << job->retryCount; // retry after 1, 2, 4, 8, 16 sec,  exponential backoff
                    job->retryCount++;
                    job->status = JobStatus::SCHEDULED;
                    job->executedAt = chrono::steady_clock::now() + chrono::seconds(backoff);

                    {
                        unique_lock<mutex> lock(scheduleMux);
                        scheduleQueue.push(job);
                    }
                    
                    notEmptyCv.notify_one();
                } else {
                    job->status = JobStatus::FAILED;
                }
                
            }

        }
    }

};

int main() {
    cout << "main is running\n";

    return 0;
}