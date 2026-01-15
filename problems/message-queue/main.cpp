#include<iostream>
#include<deque>
#include<thread>
#include<condition_variable>

using namespace std;

template <typename T>
struct Message {
public:
    string id;
    T payload;

    // timing
    chrono::steady_clock::time_point enqueueTime;
    chrono::milliseconds ttl;
    chrono::steady_clock::time_point expiryTime;
    chrono::steady_clock::time_point deliveryTime;

    int retryCount=0;
    int shardId = 0;
};

struct RetryItem {
    uint64_t messageId;
    chrono::steady_clock::time_point retryAt;

    bool operator>(const RetryItem& other) const {
        return retryAt > other.retryAt;
    }
};

template <typename T>
class Queue_shard {
    deque<Message<T>> q;
    mutex mux;
    condition_variable notFullCv;
    condition_variable notEmptyCv;
    int capacity;
    int shardId;
    bool shutDownFlag;

    atomic<uint64_t> idCounter{0};

    unordered_map<uint64_t, Message<T>> inFlightMsg;
    mutex inFlightMsgMux;

    // used to decide when and how a item is pushed from inFlightMsg back to queue for retry
    priority_queue<RetryItem, vector<RetryItem>, greater<RetryItem>> retryMinHeap;
    mutex retryMux;
    condition_variable retryCv;

    thread retryThread;

public:
    Queue_shard(int cap, int id) : capacity{cap}, shardId{id}, shutDownFlag{false} {
        retryThread = thread(&MessageQueue::retryWorker, this);
    }

    void publish(const T &payload, chrono::milliseconds ttl) {
        unique_lock<mutex> lock(mux);
        notFullCv.wait(lock, [&]{return q.size() < capacity;});

        Message<T> msg;
        msg.id = idCounter.fetch_add(1);
        msg.payload = payload;

        msg.enqueueTime = chrono::steady_clock::now();
        msg.ttl = ttl;
        msg.expiryTime = msg.enqueueTime + ttl;

        msg.shardId = shardId;

        q.push_back(move(msg));
        notEmptyCv.notify_one();
    }

    Message<T> consume() {
        unique_lock<mutex> lock(mux);

        while(true) {
            notEmptyCv.wait(lock, [&]{return !q.empty();});

            Message<T> msg = q.front();
            q.pop_front();
            notFullCv.notify_one();

            // if message is expired, check for next message
            if(msg.expiryTime <= chrono::steady_clock::now()) continue;
            
            // Put the message into in-flight ALWAYS
            {
                lock_guard<mutex> lk(inFlightMsgMux);
                msg.deliveryTime = chrono::time_point::now();
                inFlightMsg[msg.id] = msg;
            }
            
            
            return msg;
        }
    }

    void ack(int id) {
        unique_lock<mutex> lock(inFlightMsgMux);
        if(inFlightMsg.find(id) != inFlightMsg.end())
            inFlightMsg.erase(id);
    }

    void retryWorker() {
        while(!shutDownFlag) {
            vector<Message<T>> expired;

            // step 1: scan inFlight items
            {
                unique_lock<mutex> lock(inFlightMsgMux);
                auto now = chrono::time_point::now();

                for(auto &[id, msg] : inFlightMsg) {
                    // message is not acked within 100 seconds of delivery:: Retry
                    if(msg.deliveryTime < now - 100*chrono::milliseconds) {
                        expired.push_back(msg);
                    }
                }

                for(auto &m: expired) inFlightMsg.erase(m.id);
            }

            // Step 2: schedule expired messages into retryMinHeap
            {
                unique_lock<mutex> lock(retryMux);
                auto now = chrono::steady_clock::now();

                for(auto &m : expired) {
                    retryMinHeap.push(
                        RetryItem{
                            m.id,
                            now + 100*chrono::milliseconds;
                        }
                    )
                }

                retryCv.notify_one();
            }



            unique_lock<mutex> lock(retryMux);

            retryCv.wait(lock, [&]{return !retryMinHeap.empty();});

            auto retryItem = retryMinHeap.top();
            
            // wait until retry time comes
            retryCv.wait_until(retryItem.retryAt);

            retryItem = retryMinHeap.top();
            retryMinHeap.pop();
            lock.unlock();

            // move the retry item back to queue from inFlightMsg
            {
                unique_lock<mutex> lock(inFlightMsgMux);

                if(inFlightMsg.find(retryItem.messageId) == inFlightMsg.end()) continue;

                Message<T> msg = inFlightMsg[retryItem.messageId];
                inFlightMsg.erase(retryItem.messageId);
                msg.retryCount--;
            }

            publish(msg.payload, ttl);
        }
    }
};
