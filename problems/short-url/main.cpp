#include<iostream>
#include<unordered_map>
#include<string>
#include<chrono>
#include<atomic>

using namespace std;

// Simple Short URL Service

class UrlItem {
public:
    int id;
    string originalUrl;
    string shortCode;
    chrono::steady_clock::time_point createdAt;
    chrono::steady_clock::time_point expiryAt;

    UrlItem() = default;
    UrlItem(const string &orig, const string &code, chrono::steady_clock::time_point created, chrono::steady_clock::time_point expiry)
        : originalUrl{orig}, shortCode{code}, createdAt{created}, expiryAt{expiry} {}
};

class IdGenerator {
public:
    virtual int generateId() = 0;
};

class AtomicIdGenerator : public IdGenerator {
    atomic<int> counter;
public:
    AtomicIdGenerator() : counter{1} {}
    int generateId() override {
        return counter.fetch_add(1);
    }
};

class Repository {
    unordered_map<string, UrlItem> urlMap;
    unordered_map<string, string> longToShortMap;
public:
    void save(const UrlItem &item) {
        urlMap[item.shortCode] = item;
        longToShortMap[item.originalUrl] = item.shortCode;
    }

    UrlItem* getByShortCode(const string &shortCode) {
        if(urlMap.find(shortCode) != urlMap.end()) {
            return &urlMap[shortCode];
        }
        return nullptr;
    }

    UrlItem* getByOriginalUrl(const string &originalUrl) {
        if(longToShortMap.find(originalUrl) != longToShortMap.end()) {
            string shortCode = longToShortMap[originalUrl];
            return &urlMap[shortCode];
        }
        return nullptr;
    }
};

string base62Encode(int id) {
    const string chars = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    string shortCode;
    while(id > 0) {
        shortCode += chars[id % 62];
        id /= 62;
    }
    return shortCode;
}

class UrlShortenerService {
    Repository repo;
    IdGenerator &idGen;
public:
    UrlShortenerService(IdGenerator &idGenerator) : idGen{idGenerator} {}

    string shortenUrl(string &originalUrl, chrono::seconds ttl) {
        UrlItem* existing = repo.getByOriginalUrl(originalUrl);
        if(existing != nullptr) {
            return existing->shortCode;
        }

        int id = idGen.generateId();
        string shortCode = base62Encode(id);
        auto now = chrono::steady_clock::now();
        UrlItem item{originalUrl, shortCode, now, now + ttl};
        repo.save(item);
        return shortCode;
    }

    string getOriginalUrl(const string &shortCode) {
        UrlItem* item = repo.getByShortCode(shortCode);
        if(item != nullptr && item->expiryAt > chrono::steady_clock::now()) {
            return item->originalUrl;
        }
        return "";
    }
};

// use command to run: g++ -std=c++14 main.cpp -o short-url
int main() {
    cout << "Short URL Service\n";

    AtomicIdGenerator idGen;
    UrlShortenerService urlService{idGen};  
    string originalUrl = "https://www.example.com/some/very/long/url";
    chrono::seconds ttl{3600}; // 1 hour TTL
    string shortCode = urlService.shortenUrl(originalUrl, ttl);
    cout << "Original URL: " << originalUrl << "\n";
    cout << "Shortened URL Code: " << shortCode << "\n";    

    string retrievedUrl = urlService.getOriginalUrl(shortCode);
    cout << "Retrieved Original URL: " << retrievedUrl << "\n";

    return 0;
}
