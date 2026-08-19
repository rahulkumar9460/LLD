#include<iostream>
#include<chrono>
#include<unordered_map>
#include<queue>
using namespace std;

const int not_found = INT_MIN;

struct Entry {
public:
    int key;
    int value;
    chrono::steady_clock::time_point createdAt;
    chrono::steady_clock::time_point expireAt;
    int version;

    Entry(int k, int v, chrono::steady_clock::time_point cAt, chrono::steady_clock::time_point eAt, int ver) {
        this->key = k;
        this->value = v;
        this->createdAt = cAt;
        this->expireAt = eAt;
        this->version = ver;
    }

    bool operator>(const Entry &other) {
        return this->expireAt > other.expireAt;
    }
};

class KeyStore {
    chrono::seconds ttl;
    unordered_map<int, Entry> entriesMap;
    long long runnningSum = 0;
    int runnningCount = 0;

    priority_queue<Entry, vector<Entry>, greater<Entry>> minHeap;
public:
    KeyStore(chrono::seconds ttl) {
        this->ttl = ttl;
    }

    int get(int key) {
        removeExpired();

        if(!entriesMap.count(key)) return not_found;
        
        return entriesMap[key].value;
    }

    int put(int key, int value) {
        removeExpired();
        
        auto now = chrono::steady_clock::now();
        
        int version = 1;
        if(entriesMap.count(key)) {
            this->runnningSum -= entriesMap[key].value;
            this->runnningCount -= 1;

            version = entriesMap[key].version + 1;
        }

        Entry entry(key, value, now, now+this->ttl, version);

        this->entriesMap[key] = entry;
        this->minHeap.push(entry);

        this->runnningSum += value;
        this->runnningCount += 1;

        return;
    }

    double average() {
        // remove expired entries using min heap
        removeExpired();

        if(this->runnningCount == 0) return 0.0;
        else return (double)this->runnningSum/this->runnningCount;
    }

    void removeExpired() {
        auto now = chrono::steady_clock::now();
        while(!minHeap.empty() && minHeap.top().expireAt <= now) {
            Entry entry = minHeap.top();
            minHeap.pop();

            if(!entriesMap.count(entry.key)) continue;
            if(entry.version != entriesMap[entry.key].version) continue; // old version

            this->runnningSum -= entry.value;
            this->runnningCount -= 1;

            entriesMap.erase(entry.key);
        }
        return;
    }
};

int main() {
    cout << "main is running/n";
    return 0;
}