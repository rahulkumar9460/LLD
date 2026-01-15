#include<iostream>
#include<unordered_map>
#include<thread>
#include<mutex>
#include<chrono>

using namespace std;

struct Bucket {
    int capacity;
    int tokens;
    int refillRate;
    chrono::steady_clock::time_point lastRefilledAt;
    mutex mux;

    Bucket(int cap, int rate) : 
        capacity(cap), 
        tokens(rate),
        refillRate(rate),
        lastRefilledAt(chrono::steady_clock::now()) 
    {}
};

class RateLimiter {
public:
    virtual bool isAllowed(string ip) = 0;
    ~RateLimiter() {}
};

class TokenBucketRateLimiter : public RateLimiter {
    unordered_map<string, shared_ptr<Bucket>> buckets;
    int capacity;
    int refillRate;
    mutex mux;
    thread cleanupThread;
public:
    TokenBucketRateLimiter(int cap, int rate): 
        capacity(cap), 
        refillRate(rate),
        cleanupThread(thread(&TokenBucketRateLimiter::cleanupWorker, this)) {}

    bool isAllowed(string ip) {
        shared_ptr<Bucket> bucket;
        {
            lock_guard<mutex> lock(mux);
            if(buckets.find(ip) == buckets.end()) {
                buckets[ip] = make_shared<Bucket>(capacity, refillRate);
            }
            bucket = buckets[ip];
        }

        {   
            lock_guard<mutex> lock(bucket->mux);
            // refill token if needed
            auto now = chrono::steady_clock::now(); 
            auto duration = chrono::duration_cast<chrono::seconds>(now - bucket->lastRefilledAt).count();
            
            if (duration > 0) { 
                int tokensToAdd = duration * bucket->refillRate; 
                bucket->tokens = min(bucket->capacity, bucket->tokens + tokensToAdd); 
                bucket->lastRefilledAt = now; 
            }
            
            if(bucket->tokens > 0) {
                bucket->tokens--;
                return true;
            }
        }

        return false;
    }

    void cleanupWorker() {
        while(true) {
            this_thread::sleep_for(chrono::minutes(1));

            vector<string> toDelete;
            auto now = chrono::steady_clock::now();
            
            // identify stale buckets
            {
                lock_guard<mutex> lock(mux);

                for(const auto& [ip, bucket] : buckets) {
                    lock_guard<mutex> bLock(bucket->mux);
                    if(bucket->lastRefilledAt + chrono::minutes(5) < now) 
                        toDelete.push_back(ip);
                }
            }

            // delete stale buckets
            {
                lock_guard<mutex> lock(mux);
                for(string &ip : toDelete) {
                    buckets.erase(ip);
                }
            }
        }
    }
};

int main () {
    cout << "Main:: rate limitter\n";
    TokenBucketRateLimiter rateLimiter(5, 2); // 5 requests capacity, 1 request per second refill
    string testIp = "192.168.1.1";
    for(int i = 0; i < 10; i++) {
        if(rateLimiter.isAllowed(testIp)) {
            cout << "Request " << i+1 << " from " << testIp << " is allowed.\n";
        } else {
            cout << "Request " << i+1 << " from " << testIp << " is denied.\n";
        }
        this_thread::sleep_for(chrono::milliseconds(300));
    }
    return 0;
}