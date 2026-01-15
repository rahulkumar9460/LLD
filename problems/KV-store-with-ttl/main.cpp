#include<iostream>
#include<chrono>
#include<mutex>
#include<shared_mutex>
#include<unordered_map>
#include<thread>
#include<memory>

using namespace std;

class DataItem {
public:
    string key;
    string value;
    int64_t version;
    chrono::steady_clock::time_point createdAt;
    chrono::steady_clock::time_point updatedAt;

    chrono::seconds ttl;
    chrono::steady_clock::time_point expireAt;
    mutex mux;

    DataItem() = default;
    DataItem(string k, string v, chrono::seconds ttl): 
        key(k), 
        value(v), 
        version(1), 
        ttl(ttl), 
        expireAt(chrono::steady_clock::now() + ttl),
        createdAt(chrono::steady_clock::now()) {}
};

class KvNode {
    unordered_map<string, shared_ptr<DataItem>> table;
    shared_mutex mux;

    thread cleanupThread;
    bool shutDownFlag = false;

public:
    KvNode() {
        cleanupThread = thread(&KvNode::cleanupWorker, this);
    }
    string get(string key) {
        shared_ptr<DataItem> item;
        {
            shared_lock<shared_mutex> lock(mux);
            auto it = table.find(key);
            if(it == table.end()) return "";

            item = it->second;
        }

        bool isExpired = false;
        {
            lock_guard<mutex> itemLock(item->mux);
            auto now = chrono::steady_clock::now();
            if(now >= item->expireAt) {
                // key expired
                isExpired = true;
            } else return item->value;
        }
        
        if(isExpired) {
            // remove the key from table
            deleteKey(key);
            return "";
        }

        return item->value;
    }

    void put(string key, string value, chrono::seconds ttl) {
        shared_ptr<DataItem> item;
        {
            unique_lock<shared_mutex> lock(mux);
            auto it = table.find(key);
            if(it == table.end()) {
                item = make_shared<DataItem>(key, value, ttl);
                table[key] = item;
                return;
            } else {
                item = it->second;
            }
        }
        
        lock_guard<mutex> itemLock(item->mux);
        item->value = value;
        item->version += 1;
        item->updatedAt = chrono::steady_clock::now();
        item->ttl = ttl;
        item->expireAt = chrono::steady_clock::now() + ttl;
    }

    void deleteKey(string key) {
        unique_lock<shared_mutex> lock(mux);
        table.erase(key);
    }

    void cleanupWorker() {
        while(!shutDownFlag) {
            this_thread::sleep_for(chrono::seconds(10));

            vector<string> toDelete;
            auto now = chrono::steady_clock::now();

            {
                shared_lock<shared_mutex> lock(mux);
                for(const auto& [key, item] : table) {
                    lock_guard<mutex> itemLock(item->mux);
                    if(now >= item->expireAt) {
                        toDelete.push_back(key);
                    }
                }
            }

            if(!toDelete.empty()) {
                unique_lock<shared_mutex> lock(mux);
                for(string &key : toDelete) {
                    table.erase(key);
                }
            }
        }
    }
};

int main() {
    KvNode kvStore;

    kvStore.put("key1", "value1", chrono::seconds(5));
    cout << "Get key1: " << kvStore.get("key1") << endl;
    this_thread::sleep_for(chrono::seconds(6));
    cout << "Get key1 after expiry: " << kvStore.get("key1") << endl;
    kvStore.put("key2", "value2", chrono::seconds(10));
    cout << "Get key2: " << kvStore.get("key2") << endl;
    cout << "Updating key2..." << endl;
    kvStore.put("key2", "value2-updated", chrono::seconds(10));
    cout << "Get updated key2: " << kvStore.get("key2") << endl;
    return 0;
}